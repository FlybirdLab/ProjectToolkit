/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/graphics/async/GfxTask.hpp"
#include <toolkit/collection/DataCollection.hpp>
#include <toolkit/graphics/common.hpp>
#include <toolkit/graphics/GfxTexture.hpp>
#include <toolkit/graphics/async/GfxTaskContext.hpp>
#include <toolkit/graphics/GfxContext.hpp>

namespace toolkit
{
    // GPU texture mip upload task.
    class GfxUploadTextureMipTask : public GfxTask
    {
    public:
        GfxUploadTextureMipTask(GfxTexture *texture, i32 mipIndex, TkSpan<byte> data, i32 rowPitch, i32 slicePitch, bool copyData)
            : GfxTask(Type::kUploadTexture), m_texture(texture), m_mipIndex(mipIndex), m_rowPitch(rowPitch), m_slicePitch(slicePitch)
        {
            m_texture.onRelease.bind<GfxUploadTextureMipTask, &GfxUploadTextureMipTask::onResourceReleased>(this);

            if (copyData)
                mm_data.copy(data);
            else
                mm_data.link(data);
        }

    protected:
        Result run(GfxTaskContext *context) override
        {
            const auto texture = m_texture.get();
            if (texture == nullptr)
                return Result::kMissingResources;
            TK_ASSERT(texture->isAllocated());

            // Update all array slices
            const byte *dataSource = mm_data.get();
            const i32 arraySize = texture->arraySize();
            TK_ASSERT(mm_data.length() >= m_slicePitch * arraySize);
            for (i32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
            {
                context->context->updateTexture(texture, arrayIndex, m_mipIndex, dataSource, m_rowPitch, m_slicePitch);
                dataSource += m_slicePitch;
            }

            return Result::kOk;
        }

        void onSync() override
        {
            auto texture = m_texture.get();
            if (texture)
            {
                if (m_mipIndex == texture->highestResidentMipIndex() - 1)
                {
                    // Mark the new mip as loaded
                    texture->setResidentMipLevels(texture->residentMipLevels() + 1);
                }
                else
                {
                    // Mark the new mip and all lower ones as loaded (eg. when loading Model SDF texture mips at once but out of order)
                    texture->setResidentMipLevels(TkMath::max(texture->residentMipLevels(), texture->mipLevels() - m_mipIndex));
                }
            }

            // Base
            GfxTask::onSync();
        }

        void onEnd() override
        {
            m_texture.unlink();

            // Base
            GfxTask::onEnd();
        }

    private:
        void onResourceReleased()
        {
            cancel();
        }

    protected:
        GfxTextureReference m_texture;
        i32 m_mipIndex, m_rowPitch, m_slicePitch;
        ByteCollection mm_data;
    };

    // GPU buffer upload task.
    class GfxUploadBufferTask : public GfxTask
    {
    public:
        GfxUploadBufferTask(GfxBuffer *buffer, i32 offset, TkSpan<byte> data, bool copyData)
            : GfxTask(Type::kUploadBuffer), m_buffer(buffer), m_offset(offset)
        {
            m_buffer.onRelease.bind<GfxUploadBufferTask, &GfxUploadBufferTask::onResourceReleased>(this);

            if (copyData)
                m_data.copy(data);
            else
                m_data.link(data);
        }

    protected:
        Result run(GfxTaskContext *context) override
        {
            if (!m_buffer)
                return Result::kMissingResources;
            context->context->updateBuffer(m_buffer, m_data.get(), m_data.length(), m_offset);
            return Result::kOk;
        }
        void onEnd() override
        {
            m_buffer.unlink();

            // Base
            GfxTask::onEnd();
        }

    private:
        void onResourceReleased()
        {
            cancel();
        }

    protected:
        GfxBufferReference m_buffer;
        i32 m_offset;
        ByteCollection m_data;
    };

    // GPU resource copy task.
    class GfxCopyResourceTask : public GfxTask
    {
    public:
        GfxCopyResourceTask(GfxResource *src, GfxResource *dst)
            : GfxTask(Type::kCopyResource), m_srcResource(src), m_dstResource(dst)
        {
            m_srcResource.onRelease.bind<GfxCopyResourceTask, &GfxCopyResourceTask::onResourceReleased>(this);
            m_dstResource.onRelease.bind<GfxCopyResourceTask, &GfxCopyResourceTask::onResourceReleased>(this);
        }

    protected:
        Result run(GfxTaskContext *context) override
        {
            if (!m_srcResource || !m_dstResource)
                return Result::kMissingResources;
            context->context->copyResource(m_dstResource, m_srcResource);
            return Result::kOk;
        }
        void onEnd() override
        {
            m_srcResource.unlink();
            m_dstResource.unlink();

            // Base
            GfxTask::onEnd();
        }

    private:
        void onResourceReleased()
        {
            cancel();
        }

    private:
        GfxResourceReference m_srcResource;
        GfxResourceReference m_dstResource;
    };

    // GPU subresource copy task.
    class GPUCopySubresourceTask : public GfxTask
    {
    private:
        GfxResourceReference m_srcResource;
        GfxResourceReference m_dstResource;
        u32 m_srcSubresource, m_dstSubresource;

    public:
        GPUCopySubresourceTask(GfxResource *src, GfxResource *dst, u32 srcSubresource, u32 dstSubresource)
            : GfxTask(Type::kCopyResource), m_srcResource(src), m_dstResource(dst), m_srcSubresource(srcSubresource), m_dstSubresource(dstSubresource)
        {
            m_srcResource.onRelease.bind<GPUCopySubresourceTask, &GPUCopySubresourceTask::onResourceReleased>(this);
            m_dstResource.onRelease.bind<GPUCopySubresourceTask, &GPUCopySubresourceTask::onResourceReleased>(this);
        }

    private:
        void onResourceReleased()
        {
            cancel();
        }

    protected:
        Result run(GfxTaskContext *context) override
        {
            if (!m_srcResource || !m_dstResource)
                return Result::kMissingResources;
            context->context->copySubresource(m_dstResource, m_dstSubresource, m_srcResource, m_srcSubresource);
            return Result::kOk;
        }
        void onEnd() override
        {
            m_srcResource.unlink();
            m_dstResource.unlink();

            // Base
            GfxTask::onEnd();
        }
    };

}