/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/platforms/vulkan/common.hpp>

#if GFX_API_VULKAN

#include <toolkit/collection/Array.hpp>
#include "toolkit/graphics/GfxDevice.hpp"
#include "toolkit/threading/Sync.hpp"
#include "toolkit/collection/HashMap.hpp"
#include "toolkit/graphics/GfxTexture.hpp"

namespace toolkit
{

    class VulkanSemaphore
    {
    private:
        class VulkanDevice *m_device;
        VkSemaphore m_semaphoreHandle;

    public:
        VulkanSemaphore(VulkanDevice *device);
        ~VulkanSemaphore();

        // Get the handle.
        inline VkSemaphore getHandle() const
        {
            return m_semaphoreHandle;
        }
    };

    struct VulkanFence
    {
        VkFence handle;
        bool isSignaled;
    };

    class VulkanFenceManager
    {
    private:
        VulkanDevice *m_device = nullptr;
        TkArray<VulkanFence *> m_freeFences;
        TkArray<VulkanFence *> m_usedFences;

    public:
        ~VulkanFenceManager();

    public:
        void init(VulkanDevice *device)
        {
            m_device = device;
        }
        void dispose();

        VulkanFence *allocateFence(bool createSignaled = false);
        TK_FORCE_INLINE bool isFenceSignaled(VulkanFence *fence) const
        {
            return fence->isSignaled || checkFenceState(fence);
        }

        // Returns true when the fence is successfully signaled, false on timeout or failure.
        bool waitForFence(VulkanFence *fence, u64 timeInNanoseconds) const;
        void resetFence(VulkanFence *fence) const;

        // Sets the fence handle to null
        void releaseFence(VulkanFence *&fence);
        // Sets the fence handle to null
        void waitAndReleaseFence(VulkanFence *&fence, u64 timeInNanoseconds);

    private:
        // Returns true if fence was signaled, otherwise false.
        bool checkFenceState(VulkanFence *fence) const;
        void destroyFence(VulkanFence *fence) const;
    };

    class VulkanDeferredDeletionQueue
    {
    public:
        enum Type
        {
            kRenderPass,
            kBuffer,
            kBufferView,
            kImage,
            kImageView,
            kPipeline,
            kPipelineLayout,
            kFramebuffer,
            kDescriptorSetLayout,
            kSampler,
            kSemaphore,
            kShaderModule,
            kEvent,
            kQueryPool,
        };

    private:
        struct Entry
        {
            u64 fenceCounter;
            u64 handle;
            u64 frameNumber;
            VmaAllocation allocationHandle;
            Type structureType;
            class VulkanCmdBuffer *cmdBuffer;
        };

        VulkanDevice *m_device;
        TkMutex m_locker;
        TkArray<Entry> m_entries;

    public:
        VulkanDeferredDeletionQueue(VulkanDevice *device);
        ~VulkanDeferredDeletionQueue();

    public:
        template <typename T>
        inline void enqueueResource(Type type, T handle)
        {
            static_assert(sizeof(T) <= sizeof(u64), "Invalid handle size.");
            enqueueGenericResource(type, (u64)handle, VK_NULL_HANDLE);
        }

        template <typename T>
        inline void enqueueResource(Type type, T handle, VmaAllocation allocation)
        {
            static_assert(sizeof(T) <= sizeof(u64), "Invalid handle size.");
            enqueueGenericResource(type, (u64)handle, allocation);
        }
        void releaseResources(bool immediately = false);

    private:
        void enqueueGenericResource(Type type, u64 handle, VmaAllocation allocation);
    };

    struct VulkanRenderTargetLayout
    {
        union
        {
            struct
            {
                u32 layers : 10;  // Limited by GFX_MAX_TEXTURE_ARRAY_SIZE
                u32 rtsCount : 3; // Limited by GFX_MAX_RT_BINDED
                u32 readDepth : 1;
                u32 writeDepth : 1;
                u32 readStencil : 1;
                u32 writeStencil : 1;
                u32 blendEnable : 1;
            };

