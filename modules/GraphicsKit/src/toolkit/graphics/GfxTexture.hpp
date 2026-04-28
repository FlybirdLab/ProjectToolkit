/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/graphics/GfxResource.hpp"
#include "toolkit/graphics/utils/PixelFormat.hpp"
#include "toolkit/utils/Color.hpp"
#include <toolkit/graphics/common.hpp>

namespace toolkit
{

    typedef TkDataCollection<byte> ByteCollection;

    // GPU texture usage flags.
    enum class GfxTextureFlags
    {
        // No texture flags.
        kNone = 0x0000,
        // Create a texture that can be bound as a shader resource.
        kShaderResource = 0x0001,
        // Create a texture that can be bound as a render target.
        kRenderTarget = 0x0002,
        // Create a texture can be bound as an unordered access buffer.
        kUnorderedAccess = 0x0004,
        // Create a texture can be bound as a depth stencil buffer.
        kDepthStencil = 0x0008,
        // Create texture views per texture mip map (valid only for Texture2D with ShaderResource or RenderTarget flag).
        kPerMipViews = 0x0010,
        // Create texture views per texture slice map (valid only for Texture3D with ShaderResource or RenderTarget flag).
        kPerSliceViews = 0x0020,
        // Create read-only view for depth-stencil buffer. Valid only if texture uses depth-stencil and the graphics device supports it.
        kReadOnlyDepthView = 0x0040,
        // Create a texture that can be used as a native window swap chain backbuffer surface.
        kBackBuffer = 0x0080,
    };
    TK_DECLARE_ENUM_OPERATORS(GfxTextureFlags);

    // Defines the dimension of a texture object
    enum class TextureDimensions
    {
        // The texture (2d).
        kTexture,
        // The volume texture (3d texture).
        kVolumeTexture,
        // The cube texture (2d texture array of 6 items).
        kCubeTexture,
    };

    // A common description for all GPU textures
    struct TK_API GfxTextureDescription
    {
    public:
        // The dimensions of the texture.
        TextureDimensions dimensions;
        // Texture width (in texels).
        i32 width;
        // Texture height (in texels).
        i32 height;
        // Texture depth (in texels) for Volume Textures.
        i32 depth;
        // Number of textures in array for Texture Arrays.
        i32 arraySize;
        // The maximum number of mipmap levels in the texture. Use 1 for a multisampled texture; or 0 to generate a full set of subtextures.
        i32 mipLevels;
        // Texture format .
        PixelFormat format;
        // Structure that specifies multisampling parameters for the texture.
        GfxMSAALevel multiSampleLevel;
        // Flags for binding to pipeline stages. The flags can be combined by a logical OR.
        GfxTextureFlags flags;
        // Value that identifies how the texture is to be read from and written to. The most common value is <see cref="GfxResourceUsage.Default"/>; see <strong><see cref="GfxResourceUsage"/></strong> for all possible values.
        GfxResourceUsage usage;
        // Default clear color for render targets
        Color defaultClearColor;

    public:
        // Get a value indicating whether this instance is a render target.
        TK_FORCE_INLINE bool isRenderTarget() const
        {
            return (flags & GfxTextureFlags::kRenderTarget) != GfxTextureFlags::kNone;
        }

        // Get a value indicating whether this instance is a depth stencil.
        TK_FORCE_INLINE bool isDepthStencil() const
        {
            return (flags & GfxTextureFlags::kDepthStencil) != GfxTextureFlags::kNone;
        }

        // Get a value indicating whether this instance is a shader resource.
        TK_FORCE_INLINE bool isShaderResource() const
        {
            return (flags & GfxTextureFlags::kShaderResource) != GfxTextureFlags::kNone;
        }

        // Get a value indicating whether this instance is a unordered access.
        TK_FORCE_INLINE bool isUnorderedAccess() const
        {
            return (flags & GfxTextureFlags::kUnorderedAccess) != GfxTextureFlags::kNone;
        }

        // Get a value indicating whether this instance has per mip level handles.
        TK_FORCE_INLINE bool hasPerMipViews() const
        {
            return (flags & GfxTextureFlags::kPerMipViews) != GfxTextureFlags::kNone;
        }

        // Get a value indicating whether this instance has per slice views.
        TK_FORCE_INLINE bool hasPerSliceViews() const
        {
            return (flags & GfxTextureFlags::kPerSliceViews) != GfxTextureFlags::kNone;
        }

