/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "GfxTexture.hpp"

#include "GfxContext.hpp"
#include "utils/GfxUtils.hpp"
#include "GfxDevice.hpp"
#include "utils/TextureData.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/debug/Profiler.hpp"
#include "toolkit/graphics/async/GfxTasks.hpp"
#include "toolkit/math/Vector3.hpp"
#include "toolkit/math/Vector2.hpp"
#include "toolkit/collection/DataCollection.hpp"
#include "toolkit/threading/Thread.hpp"
#include "toolkit/types/String.hpp"

namespace toolkit
{
    static i32 calculateMipMapCount(i32 requestedLevel, i32 width)
    {
        // i32 size = width;
        // i32 maxMipMap = 1 + TkMath::CeilToInt(TkMath::Log(size) / TkMath::Log(2.0));
        // return requestedLevel == 0 ? maxMipMap : TkMath::min(requestedLevel, maxMipMap);
        if (requestedLevel == 0)
            requestedLevel = GFX_MAX_TEXTURE_MIP_LEVELS;
        i32 maxMipMap = 1;
        while (width > 1)
        {
            width >>= 1;
            maxMipMap++;
        }
        return TkMath::min(requestedLevel, maxMipMap);
    }

    const char *toString(TextureDimensions value)
    {
        const char *result;
        switch (value)
        {
        case TextureDimensions::kTexture:
            result = "Texture";
            break;
        case TextureDimensions::kVolumeTexture:
            result = "VolumeTexture";
            break;
        case TextureDimensions::kCubeTexture:
            result = "CubeTexture";
            break;
        default:
            result = "";
        }
        return result;
    }

    GfxTextureDescription GfxTextureDescription::new1D(i32 width, PixelFormat format, GfxTextureFlags textureFlags, i32 mipCount, i32 arraySize)
    {
        GfxTextureDescription desc;
        desc.dimensions = TextureDimensions::kTexture;
        desc.width = width;
        desc.height = 1;
        desc.depth = 1;
        desc.arraySize = arraySize;
        desc.mipLevels = calculateMipMapCount(mipCount, width);
        desc.format = format;
        desc.multiSampleLevel = GfxMSAALevel::kNone;
        desc.flags = textureFlags;
        desc.usage = GfxResourceUsage::kDefault;
        desc.defaultClearColor = Color::Black;
        return desc;
    }

    GfxTextureDescription GfxTextureDescription::new2D(i32 width, i32 height, PixelFormat format, GfxTextureFlags textureFlags, i32 mipCount, i32 arraySize, GfxMSAALevel msaaLevel)
    {
        GfxTextureDescription desc;
        desc.dimensions = TextureDimensions::kTexture;
        desc.width = width;
        desc.height = height;
        desc.depth = 1;
        desc.arraySize = arraySize;
        desc.mipLevels = calculateMipMapCount(mipCount, TkMath::max(width, height));
        desc.format = format;
        desc.multiSampleLevel = msaaLevel;
        desc.flags = textureFlags;
        desc.defaultClearColor = Color::Black;
        desc.usage = GfxResourceUsage::kDefault;
        return desc;
    }

    GfxTextureDescription GfxTextureDescription::new3D(const FVec3 &size, PixelFormat format, GfxTextureFlags textureFlags)
    {
        return new3D((i32)size.x, (i32)size.y, (i32)size.z, 1, format, textureFlags);
    }

    GfxTextureDescription GfxTextureDescription::new3D(i32 width, i32 height, i32 depth, PixelFormat format, GfxTextureFlags textureFlags, i32 mipCount)
    {
        GfxTextureDescription desc;
        desc.dimensions = TextureDimensions::kVolumeTexture;
        desc.width = width;
        desc.height = height;
        desc.depth = depth;
        desc.arraySize = 1;
        desc.mipLevels = calculateMipMapCount(mipCount, TkMath::max(width, height, depth));
        desc.format = format;
        desc.multiSampleLevel = GfxMSAALevel::kNone;
        desc.flags = textureFlags;
        desc.defaultClearColor = Color::Black;
        desc.usage = GfxResourceUsage::kDefault;
        return desc;
    }

