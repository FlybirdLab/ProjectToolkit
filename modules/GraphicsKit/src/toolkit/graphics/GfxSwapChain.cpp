/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GfxSwapChain.hpp"

#include "GfxTexture.hpp"
#include "GfxDevice.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/threading/Task.hpp"

namespace toolkit
{

    class GfxSwapChainDownloadTask : public TkTask
    {
        friend GfxSwapChain;

    public:
        GfxSwapChain *swapchain;
        GfxTexture *texture;

        ~GfxSwapChainDownloadTask()
        {
            if (texture)
                GFX_DEL_RESOURCE(texture);
        }

        TkString toString() const
        {
            return "GfxSwapChainDownloadTask";
        }

        bool run() override
        {
            return texture == nullptr;
        }

        void enqueue() override
        {
            setState(TaskState::kQueued);
        }

        void complete()
        {
            setState(TaskState::kQueued);
            execute();
        }
    };

    GfxSwapChain::GfxSwapChain()
    {
#if TK_DEBUG
        setName("Swap Chain (backbuffers)");
#endif
    }

    TkTask *GfxSwapChain::downloadDataAsync(TextureData &result)
    {
        if (m_downloadTask)
        {
            LogWarn("Can download window backuffer data only once at the time.");
            return nullptr;
        }

        auto *device = GfxDevice::getInstance();
        if (!device)
            return nullptr;

        auto texture = device->createTexture();
        if (!texture->init(GfxTextureDescription::new2D(getWidth(), getHeight(), getFormat(), GfxTextureFlags::kNone, 1).toStagingReadback()))
        {
            LogWarn("Failed to create staging texture for the window swapchain backuffer download.");
            GFX_DEL_RESOURCE(texture);
            return nullptr;
        }

        auto task = TkMemory::newObject<GfxSwapChainDownloadTask>();
        if (!task)
        {
            GFX_DEL_RESOURCE(texture);
            return nullptr;
        }
        task->swapchain = this;
        task->texture = texture;

        const auto downloadTask = texture->downloadDataAsync(result);
        if (!downloadTask)
        {
            TkMemory::deletePtr(task);
            return nullptr;
        }
        task->continueWith(downloadTask);

        m_downloadTask = task;
        return task;
    }

    void GfxSwapChain::end()
    {
        if (m_downloadTask)
        {
            auto downloadTask = (GfxSwapChainDownloadTask *)m_downloadTask;
            auto *device = GfxDevice::getInstance();
            if (device)
            {
                auto context = device->getMainContext();
                copyBackbuffer(context, downloadTask->texture);
                downloadTask->complete();
            }
            m_downloadTask = nullptr;
        }
    }

    void GfxSwapChain::present(bool vsync)
    {
        onPresented();
    }

    TkString GfxSwapChain::toString() const
    {
#if TK_DEBUG
        return TkString::format("SwapChain %ix%i, %s", getWidth(), getHeight(), getName());
#else
        return "SwapChain";
#endif
    }

    GfxResType GfxSwapChain::getType() const
    {
        return GfxResType::kTexture;
    }

}