        // Get a value indicating whether this instance is a multi sample texture.
        TK_FORCE_INLINE bool isMultiSample() const
        {
            return multiSampleLevel > GfxMSAALevel::kNone;
        }

        // Get a value indicating whether this instance is a cubemap texture.
        TK_FORCE_INLINE bool isCubeMap() const
        {
            return dimensions == TextureDimensions::kCubeTexture;
        }

        // Get a value indicating whether this instance is a volume texture.
        TK_FORCE_INLINE bool isVolume() const
        {
            return dimensions == TextureDimensions::kVolumeTexture;
        }

        // Get a value indicating whether this instance is an array texture.
        TK_FORCE_INLINE bool isArray() const
        {
            return arraySize != 1;
        }

    public:
        // Create a new 1D texture desc with a single mipmap.
        static GfxTextureDescription new1D(i32 width, PixelFormat format, GfxTextureFlags textureFlags = GfxTextureFlags::kShaderResource | GfxTextureFlags::kRenderTarget, i32 arraySize = 1)
        {
            return new1D(width, format, textureFlags, 1, arraySize);
        }

        // Create a new 1D texture desc.
        static GfxTextureDescription new1D(i32 width, i32 mipCount, PixelFormat format, GfxTextureFlags textureFlags = GfxTextureFlags::kShaderResource | GfxTextureFlags::kRenderTarget, i32 arraySize = 1)
        {
            return new1D(width, format, textureFlags, mipCount, arraySize);
        }

        // Create a new 1D texture desc.
        static GfxTextureDescription new1D(i32 width, PixelFormat format, GfxTextureFlags textureFlags, i32 mipCount, i32 arraySize);

    public:
        // Create a new texture desc with a single mipmap.
        static GfxTextureDescription new2D(i32 width, i32 height, PixelFormat format, GfxTextureFlags textureFlags = GfxTextureFlags::kShaderResource | GfxTextureFlags::kRenderTarget, i32 arraySize = 1)
        {
            return new2D(width, height, 1, format, textureFlags, arraySize);
        }

        // Create a new texture desc.
        static GfxTextureDescription new2D(i32 width, i32 height, i32 mipCount, PixelFormat format, GfxTextureFlags textureFlags = GfxTextureFlags::kShaderResource | GfxTextureFlags::kRenderTarget, i32 arraySize = 1, GfxMSAALevel msaaLevel = GfxMSAALevel::kNone)
        {
            return new2D(width, height, format, textureFlags, mipCount, arraySize, msaaLevel);
        }

        // Create a new texture desc.
        static GfxTextureDescription new2D(i32 width, i32 height, PixelFormat format, GfxTextureFlags textureFlags, i32 mipCount, i32 arraySize, GfxMSAALevel msaaLevel = GfxMSAALevel::kNone);

    public:
        // Create a new texture desc with a single mipmap.
        static GfxTextureDescription new3D(const FVec3 &size, PixelFormat format, GfxTextureFlags textureFlags = GfxTextureFlags::kShaderResource | GfxTextureFlags::kRenderTarget);

        // Create a new texture desc with a single mipmap.
        static GfxTextureDescription new3D(i32 width, i32 height, i32 depth, PixelFormat format, GfxTextureFlags textureFlags = GfxTextureFlags::kShaderResource | GfxTextureFlags::kRenderTarget)
        {
            return new3D(width, height, depth, 1, format, textureFlags);
        }

        // Create a new texture desc.
        static GfxTextureDescription new3D(i32 width, i32 height, i32 depth, i32 mipCount, PixelFormat format, GfxTextureFlags textureFlags = GfxTextureFlags::kShaderResource | GfxTextureFlags::kRenderTarget)
        {
            return new3D(width, height, depth, format, textureFlags, mipCount);
        }

        // Create a new texture desc.
        static GfxTextureDescription new3D(i32 width, i32 height, i32 depth, PixelFormat format, GfxTextureFlags textureFlags, i32 mipCount);

    public:
        // Create a new Cube texture desc.
        static GfxTextureDescription newCube(i32 size, PixelFormat format, GfxTextureFlags textureFlags = GfxTextureFlags::kShaderResource | GfxTextureFlags::kRenderTarget)
        {
            return newCube(size, 1, format, textureFlags);
        }