    GfxTextureDescription GfxTextureDescription::newCube(i32 size, PixelFormat format, GfxTextureFlags textureFlags, i32 mipCount)
    {
        auto desc = new2D(size, size, format, textureFlags, mipCount, 6, GfxMSAALevel::kNone);
        desc.dimensions = TextureDimensions::kCubeTexture;
        return desc;
    }

    void GfxTextureDescription::clear()
    {
        TkMemory::memoryClear(this, sizeof(GfxTextureDescription));
        multiSampleLevel = GfxMSAALevel::kNone;
    }

    GfxTextureDescription GfxTextureDescription::toStagingUpload() const
    {
        auto copy = *this;
        copy.flags = GfxTextureFlags::kNone;
        copy.usage = GfxResourceUsage::kStagingUpload;
        return copy;
    }

    GfxTextureDescription GfxTextureDescription::toStagingReadback() const
    {
        auto copy = *this;
        copy.flags = GfxTextureFlags::kNone;
        copy.usage = GfxResourceUsage::kStagingReadback;
        return copy;
    }

    GfxTextureDescription GfxTextureDescription::toStaging() const
    {
        auto copy = *this;
        copy.flags = GfxTextureFlags::kNone;
        copy.usage = GfxResourceUsage::kStaging;
        return copy;
    }

    bool GfxTextureDescription::equals(const GfxTextureDescription &other) const
    {
        return dimensions == other.dimensions && width == other.width && height == other.height && depth == other.depth && arraySize == other.arraySize && mipLevels == other.mipLevels && format == other.format && multiSampleLevel == other.multiSampleLevel && flags == other.flags && usage == other.usage && Color::nearEqual(defaultClearColor, other.defaultClearColor);
    }

    TkString GfxTextureDescription::toString() const
    {
        return TkString::format("Size: %ix%ix%i[%i], Type: %s, Mips: %i, Format: %i, MSAA: %i, Flags: %i, Usage: %i",
                              width,
                              height,
                              depth,
                              arraySize,
                              toolkit::toString(dimensions),
                              mipLevels,
                              format,
                              multiSampleLevel,
                              flags,
                              (i32)usage);
    }

    u32 getHash(const GfxTextureDescription &key)
    {
        u32 hashCode = key.width;
        hashCode = (hashCode * 397) ^ key.height;
        hashCode = (hashCode * 397) ^ key.depth;
        hashCode = (hashCode * 397) ^ key.arraySize;
        hashCode = (hashCode * 397) ^ (u32)key.dimensions;
        hashCode = (hashCode * 397) ^ key.mipLevels;
        hashCode = (hashCode * 397) ^ (u32)key.format;
        hashCode = (hashCode * 397) ^ (u32)key.multiSampleLevel;
        hashCode = (hashCode * 397) ^ (u32)key.flags;
        hashCode = (hashCode * 397) ^ (u32)key.usage;
        hashCode = (hashCode * 397) ^ key.defaultClearColor.getHashCode();
        return hashCode;
    }

    GfxTexture *GfxTexture::newTexture()
    {
        return GfxDevice::sInstance->createTexture();
    }

    GfxTexture::GfxTexture()
        : GfxResource{}, m_residentMipLevels(0), m_sRGB(false), m_isBlockCompressed(false)
    {
        // Keep description data clear (we use m_desc.mipLevels to check if it's has been initated)
        m_desc.clear();
    }

    bool GfxTexture::isStaging() const
    {
        return m_desc.usage == GfxResourceUsage::kStagingUpload || m_desc.usage == GfxResourceUsage::kStagingReadback || m_desc.usage == GfxResourceUsage::kStaging;
    }

    FVec2 GfxTexture::size() const
    {
        return FVec2(static_cast<float>(m_desc.width), static_cast<float>(m_desc.height));
    }

    FVec3 GfxTexture::size3() const
    {
        return FVec3(static_cast<float>(m_desc.width), static_cast<float>(m_desc.height), static_cast<float>(m_desc.depth));
    }

    bool GfxTexture::isPowerOfTwo() const
    {
        return TkMath::isPowerOfTwo(m_desc.width) && TkMath::isPowerOfTwo(m_desc.height);
    }