            u32 flags;
        };

        GfxMSAALevel msaa;
        PixelFormat depthFormat;
        PixelFormat rtvsFormats[GFX_MAX_RT_BINDED];
        VkExtent2D extent;

        TK_FORCE_INLINE bool operator==(const VulkanRenderTargetLayout &other) const
        {
            return TkMemory::memoryCompare(this, &other, sizeof(VulkanRenderTargetLayout)) == 0;
        }
    };

    u32 GetHash(const VulkanRenderTargetLayout &key);

    class VulkanFramebuffer
    {
    public:
        struct Key
        {
            class VulkanRenderPass *renderPass;
            i32 attachmentCount;
            VkImageView attachments[GFX_MAX_RT_BINDED + 1];

            TK_FORCE_INLINE bool operator==(const Key &other) const
            {
                return TkMemory::memoryCompare(this, &other, sizeof(Key)) == 0;
            }
        };

        VulkanFramebuffer(VulkanDevice *device, const Key &key, const VkExtent2D &extent, u32 layers);
        ~VulkanFramebuffer();

        VulkanDevice *device;
        VkFramebuffer handle;
        VkImageView attachments[GFX_MAX_RT_BINDED + 1];
        VkExtent2D extent;
        u32 layers;

        bool hasReference(VkImageView imageView) const;
    };
    u32 getHash(const VulkanFramebuffer::Key &key);

    class VulkanRenderPass
    {
    public:
        VulkanDevice *device;
        VkRenderPass handle;
        VulkanRenderTargetLayout layout;
        bool canDepthWrite;
#if VK_USE_DEBUG_DATA
        VkRenderPassCreateInfo debugCreateInfo;
#endif

        VulkanRenderPass(VulkanDevice *device, const VulkanRenderTargetLayout &layout);
        ~VulkanRenderPass();
    };

    class VulkanQueryPool
    {
    protected:
        struct Range
        {
            u32 start;
            u32 count;
        };

        VulkanDevice *m_device;
        VkQueryPool m_handle;

        const VkQueryType m_type;
#if VK_RESET_QUERY_POOLS
        TkArray<Range> m_resetRanges;
#endif

    public:
        VulkanQueryPool(VulkanDevice *device, i32 capacity, VkQueryType type);
        ~VulkanQueryPool();

    public:
        inline VkQueryPool getHandle() const
        {
            return m_handle;
        }

#if VK_RESET_QUERY_POOLS
        bool resetBeforeUse;
        void reset(VulkanCmdBuffer *cmdBuffer);
#endif
    };

    class VulkanBufferedQueryPool : public VulkanQueryPool
    {
    private:
        TkArray<u64> m_queryOutput;
        TkArray<u64> m_usedQueryBits;
        TkArray<u64> m_startedQueryBits;
        TkArray<u64> m_readResultsBits;

        // Last potentially free index in the pool
        i32 m_lastBeginIndex;

    public:
        VulkanBufferedQueryPool(VulkanDevice *device, i32 capacity, VkQueryType type);
        // Returns true when a query slot was acquired, false when the pool is full.
        bool acquireQuery(VulkanCmdBuffer *cmdBuffer, u32 &resultIndex);
        void releaseQuery(u32 queryIndex);
        void markQueryAsStarted(u32 queryIndex);
        // Returns true when the result is available, false when it is not ready or an error occurred.
        bool getResults(class VulkanContext *context, u32 index, u64 &result);
        bool hasRoom() const;
    };

    // The dummy Vulkan resources manager. Helps when user need to pass null texture handle to the shader.
    class VulkanResourcesHelper
    {
    private:
        VulkanDevice *m_device;
        class VulkanTexture *m_dummyTextures[6];
        class VulkanBuffer **m_dummyBuffers = nullptr;
        VulkanBuffer *m_dummyVB = nullptr;
        GfxConstantBuffer *m_dummyCB = nullptr;
        VkSampler m_staticSamplers[GFX_STATIC_SAMPLERS_COUNT];