        // Create a new Cube <see cref="GfxTextureDescription"/>.
        static GfxTextureDescription newCube(i32 size, i32 mipCount, PixelFormat format, GfxTextureFlags textureFlags = GfxTextureFlags::kShaderResource | GfxTextureFlags::kRenderTarget)
        {
            return newCube(size, format, textureFlags, mipCount);
        }

        // Create a new Cube <see cref="GfxTextureDescription"/>.
        static GfxTextureDescription newCube(i32 size, PixelFormat format, GfxTextureFlags textureFlags, i32 mipCount);

    public:
        void clear();
        GfxTextureDescription toStagingUpload() const;
        GfxTextureDescription toStagingReadback() const;
        GfxTextureDescription toStaging() const;
        bool equals(const GfxTextureDescription &other) const;
        TkString toString() const;

    public:
        TK_FORCE_INLINE bool operator==(const GfxTextureDescription &other) const
        {
            return equals(other);
        }

        TK_FORCE_INLINE bool operator!=(const GfxTextureDescription &other) const
        {
            return !equals(other);
        }
    };

    u32 getHash(const GfxTextureDescription &key);
    template <>
    struct HashFunc<GfxTextureDescription>
    {
        static u32 get(const GfxTextureDescription &k)
        {
            return getHash(k);
        }
    };

    // Defines a view for the texture surface, full resource or any of the sub-parts. Can be used to define a single subresource of the texture, volume texture or texture array. Used to render to the texture and/or use textures in the shaders.
    class TK_API GfxTextureView : public GfxResourceView
    {
    protected:
        PixelFormat m_format = PixelFormat::kUnknown;
        GfxMSAALevel m_msaa = GfxMSAALevel::kNone;

        GfxTextureView()
            : GfxResourceView {}
        {
        }

        TK_FORCE_INLINE void init(GfxResource *parent, PixelFormat format, GfxMSAALevel msaa)
        {
            m_parent = parent;
            m_format = format;
            m_msaa = msaa;
            if (parent)
                lastRenderTime = &parent->lastRenderTime;
        }

    public:
        // Get the view format.
        TK_FORCE_INLINE PixelFormat getFormat() const
        {
            return m_format;
        }

        // Get view MSAA level.
        TK_FORCE_INLINE GfxMSAALevel getMSAA() const
        {
            return m_msaa;
        }
    };

    // <summary>
    // The GPU texture resource object. This class is able to create 2D/3D textures, volume textures and render targets.
    // </summary>
    class TK_API GfxTexture : public GfxResource
    {
        static GfxTexture *newTexture();

    protected:
        GfxTexture();

    public:
        // Get a value indicating whether this texture has any resided mip (data already uploaded to the GPU).
        TK_FORCE_INLINE bool hasResidentMip() const
        {
            return m_residentMipLevels != 0;
        }

        // Get a value indicating whether this texture has been allocated.
        TK_FORCE_INLINE bool isAllocated() const
        {
            return m_desc.mipLevels > 0;
        }

        // Get texture width (in texels).
        TK_FORCE_INLINE i32 width() const
        {
            return m_desc.width;
        }

        // Get texture height (in texels).
        TK_FORCE_INLINE i32 height() const
        {
            return m_desc.height;
        }

        // Get texture depth (in texels).
        TK_FORCE_INLINE i32 depth() const
        {
            return m_desc.depth;
        }

        // Get number of textures in the array.
        TK_FORCE_INLINE i32 arraySize() const
        {
            return m_desc.arraySize;
        }

        // Get multi-sampling parameters for the texture.
        TK_FORCE_INLINE GfxMSAALevel multiSampleLevel() const
        {
            return m_desc.multiSampleLevel;
        }

        // Get number of mipmap levels in the texture.
        TK_FORCE_INLINE i32 mipLevels() const
        {
            return m_desc.mipLevels;
        }

        // Get the number of resident mipmap levels in the texture (already uploaded to the GPU).
        TK_FORCE_INLINE i32 residentMipLevels() const
        {
            return m_residentMipLevels;
        }

        // Get the index of the highest resident mip map (may be equal to MipLevels if no mip has been uploaded). Note: mip=0 is the highest (top quality).
        TK_FORCE_INLINE i32 highestResidentMipIndex() const
        {
            return mipLevels() - residentMipLevels();
        }

        // Get texture data format.
        TK_FORCE_INLINE PixelFormat format() const
        {
            return m_desc.format;
        }