    void GfxTexture::getMipSize(i32 mipLevelIndex, i32 &mipWidth, i32 &mipHeight) const
    {
        TK_ASSERT(mipLevelIndex >= 0 && mipLevelIndex < mipLevels());
        mipWidth = TkMath::max(1, width() >> mipLevelIndex);
        mipHeight = TkMath::max(1, height() >> mipLevelIndex);
    }

    void GfxTexture::getMipSize(i32 mipLevelIndex, i32 &mipWidth, i32 &mipHeight, i32 &mipDepth) const
    {
        TK_ASSERT(mipLevelIndex >= 0 && mipLevelIndex < mipLevels());
        mipWidth = TkMath::max(1, width() >> mipLevelIndex);
        mipHeight = TkMath::max(1, height() >> mipLevelIndex);
        mipDepth = TkMath::max(1, depth() >> mipLevelIndex);
    }

    void GfxTexture::getResidentSize(i32 &width, i32 &height) const
    {
        // Check if texture isn't loaded
        if (m_residentMipLevels <= 0)
        {
            width = height = 0;
            return;
        }

        const i32 mipIndex = m_desc.mipLevels - m_residentMipLevels;
        width = GfxTexture::width() >> mipIndex;
        height = GfxTexture::height() >> mipIndex;
    }

    void GfxTexture::getResidentSize(i32 &width, i32 &height, i32 &depth) const
    {
        // Check if texture isn't loaded
        if (m_residentMipLevels <= 0)
        {
            width = height = depth = 0;
            return;
        }

        const i32 mipIndex = m_desc.mipLevels - m_residentMipLevels;
        width = this->width() >> mipIndex;
        height = this->height() >> mipIndex;
        depth = this->depth() >> mipIndex;
    }

    u32 GfxTexture::rowPitch(i32 mipIndex) const
    {
        u32 rowPitch, slicePitch;
        computePitch(mipIndex, rowPitch, slicePitch);
        return rowPitch;
    }

    u32 GfxTexture::slicePitch(i32 mipIndex) const
    {
        u32 rowPitch, slicePitch;
        computePitch(mipIndex, rowPitch, slicePitch);
        return slicePitch;
    }

    void GfxTexture::computePitch(i32 mipIndex, u32 &rowPitch, u32 &slicePitch) const
    {
        i32 mipWidth, mipHeight;
        getMipSize(mipIndex, mipWidth, mipHeight);
        GfxUtils::computePitch(this->format(), mipWidth, mipHeight, rowPitch, slicePitch);
    }

    i32 GfxTexture::calculateMipSize(i32 size, i32 mipLevel) const
    {
        mipLevel = TkMath::min(mipLevel, mipLevels());
        return TkMath::max(1, size >> mipLevel);
    }

    i32 GfxTexture::computeSubresourceSize(i32 subresource, i32 rowAlign, i32 sliceAlign) const
    {
        const i32 mipLevel = subresource % mipLevels();
        const i32 slicePitch = computeSlicePitch(mipLevel, rowAlign);
        const i32 depth = calculateMipSize(GfxTexture::depth(), mipLevel);
        return TkMath::alignUp<i32>(slicePitch * depth, sliceAlign);
    }

    i32 GfxTexture::computeBufferOffset(i32 subresource, i32 rowAlign, i32 sliceAlign) const
    {
        i32 offset = 0;
        for (i32 i = 0; i < subresource; i++)
        {
            offset += computeSubresourceSize(i, rowAlign, sliceAlign);
        }
        return offset;
    }

    i32 GfxTexture::computeBufferTotalSize(i32 rowAlign, i32 sliceAlign) const
    {
        i32 result = 0;
        for (i32 mipLevel = 0; mipLevel < mipLevels(); mipLevel++)
        {
            const i32 slicePitch = computeSlicePitch(mipLevel, rowAlign);
            const i32 depth = calculateMipSize(GfxTexture::depth(), mipLevel);
            result += TkMath::alignUp<i32>(slicePitch * depth, sliceAlign);
        }
        return result * arraySize();
    }