    public:
        VulkanResourcesHelper(VulkanDevice *device);

    public:
        VkSampler *getStaticSamplers();
        VulkanTexture *getDummyTexture(SpirvShaderResourceType type);
        VulkanBuffer *getDummyBuffer(PixelFormat format);
        VulkanBuffer *getDummyVertexBuffer();
        GfxConstantBuffer *getDummyConstantBuffer();
        void dispose();
    };

    // Vulkan staging buffers manager.
    class VulkanStagingManager
    {
    private:
        struct PendingEntry
        {
            GfxBuffer *buffer;
            VulkanCmdBuffer *cmdBuffer;
            u64 fenceCounter;
        };

        struct FreeEntry
        {
            GfxBuffer *buffer;
            u64 frameNumber;
        };

        VulkanDevice *m_device;
        TkMutex m_locker;
        TkArray<GfxBuffer *> m_allBuffers;
        TkArray<FreeEntry> m_freeBuffers;
        TkArray<PendingEntry> m_pendingBuffers;
#if TK_DEBUG
        u64 m_allBuffersTotalSize = 0;
        u64 m_allBuffersPeekSize = 0;
        u64 m_allBuffersAllocSize = 0;
        u64 m_allBuffersFreeSize = 0;
#endif

    public:
        VulkanStagingManager(VulkanDevice *device);
        GfxBuffer *acquireBuffer(u32 size, GfxResourceUsage usage);
        void releaseBuffer(VulkanCmdBuffer *cmdBuffer, GfxBuffer *&buffer);
        void processPendingFree();
        void dispose();
    };

    // Implementation of Graphics Device for Vulkan backend.
    class VulkanDevice : public GfxDevice
    {
        friend VulkanContext;
        friend class VulkanSwapChain;
        friend VulkanFenceManager;
        friend class VulkanTimerQuery;

    private:
        TkMutex m_fenceLock;
        mutable void *m_nativePtr[2];

        TkHashMap<VulkanRenderTargetLayout, VulkanRenderPass *> m_renderPasses;
        TkHashMap<VulkanFramebuffer::Key, VulkanFramebuffer *> m_framebuffers;
        TkHashMap<class VulkanDescriptorSetLayoutInfo, class VulkanPipelineLayout *> m_layouts;
        // TODO: use mutex to protect those collections BUT use 2 pools per cache: one lock-free with lookup only and second protected with mutex synced on frame end!

    public:
        static GfxDevice *create();

        VulkanDevice(GfxShaderProfile shaderProfile, class VulkanAdapter *adapter);
        ~VulkanDevice();

    public:
        struct OptionalVulkanDeviceExtensions
        {
            u32 hasKHRMaintenance1 : 1;
            u32 hasKHRMaintenance2 : 1;
            u32 hasMirrorClampToEdge : 1;
#if VK_USE_VALIDATION_CACHE
            u32 hasEXTValidationCache : 1;
#endif
        };

        static OptionalVulkanDeviceExtensions optionalDeviceExtensions;

    private:
        static void getInstanceLayersAndExtensions(TkArray<const char *> &outInstanceExtensions, TkArray<const char *> &outInstanceLayers, bool &outDebugUtils);
        void getDeviceExtensionsAndLayers(VkPhysicalDevice gpu, TkArray<const char *> &outDeviceExtensions, TkArray<const char *> &outDeviceLayers);
        static void parseOptionalDeviceExtensions(const TkArray<const char *> &deviceExtensions);

    public:
        // The Vulkan instance.
        static VkInstance instance;
        // The Vulkan instance extensions.
        static TkArray<const char *> instanceExtensions;
        // The Vulkan instance layers.
        static TkArray<const char *> instanceLayers;