        // Get flags of the texture.
        TK_FORCE_INLINE GfxTextureFlags flags() const
        {
            return m_desc.flags;
        }

        // Get texture dimensions.
        TK_FORCE_INLINE TextureDimensions dimensions() const
        {
            return m_desc.dimensions;
        }

        // Get texture description structure.
        TK_FORCE_INLINE const GfxTextureDescription &getDescription() const
        {
            return m_desc;
        }

    public:
        // Get a value indicating whether this texture is a render target.
        TK_FORCE_INLINE bool isRenderTarget() const
        {
            return m_desc.isRenderTarget();
        }

        // Get a value indicating whether this texture is a shader resource.
        TK_FORCE_INLINE bool isShaderResource() const
        {
            return m_desc.isShaderResource();
        }

        // Get a value indicating whether this texture is a depth stencil.
        TK_FORCE_INLINE bool isDepthStencil() const
        {
            return m_desc.isDepthStencil();
        }

        // Get a value indicating whether this texture is a unordered access.
        TK_FORCE_INLINE bool isUnorderedAccess() const
        {
            return m_desc.isUnorderedAccess();
        }

        // Get a value indicating whether this instance has per mip level views.
        TK_FORCE_INLINE bool hasPerMipViews() const
        {
            return m_desc.hasPerMipViews();
        }

        // Get a value indicating whether this instance has per slice views.
        TK_FORCE_INLINE bool hasPerSliceViews() const
        {
            return m_desc.hasPerSliceViews();
        }

        // Get a value indicating whether this instance is a multi sample texture.
        TK_FORCE_INLINE bool isMultiSample() const
        {
            return m_desc.isMultiSample();
        }

        // Get a value indicating whether this instance is a cubemap texture.
        TK_FORCE_INLINE bool isCubeMap() const
        {
            return m_desc.dimensions == TextureDimensions::kCubeTexture;
        }

        // Get a value indicating whether this instance is a volume texture.
        TK_FORCE_INLINE bool isVolume() const
        {
            return m_desc.dimensions == TextureDimensions::kVolumeTexture;
        }

        // Get a value indicating whether this instance is an array texture.
        TK_FORCE_INLINE bool isArray() const
        {
            return m_desc.arraySize != 1;
        }

        // Checks if texture contains sRGB colors data.
        TK_FORCE_INLINE bool isSRGB() const
        {
            return m_sRGB;
        }

        // Checks if texture is normal texture asset (not render target or unordered access or depth buffer or sth else).
        TK_FORCE_INLINE bool isRegularTexture() const
        {
            return m_desc.flags == GfxTextureFlags::kShaderResource;
        }

        // Checks if texture is a staging buffer (supports direct CPU access).
        bool isStaging() const;

        // Get a boolean indicating whether this texture is a using a block compress format (BC1, BC2, BC3, BC4, BC5, BC6H, BC7, etc.).
        TK_FORCE_INLINE bool isBlockCompressed() const
        {
            return m_isBlockCompressed;
        }

    public:
        // Get the texture total size in pixels.
        FVec2 size() const;

        // Get the texture total size in pixels (with depth).
        FVec3 size3() const;

        // Returns true if texture has size that is power of two.
        bool isPowerOfTwo() const;

        // Get the texture mip map dimensions.
        void getMipSize(i32 mipLevelIndex, i32 &mipWidth, i32 &mipHeight) const;
        // Get the texture mip map dimensions.
        void getMipSize(i32 mipLevelIndex, i32 &mipWidth, i32 &mipHeight, i32 &mipDepth) const;

        // Get current texture size (uploaded to the GPU and in use).
        void getResidentSize(i32 &width, i32 &height) const;
        // Get current texture size (uploaded to the GPU and in use).
        void getResidentSize(i32 &width, i32 &height, i32 &depth) const;

    public:
        // Calculates mip map row pitch (in bytes).
        u32 rowPitch(i32 mipIndex = 0) const;
        // Calculates mip map slice pitch (in bytes).
        u32 slicePitch(i32 mipIndex = 0) const;
        // Computes row and slice pitch of the mip map.
        void computePitch(i32 mipIndex, u32 &rowPitch, u32 &slicePitch) const;
        // Calculates the size of a particular mip.
        i32 calculateMipSize(i32 size, i32 mipLevel) const;