    i32 GfxTexture::computeSlicePitch(i32 mipLevel, i32 rowAlign) const
    {
        return computeRowPitch(mipLevel, rowAlign) * calculateMipSize(height(), mipLevel);
    }

    i32 GfxTexture::computeRowPitch(i32 mipLevel, i32 rowAlign) const
    {
        i32 mipWidth = calculateMipSize(width(), mipLevel);
        i32 mipHeight = calculateMipSize(height(), mipLevel);
        u32 rowPitch, slicePitch;
        GfxUtils::computePitch(GfxTexture::format(), mipWidth, mipHeight, rowPitch, slicePitch);
        return TkMath::alignUp<i32>(rowPitch, rowAlign);
    }

    bool GfxTexture::init(const GfxTextureDescription &desc)
    {
        // Validate description
        const auto device = GfxDevice::sInstance;
        if (desc.usage == GfxResourceUsage::kDynamic)
        {
            LogWarn("Cannot create texture. Dynamic textures are not supported. Description: %s", desc.toString());
            return false;
        }
        if (desc.mipLevels < 0 || desc.mipLevels > GFX_MAX_TEXTURE_MIP_LEVELS)
        {
            LogWarn("Cannot create texture. Invalid amount of mip levels. Description: %s", desc.toString());
            return false;
        }
        if (desc.isDepthStencil())
        {
            if (desc.mipLevels > 1)
            {
                LogWarn("Cannot create texture. Depth Stencil texture cannot have mip maps. Description: %s", desc.toString());
                return false;
            }
            if (desc.isRenderTarget())
            {
                LogWarn("Cannot create texture. Depth Stencil texture cannot be used as a Render Target. Description: %s", desc.toString());
                return false;
            }
            if (enumHasAnyFlags(desc.flags, GfxTextureFlags::kReadOnlyDepthView) && !device->limits.hasReadOnlyDepth)
            {
                LogWarn("Cannot create texture. The current graphics platform does not support read-only Depth Stencil texture. Description: %s", desc.toString());
                return false;
            }
        }
        else
        {
            if (enumHasAnyFlags(desc.flags, GfxTextureFlags::kReadOnlyDepthView))
            {
                LogWarn("Cannot create texture. Cannot create read-only Depth Stencil texture that is not a Depth Stencil texture. Add DepthStencil flag. Description: %s", desc.toString());
                return false;
            }
        }
        if (desc.hasPerMipViews() && !(desc.isShaderResource() || desc.isRenderTarget()))
        {
            LogWarn("Cannot create texture. Depth Stencil texture cannot have mip maps. Description: %s", desc.toString());
            return false;
        }
        switch (desc.dimensions)
        {
        case TextureDimensions::kTexture:
        {
            if (desc.hasPerSliceViews())
            {
                LogWarn("Cannot create texture. Texture cannot have per slice views. Description: %s", desc.toString());
                return false;
            }
            if (desc.width <= 0 || desc.height <= 0 || desc.arraySize <= 0 || desc.width > device->limits.maxTexture2DSize || desc.height > device->limits.maxTexture2DSize || desc.arraySize > device->limits.maxTexture2DArraySize)
            {
                LogWarn("Cannot create texture. Invalid dimensions. Description: %s", desc.toString());
                return false;
            }

            break;
        }
        case TextureDimensions::kVolumeTexture:
        {
            if (desc.isDepthStencil())
            {
                LogWarn("Cannot create texture. Only 2D Texture can be used as a Depth Stencil. Description: %s", desc.toString());
                return false;
            }
            if (desc.arraySize != 1)
            {
                LogWarn("Cannot create texture. Volume texture cannot create array of volume textures. Description: %s", desc.toString());
                return false;
            }
            if (desc.multiSampleLevel != GfxMSAALevel::kNone)
            {
                LogWarn("Cannot create texture. Volume texture cannot use multi-sampling. Description: %s", desc.toString());
                return false;
            }
            if (desc.hasPerMipViews())
            {
                LogWarn("Cannot create texture. Volume texture cannot have per mip map views. Description: %s", desc.toString());
                return false;
            }
            if (desc.hasPerMipViews() && desc.isRenderTarget() == false)
            {
                LogWarn("Cannot create texture. Volume texture cannot have per slice map views if is not a render target. Description: %s", desc.toString());
                return false;
            }
            if (desc.width <= 0 || desc.height <= 0 || desc.depth <= 0 || desc.width > device->limits.maxTexture3DSize || desc.height > device->limits.maxTexture3DSize || desc.depth > device->limits.maxTexture3DSize)
            {
                LogWarn("Cannot create texture. Invalid dimensions. Description: %s", desc.toString());
                return false;
            }

            break;
        }
        case TextureDimensions::kCubeTexture:
        {
            if (desc.hasPerSliceViews())
            {
                LogWarn("Cannot create texture. Cube texture cannot have per slice views. Description: %s", desc.toString());
                return false;
            }
            if (desc.width <= 0 || desc.arraySize <= 0 || desc.width > device->limits.maxTextureCubeSize || desc.height > device->limits.maxTextureCubeSize || desc.arraySize * 6 > device->limits.maxTexture2DArraySize || desc.width != desc.height)
            {
                LogWarn("Cannot create texture. Invalid dimensions. Description: %s", desc.toString());
                return false;
            }

            break;
        }
        }
        const bool isCompressed = GfxPixelFormat::isCompressed(desc.format);
        if (isCompressed)
        {
            const i32 blockSize = GfxPixelFormat::computeBlockSize(desc.format);
            if (desc.width < blockSize || desc.height < blockSize)
            {
                LogWarn("Cannot create texture. Invalid dimensions. Description: %s", desc.toString());
                return false;
            }
        }

        // Release previous data
        releaseData();

        // Initialize
        m_desc = desc;
        m_sRGB = GfxPixelFormat::isSRGB(desc.format);
        m_isBlockCompressed = isCompressed;
        if (!onInit())
        {
            releaseData();
            m_desc.clear();
            m_residentMipLevels = 0;
            LogWarn("Cannot initialize texture. Description: %s", desc.toString());
            return false;
        }

        // Render targets and depth buffers doesn't support normal textures streaming and are considered to be always resident
        if (isRegularTexture() == false)
        {
            m_residentMipLevels = mipLevels();
        }

        return true;
    }