    public:
        // The main Vulkan commands context.
        VulkanContext *mainContext = nullptr;
        // The Vulkan adapter.
        VulkanAdapter *adapter = nullptr;
        // The Vulkan device.
        VkDevice device = VK_NULL_HANDLE;
        // The Vulkan device queues family properties.
        TkArray<VkQueueFamilyProperties> queueFamilyProps;
        // The Vulkan fence manager.
        VulkanFenceManager fenceManager;
        // The Vulkan resources deferred deletion queue.
        VulkanDeferredDeletionQueue deferredDeletionQueue;
        // The staging buffers manager.
        VulkanStagingManager stagingManager;
        // The helper device resources manager.
        VulkanResourcesHelper helperResources;
        // The graphics queue.
        class VulkanQueue *graphicsQueue = nullptr;
        // The compute queue.
        VulkanQueue *computeQueue = nullptr;
        // The transfer queue.
        VulkanQueue *transferQueue = nullptr;
        // The present queue.
        VulkanQueue *presentQueue = nullptr;
        // The Vulkan memory allocator.
        VmaAllocator allocator = VK_NULL_HANDLE;
        // The pipeline cache.
        VkPipelineCache pipelineCache = VK_NULL_HANDLE;

#if VK_USE_VALIDATION_CACHE
        // The optional validation cache.
        VkValidationCacheEXT validationCache = VK_NULL_HANDLE;
#endif

        // The uniform buffers uploader.
        class VulkanUniformBufferUploader *uniformBufferUploader = nullptr;
        // The descriptor pools manager.
        class VulkanDescriptorPoolsManager *descriptorPoolsManager = nullptr;
        // The physical device limits.
        VkPhysicalDeviceLimits physicalDeviceLimits;
        // The physical device enabled features.
        VkPhysicalDeviceFeatures physicalDeviceFeatures;
        TkArray<VulkanBufferedQueryPool *> timestampQueryPools;

#if VK_RESET_QUERY_POOLS
        TkArray<VulkanQueryPool *> queriesToReset;
#endif

        inline VulkanBufferedQueryPool *findAvailableQueryPool(TkArray<VulkanBufferedQueryPool *> &pools, VkQueryType queryType)
        {
            // Try to use pool with available space inside
            for (i32 i = 0; i < pools.count(); i++)
            {
                auto pool = pools.get()[i];
                if (pool->hasRoom())
                    return pool;
            }

            // Create new pool
            enum
            {
                NUM_OCCLUSION_QUERIES_PER_POOL = 4096,
                NUM_TIMESTAMP_QUERIES_PER_POOL = 1024,
            };
            const auto pool = TkMemory::newObject<VulkanBufferedQueryPool>(TkMemory::getDefault(), this, queryType == VK_QUERY_TYPE_OCCLUSION ? NUM_OCCLUSION_QUERIES_PER_POOL : NUM_TIMESTAMP_QUERIES_PER_POOL, queryType);
            pools.add(pool);
            return pool;
        }

        inline VulkanBufferedQueryPool *findAvailableTimestampQueryPool()
        {
            return findAvailableQueryPool(timestampQueryPools, VK_QUERY_TYPE_TIMESTAMP);
        }

        VulkanRenderPass *getOrCreateRenderPass(VulkanRenderTargetLayout &layout);
        VulkanFramebuffer *getOrCreateFramebuffer(VulkanFramebuffer::Key &key, VkExtent2D &extent, u32 layers);
        VulkanPipelineLayout *getOrCreateLayout(VulkanDescriptorSetLayoutInfo &key);
        void onImageViewDestroy(VkImageView imageView);

    public:
        // Setups the present queue to be ready for the given window surface.
        void setupPresentQueue(VkSurfaceKHR surface);
        // Finds the closest pixel format that a specific Vulkan device supports.
        PixelFormat getClosestSupportedPixelFormat(PixelFormat format, GfxTextureFlags flags, bool optimalTiling);
        // Returns true when the pipeline cache was saved, false on failure.
        bool savePipelineCache();
#if VK_USE_VALIDATION_CACHE
        // Returns true when the validation cache was saved, false on failure.
        bool saveValidationCache();
#endif

    public:
        bool isVkFormatSupported(VkFormat vkFormat, VkFormatFeatureFlags wantedFeatureFlags, bool optimalTiling) const;