    public:
        i32 computeSubresourceSize(i32 subresource, i32 rowAlign, i32 sliceAlign) const;
        i32 computeBufferOffset(i32 subresource, i32 rowAlign, i32 sliceAlign) const;
        i32 computeBufferTotalSize(i32 rowAlign, i32 sliceAlign) const;
        i32 computeSlicePitch(i32 mipLevel, i32 rowAlign) const;
        i32 computeRowPitch(i32 mipLevel, i32 rowAlign) const;

    public:
        // Get the view to the first surface (only for 2D textures).
        TK_FORCE_INLINE GfxTextureView *view() const
        {
            return view(0);
        }
        // Get the view to the surface at index in an array.
        virtual GfxTextureView *view(i32 arrayOrDepthIndex) const = 0;
        // Get the view to the mip map surface at index in an array.
        virtual GfxTextureView *view(i32 arrayOrDepthIndex, i32 mipMapIndex) const = 0;

        // Get the view to the array of surfaces
        virtual GfxTextureView *viewArray() const = 0;
        // Get the view to the volume texture (3D).
        virtual GfxTextureView *viewVolume() const = 0;
        // Get the view to the texture as read-only depth/stencil buffer. Valid only if graphics device supports it and the texture uses depth/stencil.
        virtual GfxTextureView *viewReadOnlyDepth() const = 0;

        // Implicit conversion to the first surface (only for 2D textures).
        // <returns>The view to the main texture surface.</returns>
        TK_FORCE_INLINE operator GfxTextureView *() const
        {
            return view(0);
        }

    public:
        // Init a texture resource (allocates the GPU memory and performs the resource setup).
        bool init(const GfxTextureDescription &desc);

        // Create new staging readback texture with the same dimensions and properties as a source texture (but without a data transferred; warning: caller must delete object).
        GfxTexture *toStagingReadback() const;
        // Create new staging upload texture with the same dimensions and properties as a source texture (but without a data transferred; warning: caller must delete object).
        GfxTexture *toStagingUpload() const;

        // Resize the texture. It must be created first.
        bool resize(i32 width, i32 height, PixelFormat format = PixelFormat::kUnknown)
        {
            const auto depth = isAllocated() ? this->depth() : 1;
            return resize(width, height, depth, format);
        }
        // Resize the texture. It must be created first.
        bool resize(i32 width, i32 height, i32 depth, PixelFormat format = PixelFormat::kUnknown);

    public:
        // Event called when texture residency gets changed. Texture Mip gets loaded into GPU memory and is ready to use.
        TkAction<GfxTexture *> onResidentMipsChanged;

        // Get the native pointer to the underlying resource. It's a low-level platform-specific handle.
        virtual void *getNativePtr() const = 0;
        
        // Uploads mip map data to the GPU. Create async GPU task.
        class GfxTask *uploadMipMapAsync(const ByteCollection &data, i32 mipIndex, bool copyData = false);
        // Uploads mip map data to the GPU. Create async GPU task.
        GfxTask *uploadMipMapAsync(const ByteCollection &data, i32 mipIndex, i32 rowPitch, i32 slicePitch, bool copyData = false);
        // Uploads texture data to the GPU. Actual data copy to the GPU memory is performed via async task.
        bool uploadData(class TextureData &data, bool copyData = false);
        
        // Downloads the texture data to be accessible from CPU. For frequent access, use staging textures, otherwise current thread will be stalled to wait for the GPU frame to copy data into staging buffer.
        bool downloadData(TextureData &result);
        // Create GPU async task that will gather texture data from the GPU.
        class TkTask *downloadDataAsync(TextureData &result);

        // Get texture mipmap data (raw bytes). Can be used only with textures created with Staging flag.
        virtual bool getData(i32 arrayIndex, i32 mipMapIndex, class TextureMipData &data, u32 mipRowPitch = 0) = 0;
        // Sets the number of resident mipmap levels in the texture (already uploaded to the GPU).
        void setResidentMipLevels(i32 count);


        GfxResType getType() const final override;

    protected:
        virtual bool onInit() = 0;
        u64 calculateMemoryUsage() const;
        virtual void onResidentMipsChange() = 0;
        void onReleaseData() override;
        TkString toString() const;

    protected:
        i32 m_residentMipLevels;
        bool m_sRGB, m_isBlockCompressed;
        GfxTextureDescription m_desc;
    };

}