    GfxTexture *GfxTexture::toStagingReadback() const
    {
        const auto desc = m_desc.toStagingReadback();
        TkMutexGuard gpuLock(GfxDevice::sInstance->mtx);
        auto *staging = GfxDevice::sInstance->createTexture("Staging.Readback");
        if (!staging->init(desc))
        {
            TkMemory::deletePtr(staging);
            return nullptr;
        }
        return staging;
    }

    GfxTexture *GfxTexture::toStagingUpload() const
    {
        const auto desc = m_desc.toStagingUpload();
        TkMutexGuard gpuLock(GfxDevice::sInstance->mtx);
        auto *staging = GfxDevice::sInstance->createTexture("Staging.Upload");
        if (!staging->init(desc))
        {
            TkMemory::deletePtr(staging);
            return nullptr;
        }
        return staging;
    }

    bool GfxTexture::resize(i32 width, i32 height, i32 depth, PixelFormat format)
    {
        PROFILE();
        if (!isAllocated())
        {
            LogWarn("Cannot resize not created textures.");
            return false;
        }

        auto desc = getDescription();
        if (format == PixelFormat::kUnknown)
            format = desc.format;

        // Skip if size won't change
        if (desc.width == width && desc.height == height && desc.depth == depth && desc.format == format)
            return true;

        desc.format = format;
        desc.width = width;
        desc.height = height;
        desc.depth = depth;
        if (desc.mipLevels > 1)
            desc.mipLevels = calculateMipMapCount(0, TkMath::max(width, height));

        // Recreate
        return init(desc);
    }

    u64 GfxTexture::calculateMemoryUsage() const
    {
        return GfxUtils::calculateTextureMemoryUsage(format(), width(), height(), depth(), mipLevels()) * arraySize();
    }

    TkString GfxTexture::toString() const
    {
#if GFX_ENABLE_RESOURCE_NAMING
        return TkString::format("Texture %s, Residency: %i, Name: %s", m_desc.toString(), m_residentMipLevels, getName());
#else
        return "Texture";
#endif
    }