        GfxContext *getMainContext() override;
        GfxAdapter *getAdapter() const override;
        void *getNativePtr() const override;
        bool init() override;
        void drawBegin() override;
        void dispose() override;
        void waitForGPU() override;
        GfxTexture *createTexture(const TkStringView &name) override;
        GfxShader *createShader(const TkStringView &name) override;
        GfxPipelineState *createPipelineState() override;
        GfxTimerQuery *createTimerQuery() override;
        GfxBuffer *createBuffer(const TkStringView &name) override;
        GfxSampler *createSampler() override;
        GfxVertexLayout *createVertexLayout(const VertexElements &elements, bool explicitOffsets) override;
        GfxSwapChain *createSwapChain(GfxWindow *window) override;
        GfxConstantBuffer *createConstantBuffer(u32 size, const TkStringView &name) override;
    };

    // GPU resource implementation for Vulkan backend
    template <class BaseType>
    class VulkanResource : public GfxResourceBase<VulkanDevice, BaseType>
    {
    public:
        VulkanResource(VulkanDevice *device, const TkStringView &name)
            : GfxResourceBase<VulkanDevice, BaseType>(device, name)
        {
        }
    };

    // Represents a GPU resource that contain descriptor resource for binding to the pipeline (shader resource, sampler, buffer, etc.)
    class VulkanDescriptorOwnerResource
    {
    public:
        virtual ~VulkanDescriptorOwnerResource() = default;

    public:
        // Get the sampler descriptor.
        virtual void descriptorAsSampler(VulkanContext *context, VkSampler &sampler)
        {
            TK_ASSERT(false && "VulkanDescriptorOwnerResource descriptorAsSampler");
        }

        // Get the image descriptor.
        virtual void descriptorAsImage(VulkanContext *context, VkImageView &imageView, VkImageLayout &layout)
        {
            TK_ASSERT(false && "VulkanDescriptorOwnerResource descriptorAsImage");
        }

        // Get the storage image descriptor (VK_DESCRIPTOR_TYPE_STORAGE_IMAGE).
        virtual void descriptorAsStorageImage(VulkanContext *context, VkImageView &imageView, VkImageLayout &layout)
        {
            TK_ASSERT(false && "VulkanDescriptorOwnerResource descriptorAsStorageImage");
        }

        // Get the uniform texel buffer descriptor (VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER).
        virtual void descriptorAsUniformTexelBuffer(VulkanContext *context, VkBufferView &bufferView)
        {
            TK_ASSERT(false && "VulkanDescriptorOwnerResource descriptorAsUniformTexelBuffer");
        }

        // Get the storage buffer descriptor (VK_DESCRIPTOR_TYPE_STORAGE_BUFFER).
        virtual void descriptorAsStorageBuffer(VulkanContext *context, VkBuffer &buffer, VkDeviceSize &offset, VkDeviceSize &range)
        {
            TK_ASSERT(false && "VulkanDescriptorOwnerResource descriptorAsStorageBuffer");
        }

        // Get the storage texel buffer descriptor (VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER).
        virtual void descriptorAsStorageTexelBuffer(VulkanContext *context, VkBufferView &bufferView)
        {
            TK_ASSERT(false && "VulkanDescriptorOwnerResource descriptorAsStorageTexelBuffer");
        }

        // Get the dynamic uniform buffer descriptor (VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC).
        virtual void descriptorAsDynamicUniformBuffer(VulkanContext *context, VkBuffer &buffer, VkDeviceSize &offset, VkDeviceSize &range, u32 &dynamicOffset)
        {
            TK_ASSERT(false && "VulkanDescriptorOwnerResource descriptorAsDynamicUniformBuffer");
        }

#if TK_DEBUG
        // Utilities for incorrect resource usage.
        virtual bool hasSRV() const { return false; }
        virtual bool hasUAV() const { return false; }
#endif
    };

    extern GfxDevice *createVulkanDevice();

}

#endif