    GfxResType GfxTexture::getType() const
    {
        if (isVolume())
            return GfxResType::kVolumeTexture;
        if (isCubeMap())
            return GfxResType::kCubeTexture;
        return isRegularTexture() ? GfxResType::kTexture : GfxResType::kRenderTarget;
    }

    void GfxTexture::onReleaseData()
    {
        m_desc.clear();
        m_residentMipLevels = 0;
    }

    GfxTask *GfxTexture::uploadMipMapAsync(const ByteCollection &data, i32 mipIndex, bool copyData)
    {
        u32 rowPitch, slicePitch;
        computePitch(mipIndex, rowPitch, slicePitch);
        return uploadMipMapAsync(data, mipIndex, rowPitch, slicePitch, copyData);
    }

    GfxTask *GfxTexture::uploadMipMapAsync(const ByteCollection &data, i32 mipIndex, i32 rowPitch, i32 slicePitch, bool copyData)
    {
        PROFILE();
        TK_ASSERT(isAllocated());
        TK_ASSERT(mipIndex < mipLevels() && data.isValid());
        TK_ASSERT(data.length() >= slicePitch);

        // Optimize texture upload invoked during rendering
        if (TkThread::isInMainThread() && GfxDevice::sInstance->isRendering())
        {
            // Update all array slices
            const byte *dataSource = data.get();
            for (i32 arrayIndex = 0; arrayIndex < m_desc.arraySize; arrayIndex++)
            {
                GfxDevice::sInstance->getMainContext()->updateTexture(this, arrayIndex, mipIndex, dataSource, rowPitch, slicePitch);
                dataSource += slicePitch;
            }
            if (mipIndex == highestResidentMipIndex() - 1)
            {
                // Mark as mip loaded
                setResidentMipLevels(residentMipLevels() + 1);
            }
            return nullptr;
        }

        auto task = TkMemory::newObject<GfxUploadTextureMipTask>(TkMemory::getDefault(), this, mipIndex, data, rowPitch, slicePitch, copyData);
        TK_ASSERT(task); // && task->HasReference(this));
        return task;
    }

    bool GfxTexture::uploadData(TextureData &data, bool copyData)
    {
        if (!isAllocated())
            return false;
        if (data.width != width() || data.height != height() || data.depth != depth() || data.getArraySize() != arraySize() || data.format != format())
            return false;
        for (i32 arrayIndex = 0; arrayIndex < arraySize(); arrayIndex++)
        {
            for (i32 mipLevel = 0; mipLevel < mipLevels(); mipLevel++)
            {
                TextureMipData *mip = data.getData(arrayIndex, mipLevel);
                uploadMipMapAsync(mip->Data, mipLevel, mip->RowPitch, mip->DepthPitch, copyData);
            }
        }
        return true;
    }

    class TextureDownloadDataTask : public ThreadPoolTask
    {
    private:
        GfxTextureReference m_texture;
        GfxTexture *m_staging;
        TextureData *m_data;
        bool m_deleteStaging;

    public:
        TextureDownloadDataTask(GfxTexture *texture, GfxTexture *staging, TextureData &data)
            : m_texture(texture), m_staging(staging), m_data(&data)
        {
            m_deleteStaging = texture != staging;
        }

        ~TextureDownloadDataTask()
        {
            if (m_deleteStaging && m_staging)
                TkMemory::deletePtr(m_staging);
        }

    protected:
        // [ThreadPoolTask]
        bool run() override
        {
            auto texture = m_texture.get();
            if (texture == nullptr || m_staging == nullptr || m_data == nullptr)
            {
                LogWarn("Cannot download texture data. Missing objects.");
                return false;
            }
            return m_staging->downloadData(*m_data);
        }

        void onEnd() override
        {
            m_texture.unlink();

            // Base
            ThreadPoolTask::onEnd();
        }
    };

    bool GfxTexture::downloadData(TextureData &result)
    {
        // Skip for empty ones
        if (mipLevels() == 0)
        {
            LogWarn("Cannot download GPU texture data from an empty texture.");
            return true;
        }
        if (depth() != 1)
        {
            TK_ASSERT(false && "support volume texture data downloading.");
        }
        PROFILE();

        // Use faster path for staging resources
        if (isStaging()) // TODO: what about chips with unified memory? if rendering is not active then we can access GPU memory from CPU directly (eg. mobile, integrated GPUs and some consoles)
        {
            const auto arraySize = GfxTexture::arraySize();
            const auto mipLevels = GfxTexture::mipLevels();

            // Set texture info
            result.width = width();
            result.height = height();
            result.depth = depth();
            result.format = format();

            // Get all mip maps for each array slice
            auto &rawResultData = result.items;
            rawResultData.resize(arraySize, false);
            for (i32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
            {
                auto &arraySlice = rawResultData[arrayIndex];
                arraySlice.mips.resize(mipLevels);

                for (i32 mipMapIndex = 0; mipMapIndex < mipLevels; mipMapIndex++)
                {
                    auto &mip = arraySlice.mips[mipMapIndex];
                    const i32 mipWidth = result.width >> mipMapIndex;
                    const i32 mipHeight = result.height >> mipMapIndex;
                    u32 mipRowPitch, mipSlicePitch;
                    GfxUtils::computePitch(result.format, mipWidth, mipHeight, mipRowPitch, mipSlicePitch);

                    // Gather data
                    if (getData(arrayIndex, mipMapIndex, mip, mipRowPitch))
                    {
                        LogWarn("Staging resource of \'%s\' get data failed.", toString());
                        return false;
                    }
                }
            }

            return true;
        }

#if GFX_ENABLE_RESOURCE_NAMING
        const auto name = toString();
#else
        const TkStringView name = "Texture";
#endif

        // Ensure not running on main thread. Texture readback is supported only on worker threads.
        if (TkThread::isInMainThread())
        {
            LogWarn("Downloading GPU texture data from the main thread is not supported.");
            return false;
        }

        // Create async task
        auto task = downloadDataAsync(result);
        if (task == nullptr)
        {
            LogWarn("Cannot create async download task for resource %s.", name);
            return false;
        }

        // Wait for work to be done
        task->start();
        if (task->wait())
        {
            LogWarn("Resource \'%s\' copy failed.", name);
            return false;
        }

        return true;
    }


    TkTask *GfxTexture::downloadDataAsync(TextureData &result)
    {
        // Skip for empty ones
        if (mipLevels() == 0)
        {
            LogWarn("Cannot download texture data. It has not ben created yet.");
            return nullptr;
        }
        if (depth() != 1)
        {
            TK_ASSERT(false && "support volume texture data downloading.");
        }
        PROFILE();

        // Use faster path for staging resources
        if (isStaging())
        {
            // Create task to copy downloaded data to TextureData container
            auto getDataTask = TkMemory::newObject<TextureDownloadDataTask>(TkMemory::getDefault(), this, this, result);
            // TK_ASSERT(getDataTask->HasReference(this));
            return getDataTask;
        }

        // Create the staging resource
        const auto staging = toStagingReadback();
        if (staging == nullptr)
        {
            LogError("Cannot create staging resource from %s.", toString());
            return nullptr;
        }

        // Create async resource copy task
        auto copyTask = TkMemory::newObject<GfxCopyResourceTask>(TkMemory::getDefault(), this, staging);
        // TK_ASSERT(copyTask->HasReference(this) && copyTask->HasReference(staging));

        // Create task to copy downloaded data to TextureData container
        auto getDataTask = TkMemory::newObject<TextureDownloadDataTask>(TkMemory::getDefault(), this, staging, result);
        // TK_ASSERT(getDataTask->HasReference(this) && getDataTask->HasReference(staging));

        // Set continuation
        copyTask->continueWith(getDataTask);

        return copyTask;
    }

    void GfxTexture::setResidentMipLevels(i32 count)
    {
        count = TkMath::clamp(count, 0, mipLevels());
        if (m_residentMipLevels == count || !isRegularTexture())
            return;
        m_residentMipLevels = count;
        onResidentMipsChange();
        onResidentMipsChanged(this);
    }

}
