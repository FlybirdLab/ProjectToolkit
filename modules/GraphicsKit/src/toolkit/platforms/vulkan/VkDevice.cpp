/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "VkDevice.hpp"
#include "toolkit/platforms/vulkan/VkAdapter.hpp"
#include "toolkit/platforms/vulkan/VkSampler.hpp"
#include "toolkit/platforms/vulkan/VkShader.hpp"

#if GFX_API_VULKAN

#include "VkUtils.hpp"
#include "toolkit/debug/Logger.hpp"
#include "toolkit/platforms/vulkan/VkContext.hpp"
#include "toolkit/platforms/vulkan/VkPipelineState.hpp"
#include "toolkit/platforms/vulkan/VkPlatform.hpp"
#include "toolkit/memory/Memory.hpp"
#include "toolkit/platforms/vulkan/VkBuffer.hpp"
#include "toolkit/platforms/vulkan/VkQueue.hpp"
#include "toolkit/platforms/vulkan/VkTexture.hpp"
#include "toolkit/platforms/vulkan/VkVertexLayout.hpp"
#include "toolkit/utils/Color32.hpp"
#include "toolkit/utils/Convertor.hpp"
#include "toolkit/platforms/System.hpp"
#include "toolkit/debug/Profiler.hpp"
#include "toolkit/platforms/FileSystem.hpp"
#include "VkSwapChain.hpp"
#include "VkTimerQuery.hpp"
#include "toolkit/utils/Sorting.hpp"

namespace toolkit
{
    VulkanDevice::OptionalVulkanDeviceExtensions VulkanDevice::optionalDeviceExtensions;
    VkInstance VulkanDevice::instance;
    TkArray<const char *> VulkanDevice::instanceExtensions;
    TkArray<const char *> VulkanDevice::instanceLayers;

    bool SupportsDebugUtilsExt = false;
#if VK_USE_DEBUG_LAYER
#if VK_EXT_debug_utils
    VkDebugUtilsMessengerEXT Messenger = VK_NULL_HANDLE;
#endif

    bool SupportsDebugCallbackExt = false;
    VkDebugReportCallbackEXT MsgCallback = VK_NULL_HANDLE;

    extern VulkanValidationLevel ValidationLevel;

#if VK_EXT_debug_report

    static VKAPI_ATTR VkBool32 VKAPI_PTR DebugReportFunction(VkDebugReportFlagsEXT msgFlags, VkDebugReportObjectTypeEXT objType, u64 srcObject, size_t location, i32 msgCode, const char *layerPrefix, const char *msg, void *userData)
    {
        const char *msgPrefix = "UNKNOWN";
        if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        {
            msgPrefix = "ERROR";
            if (!TkStringUtils::compareStrings(layerPrefix, "SC"))
            {
                if (msgCode == 3)
                {
                    // Attachment N not written by fragment shader
                    return VK_FALSE;
                }
                else if (msgCode == 5)
                {
                    // SPIR-V module not valid: MemoryBarrier: Vulkan specification requires Memory Semantics to have one of the following bits set: Acquire, Release, AcquireRelease or SequentiallyConsistent
                    return VK_FALSE;
                }
            }
        }
        else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        {
            msgPrefix = "WARN";
            if (!TkStringUtils::compareStrings(layerPrefix, "SC"))
            {
                if (msgCode == 2)
                {
                    // Fragment shader writes to output location 0 with no matching attachment
                    return VK_FALSE;
                }
            }
        }
        else if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        {
            msgPrefix = "PERF";
            if (!TkStringUtils::compareStrings(layerPrefix, "SC"))
            {
                if (msgCode == 2)
                {
                    // Vertex shader outputs unused interpolator
                    return VK_FALSE;
                }
            }
            else if (!TkStringUtils::compareStrings(layerPrefix, "DS"))
            {
                if (msgCode == 15)
                {
                    // DescriptorSet previously bound is incompatible with set newly bound as set #0 so set #1 and any subsequent sets were disturbed by newly bound pipelineLayout
                    return VK_FALSE;
                }
            }
        }
        else if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
        {
            msgPrefix = "INFO";
        }
        else if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
        {
            msgPrefix = "DEBUG";
        }

        LogInfo("[Vulkan] %s:%s:%d %s", msgPrefix, layerPrefix, msgCode, msg);
        return VK_FALSE;
    }

#endif

#if VK_EXT_debug_utils

    static VKAPI_ATTR VkBool32 VKAPI_PTR DebugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity, VkDebugUtilsMessageTypeFlagsEXT msgType, const VkDebugUtilsMessengerCallbackDataEXT *callbackData, void *userData)
    {
        // Ignore some errors
        switch (msgType)
        {
        case 2:
            switch (callbackData->messageIdNumber)
            {
            case 0:           // Attachment 4 not written by fragment shader; undefined values will be written to attachment
            case 2:           // Fragment shader writes to output location 0 with no matching attachment
            case 3:           // Attachment 2 not written by fragment shader
            case 5:           // SPIR-V module not valid: MemoryBarrier: Vulkan specification requires Memory Semantics to have one of the following bits set: Acquire, Release, AcquireRelease or SequentiallyConsistent
            case -1666394502: // After query pool creation, each query must be reset before it is used. Queries must also be reset between uses.
            case 602160055:   // Attachment 4 not written by fragment shader; undefined values will be written to attachment. TODO: investigate it for PS_GBuffer shader from Deferred material with USE_LIGHTMAP=1
            case 7060244:     //  Image Operand Offset can only be used with OpImage*Gather operations
            case -1539028524: // SortedIndices is null so Vulkan backend sets it to default R32_SFLOAT format which is not good for UINT format of the buffer
            case -1810835948: // SortedIndices is null so Vulkan backend sets it to default R32_SFLOAT format which is not good for UINT format of the buffer
            case -1621360350: // VkFramebufferCreateInfo attachment #0 has a layer count (1) smaller than the corresponding framebuffer layer count (64). The Vulkan spec states: If flags does not include VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT, each element of pAttachments that is used as an input, color, resolve, or depth/stencil attachment by renderPass must have been created with a VkImageViewCreateInfo::subresourceRange.layerCount greater than or equal to layers
                return VK_FALSE;
            }
            break;
        case 4:
            switch (callbackData->messageIdNumber)
            {
            case 0:         // Vertex shader writes to output location 0.0 which is not consumed by fragment shader
            case 558591440: // preTransform doesn't match the currentTransform returned by vkGetPhysicalDeviceSurfaceCapabilitiesKHR, the presentation engine will transform the image content as part of the presentation operation. TODO: implement preTransform for Android to improve swapchain presentation performance
            case 101294395: // Vertex shader writes to output location 0.0 which is not consumed by fragment shader
                return VK_FALSE;
            }
            break;
        case 6:
            switch (callbackData->messageIdNumber)
            {
            case 2: // Vertex shader writes to output location 0.0 which is not consumed by fragment shader
                return VK_FALSE;
            }
            break;
        }

        const char *severity = "";
        if (msgSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            severity = "Error";
        }
        else if (msgSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            severity = "Warning";
        }
        else if (msgSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            severity = "Info";
        }
        else if (msgSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
            severity = "Verbose";
        }

        const char *type = "";
        if (msgType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
        {
            if (msgType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
            {
                type = "General/Validation";
            }
            else
            {
                type = "General";
            }
        }
        else if (msgType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
        {
            if (msgType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
            {
                type = "Perf/Validation";
            }
            else
            {
                type = "Validation";
            }
        }
        else if (msgType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
        {
            type = "Perf";
        }

        // Fix invalid characters in hex values (bug in Debug Layer)
        char *handleStart = (char *)TkStringUtils::findInsensitive(callbackData->pMessage, "0x");
        while (handleStart != nullptr)
        {
            while (*handleStart != ' ' && *handleStart != 0)
            {
                *handleStart = TkMath::clamp<char>(*handleStart, '0', 'z');
                handleStart++;
            }
            if (*handleStart == 0)
                break;
            handleStart = (char *)TkStringUtils::findInsensitive(handleStart, "0x");
        }

        const TkString message(callbackData->pMessage);
        if (callbackData->pMessageIdName)
            LogInfo("[Vulkan] %s %s:%d(%s) %s", type, severity, callbackData->messageIdNumber, callbackData->pMessageIdName, *message);
        else
            LogInfo("[Vulkan] %s %s:%d %s", type, severity, callbackData->messageIdNumber, *message);

#if TK_DEBUG
        if (auto *context = (VulkanContext *)GfxDevice::sInstance->getMainContext())
        {
            if (auto *state = (VulkanPipelineState *)context->getState())
            {
                const TkString vsName = state->debugDesc.vs ? state->debugDesc.vs->getName() : TkString::Empty;
                const TkString psName = state->debugDesc.ps ? state->debugDesc.ps->getName() : TkString::Empty;
            LogWarn("[Vulkan] Error during rendering with VS=%s, PS=%s", *vsName, *psName);
            }
        }
#endif

        return VK_FALSE;
    }

#endif

    void setupDebugLayerCallback()
    {
#if VK_EXT_debug_utils
        if (SupportsDebugUtilsExt)
        {
            if (vkCreateDebugUtilsMessengerEXT)
            {
                VkDebugUtilsMessengerCreateInfoEXT createInfo;
                VkUtils::zeroStruct(createInfo, VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT);
                createInfo.pfnUserCallback = DebugUtilsCallback;
                switch ((i32)ValidationLevel)
                {
                default:
                    createInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
                case 4:
                    createInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
                case 3:
                    createInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                case 2:
                    createInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
                    createInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
                case 1:
                    createInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                    createInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
                    break;
                case 0:
                    break;
                }
                const VkResult result = vkCreateDebugUtilsMessengerEXT(VulkanDevice::instance, &createInfo, nullptr, &Messenger);
                LOG_VK_RESULT(result);
            }
        }
        else if (SupportsDebugCallbackExt)
#else
        if (SupportsDebugCallbackExt)
#endif
        {
#if VK_EXT_debug_report
            if (vkCreateDebugReportCallbackEXT)
            {
                VkDebugReportCallbackCreateInfoEXT createInfo;
                VkUtils::zeroStruct(createInfo, VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT);
                createInfo.pfnCallback = DebugReportFunction;
                switch ((i32)ValidationLevel)
                {
                default:
                    createInfo.flags |= VK_DEBUG_REPORT_DEBUG_BIT_EXT;
                case 4:
                    createInfo.flags |= VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
                case 3:
                    createInfo.flags |= VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
                case 2:
                    createInfo.flags |= VK_DEBUG_REPORT_WARNING_BIT_EXT;
                case 1:
                    createInfo.flags |= VK_DEBUG_REPORT_ERROR_BIT_EXT;
                    break;
                case 0:
                    break;
                }
                const VkResult result = vkCreateDebugReportCallbackEXT(VulkanDevice::instance, &createInfo, nullptr, &MsgCallback);
                LOG_VK_RESULT(result);
            }
            else
            {
                LogWarn("GetProcAddr: Unable to find vkDbgCreateMsgCallback; debug reporting skipped!");
            }
#endif
        }
        else
        {
            LogWarn("Instance does not support 'VK_EXT_debug_report' extension; debug reporting skipped!");
        }
    }

    void removeDebugLayerCallback()
    {
#if VK_EXT_debug_utils
        if (Messenger != VK_NULL_HANDLE)
        {
            if (vkDestroyDebugUtilsMessengerEXT)
                vkDestroyDebugUtilsMessengerEXT(VulkanDevice::instance, Messenger, nullptr);
            Messenger = VK_NULL_HANDLE;
        }
        else if (MsgCallback != VK_NULL_HANDLE)
#else
        if (MsgCallback != VK_NULL_HANDLE)
#endif
        {
#if VK_EXT_debug_report
            if (vkDestroyDebugReportCallbackEXT)
                vkDestroyDebugReportCallbackEXT(VulkanDevice::instance, MsgCallback, nullptr);
#endif
            MsgCallback = VK_NULL_HANDLE;
        }
    }

#endif

    VulkanDeferredDeletionQueue::VulkanDeferredDeletionQueue(VulkanDevice *device)
        : m_device(device)
    {
    }

    VulkanDeferredDeletionQueue::~VulkanDeferredDeletionQueue()
    {
        TK_ASSERT(m_entries.isEmpty());
    }

    void VulkanDeferredDeletionQueue::releaseResources(bool immediately)
    {
        const u64 frameIndex = m_device ? m_device->getFrameIndex() : 0;
        const u64 checkFrame = frameIndex - TkMath::min<u64>(frameIndex, VK_RESOURCE_DELETE_SAFE_FRAMES_COUNT);
        TkMutexGuard lock(m_locker);
        for (i32 i = 0; i < m_entries.count(); i++)
        {
            Entry *e = &m_entries.get()[i];
            if (immediately || (checkFrame > e->frameNumber && (e->cmdBuffer == nullptr || e->fenceCounter < e->cmdBuffer->getFenceSignaledCounter())))
            {
                if (e->allocationHandle == VK_NULL_HANDLE)
                {
                    switch (e->structureType)
                    {
#define SWITCH_CASE(type)                                                \
    case Type::k##type:                                                  \
        vkDestroy##type(m_device->device, (Vk##type)e->handle, nullptr); \
        break
                        SWITCH_CASE(RenderPass);
                        SWITCH_CASE(Buffer);
                        SWITCH_CASE(BufferView);
                        SWITCH_CASE(Image);
                        SWITCH_CASE(ImageView);
                        SWITCH_CASE(Pipeline);
                        SWITCH_CASE(PipelineLayout);
                        SWITCH_CASE(Framebuffer);
                        SWITCH_CASE(DescriptorSetLayout);
                        SWITCH_CASE(Sampler);
                        SWITCH_CASE(Semaphore);
                        SWITCH_CASE(ShaderModule);
                        SWITCH_CASE(Event);
                        SWITCH_CASE(QueryPool);
#undef SWITCH_CASE
                    default:
#if TK_DEBUG
                        TK_ASSERT(false && "releaseResources");
#endif
                        break;
                    }
                }
                else
                {
                    if (e->structureType == kImage)
                    {
                        vmaDestroyImage(m_device->allocator, (VkImage)e->handle, e->allocationHandle);
                    }
                    else if (e->structureType == kBuffer)
                    {
                        vmaDestroyBuffer(m_device->allocator, (VkBuffer)e->handle, e->allocationHandle);
                    }
#if TK_DEBUG
                    else
                    {
                        TK_ASSERT(false && "releaseResources2");
                    }
#endif
                }

                m_entries.removeAt(i--);
                if (m_entries.isEmpty())
                    break;
            }
        }
    }

    void VulkanDeferredDeletionQueue::enqueueGenericResource(Type type, u64 handle, VmaAllocation allocation)
    {
        TK_ASSERT(handle != 0);

        Entry entry;
        m_device->graphicsQueue->getLastSubmittedInfo(entry.cmdBuffer, entry.fenceCounter);
        entry.handle = handle;
        entry.allocationHandle = allocation;
        entry.structureType = type;
        entry.frameNumber = m_device ? m_device->getFrameIndex() : 0;

        TkMutexGuard lock(m_locker);
#if TK_DEBUG && 0
        const Function<bool(const Entry &)> ContainsHandle = [handle](const Entry &e)
        {
            return e.handle == handle;
        };
        TK_ASSERT(!ArrayExtensions::any(m_entries, ContainsHandle));
#endif
        m_entries.add(entry);
    }

    u32 getHash(const VulkanRenderTargetLayout &key)
    {
        u32 hash = (i32)key.msaa * 11;
        combineHash(hash, key.flags);
        combineHash(hash, (u32)key.depthFormat * 93473262);
        // Extent intentionally excluded — see VulkanRenderTargetLayout in VkDevice.hpp.
        for (i32 i = 0; i < lengthOf(key.rtvsFormats); i++)
            combineHash(hash, (u32)key.rtvsFormats[i]);
        return hash;
    }

    u32 getHash(const VulkanFramebuffer::Key &key)
    {
        u32 hash = (i32)(intptr)key.renderPass;
        combineHash(hash, (u32)key.attachmentCount * 136);
        for (i32 i = 0; i < lengthOf(key.attachments); i++)
            combineHash(hash, (u32)(intptr)key.attachments[i]);
        return hash;
    }

    VulkanVertexLayout::VulkanVertexLayout(VulkanDevice *device, const Elements &elements, bool explicitOffsets)
        : VulkanResource<GfxVertexLayout>(device, TkStringView::Empty)
    {
        setElements(elements, explicitOffsets);
        maxSlot = 0;
        for (i32 i = 0; i < elements.count(); i++)
        {
            const VertexElement &src = getElements().get()[i];
            maxSlot = TkMath::max(maxSlot, (i32)src.slot);
        }
    }

    VulkanFramebuffer::VulkanFramebuffer(VulkanDevice *device, const Key &key, const VkExtent2D &extent, u32 layers)
        : device(device), handle(VK_NULL_HANDLE), extent(extent), layers(layers)
    {
        TkMemory::memoryCopy(attachments, key.attachments, sizeof(attachments));

        VkFramebufferCreateInfo createInfo;
        VkUtils::zeroStruct(createInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
        createInfo.renderPass = key.renderPass->handle;
        createInfo.attachmentCount = key.attachmentCount;
        createInfo.pAttachments = key.attachments;
        createInfo.width = extent.width;
        createInfo.height = extent.height;
        createInfo.layers = layers;
        VALIDATE_VK_RESULT(vkCreateFramebuffer(device->device, &createInfo, nullptr, &handle));
    }

    VulkanFramebuffer::~VulkanFramebuffer()
    {
        device->deferredDeletionQueue.enqueueResource(VulkanDeferredDeletionQueue::Type::kFramebuffer, handle);
    }

    bool VulkanFramebuffer::hasReference(VkImageView imageView) const
    {
        for (i32 i = 0; i < lengthOf(attachments); i++)
        {
            if (attachments[i] == imageView)
                return true;
        }
        return false;
    }

    VulkanRenderPass::VulkanRenderPass(VulkanDevice *device, const VulkanRenderTargetLayout &layout)
        : device(device), handle(VK_NULL_HANDLE), layout(layout), canDepthWrite(true)
    {
        const i32 colorattachmentsCount = layout.rtsCount;
        const bool hasDepthStencilAttachment = layout.depthFormat != PixelFormat::kUnknown;
        const i32 attachmentsCount = colorattachmentsCount + (hasDepthStencilAttachment ? 1 : 0);

        VkAttachmentReference colorReferences[GFX_MAX_RT_BINDED];
        VkAttachmentReference depthStencilReference;
        VkAttachmentDescription attachments[GFX_MAX_RT_BINDED + 1];

        VkSubpassDescription subpassDesc;
        TkMemory::memoryClear(&subpassDesc, sizeof(subpassDesc));
        subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDesc.colorAttachmentCount = colorattachmentsCount;
        subpassDesc.pColorAttachments = colorReferences;
        subpassDesc.pResolveAttachments = nullptr;
        for (i32 i = 0; i < colorattachmentsCount; i++)
        {
            VkAttachmentDescription &attachment = attachments[i];
            attachment.flags = 0;
            attachment.format = VkUtils::toVulkanFormat(layout.rtvsFormats[i]);
            attachment.samples = (VkSampleCountFlagBits)layout.msaa;
#if TK_ANDROID
            attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; // TODO: Adreno 640 has glitches when blend is disabled and rt data not loaded
#elif TK_MACOS || TK_IOS
            attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; // MoltenVK seams to have glitches (tiled arch of gpu)
#else
            // TODO: we need render passes into high-level rendering api to perform more optimal rendering (esp. for tiled gpus)
            attachment.loadOp = layout.blendEnable ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
#endif
            attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            VkAttachmentReference &reference = colorReferences[i];
            reference.attachment = i;
            reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        if (hasDepthStencilAttachment)
        {
            VkImageLayout depthStencilLayout;
#if 0
        // TODO: enable extension and use separateDepthStencilLayouts from Vulkan 1.2
        if (layout.readStencil || layout.writeStencil)
        {
            if (layout.writeDepth && layout.writeStencil)
                depthStencilLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            else if (layout.writeDepth && !layout.writeStencil)
                depthStencilLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
            else if (layout.writeStencil && !layout.writeDepth)
                depthStencilLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
            else if (layout.readDepth)
                depthStencilLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            else
                depthStencilLayout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
        }
        else
        {
            // Depth-only
            if (layout.readDepth && !layout.writeDepth)
                depthStencilLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
            else
                depthStencilLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        }
#else
            if ((layout.readDepth || layout.readStencil) && !(layout.writeDepth || layout.writeStencil))
                depthStencilLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            else
                depthStencilLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
#endif

            // Use last slot for depth stencil attachment
            VkAttachmentDescription &attachment = attachments[colorattachmentsCount];
            attachment.flags = 0;
            attachment.format = VkUtils::toVulkanFormat(layout.depthFormat);
            attachment.samples = (VkSampleCountFlagBits)layout.msaa;
            attachment.loadOp = layout.readDepth || layout.readStencil ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            // attachment.storeOp = layout.writeDepth || layout.writeStencil ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // For some reason, read-only depth results in artifacts
            attachment.stencilLoadOp = layout.readStencil ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.stencilStoreOp = layout.writeStencil ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.initialLayout = depthStencilLayout;
            attachment.finalLayout = depthStencilLayout;
            depthStencilReference.attachment = colorattachmentsCount;
            depthStencilReference.layout = depthStencilLayout;
            subpassDesc.pDepthStencilAttachment = &depthStencilReference;
            if (!layout.writeDepth && !layout.writeStencil)
                canDepthWrite = false;
        }

        VkRenderPassCreateInfo createInfo;
        VkUtils::zeroStruct(createInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
        createInfo.attachmentCount = attachmentsCount;
        createInfo.pAttachments = attachments;
        createInfo.subpassCount = 1;
        createInfo.pSubpasses = &subpassDesc;
        VALIDATE_VK_RESULT(vkCreateRenderPass(device->device, &createInfo, nullptr, &handle));
#if VK_USE_DEBUG_DATA
        debugCreateInfo = createInfo;
#endif
    }

    VulkanRenderPass::~VulkanRenderPass()
    {
        device->deferredDeletionQueue.enqueueResource(VulkanDeferredDeletionQueue::Type::kRenderPass, handle);
    }

    VulkanQueryPool::VulkanQueryPool(VulkanDevice *device, i32 capacity, VkQueryType type)
        : m_device(device), m_handle(VK_NULL_HANDLE), m_type(type)
    {
        VkQueryPoolCreateInfo createInfo;
        VkUtils::zeroStruct(createInfo, VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO);
        createInfo.queryType = type;
        createInfo.queryCount = capacity;
        VALIDATE_VK_RESULT(vkCreateQueryPool(device->device, &createInfo, nullptr, &m_handle));

#if VK_RESET_QUERY_POOLS
        // New queries have to be reset before use
        resetBeforeUse = true;
        m_resetRanges.add(Range{0, static_cast<u32>(capacity)});
#endif
    }

    VulkanQueryPool::~VulkanQueryPool()
    {
#if VK_RESET_QUERY_POOLS
        m_device->queriesToReset.remove(this);
#endif
        m_device->deferredDeletionQueue.enqueueResource(VulkanDeferredDeletionQueue::Type::kQueryPool, m_handle);
    }

#if VK_RESET_QUERY_POOLS

    void VulkanQueryPool::reset(VulkanCmdBuffer *cmdBuffer)
    {
        for (auto &range : m_resetRanges)
        {
            vkCmdResetQueryPool(cmdBuffer->getHandle(), m_handle, range.start, range.count);
        }
        m_resetRanges.clear();
        resetBeforeUse = false;
    }

#endif

    VulkanBufferedQueryPool::VulkanBufferedQueryPool(VulkanDevice *device, i32 capacity, VkQueryType type)
        : VulkanQueryPool(device, capacity, type), m_lastBeginIndex(0)
    {
        m_queryOutput.resize(capacity);
        m_usedQueryBits.addZeroed((capacity + 63) / 64);
        m_startedQueryBits.addZeroed((capacity + 63) / 64);
        m_readResultsBits.addZeroed((capacity + 63) / 64);
    }

    bool VulkanBufferedQueryPool::acquireQuery(VulkanCmdBuffer *cmdBuffer, u32 &resultIndex)
    {
        const u64 allUsedMask = kMaxU64;
        for (i32 wordIndex = m_lastBeginIndex / 64; wordIndex < m_usedQueryBits.count(); wordIndex++)
        {
            u64 beginQueryWord = m_usedQueryBits[wordIndex];
            if (beginQueryWord != allUsedMask)
            {
                resultIndex = 0;
                while ((beginQueryWord & 1) == 1)
                {
                    resultIndex++;
                    beginQueryWord >>= 1;
                }
                resultIndex += wordIndex * 64;
                const u64 bit = (u64)1 << (u64)(resultIndex % 64);
                m_usedQueryBits[wordIndex] = m_usedQueryBits[wordIndex] | bit;
                m_readResultsBits[wordIndex] &= ~bit;
                m_lastBeginIndex = resultIndex + 1;
                if (resetBeforeUse)
                    reset(cmdBuffer);
                return true;
            }
        }
        return false;
    }

    void VulkanBufferedQueryPool::releaseQuery(u32 queryIndex)
    {
        const u32 word = queryIndex / 64;
        const u64 bit = (u64)1 << (queryIndex % 64);
        m_usedQueryBits[word] = m_usedQueryBits[word] & ~bit;
        m_readResultsBits[word] = m_readResultsBits[word] & ~bit;
        if (queryIndex < (u32)m_lastBeginIndex)
        {
            // Use the lowest word available
            const u64 allUsedMask = kMaxU64;
            const i32 lastQueryWord = m_lastBeginIndex / 64;
            if (lastQueryWord < m_usedQueryBits.count() && m_usedQueryBits[lastQueryWord] == allUsedMask)
            {
                m_lastBeginIndex = (u32)queryIndex;
            }
        }
    }

    void VulkanBufferedQueryPool::markQueryAsStarted(u32 queryIndex)
    {
        const u32 word = queryIndex / 64;
        const u64 bit = (u64)1 << (queryIndex % 64);
        m_startedQueryBits[word] = m_startedQueryBits[word] | bit;
    }

    bool VulkanBufferedQueryPool::getResults(VulkanContext *context, u32 index, u64 &result)
    {
        const u64 bit = (u64)(index % 64);
        const u64 bitMask = (u64)1 << bit;
        const u32 word = index / 64;

        if ((m_startedQueryBits[word] & bitMask) == 0)
        {
            // Query never started/ended
            result = 0;
            return true;
        }

        if ((m_readResultsBits[word] & bitMask) == 0)
        {
            const VkResult vkResult = vkGetQueryPoolResults(m_device->device, m_handle, index, 1, sizeof(u64), &m_queryOutput[index], sizeof(u64), VK_QUERY_RESULT_64_BIT);
            if (vkResult == VK_SUCCESS)
            {
                m_readResultsBits[word] = m_readResultsBits[word] | bitMask;

#if VK_RESET_QUERY_POOLS
                // Add to reset
                if (!m_device->queriesToReset.contains(this))
                    m_device->queriesToReset.add(this);
                m_resetRanges.add(Range{index, 1});
#endif
            }
            else if (vkResult == VK_NOT_READY)
            {
                // No results yet
                result = 0;
                return false;
            }
            else
            {
                LOG_VK_RESULT(vkResult);
                result = 0;
                return false;
            }
        }

        result = m_queryOutput[index];
        return true;
    }

    bool VulkanBufferedQueryPool::hasRoom() const
    {
        const u64 allUsedMask = kMaxU64;
        if (m_lastBeginIndex < m_usedQueryBits.count() * 64)
        {
            TK_ASSERT((m_usedQueryBits[m_lastBeginIndex / 64] & allUsedMask) != allUsedMask);
            return true;
        }
        return false;
    }

    VulkanResourcesHelper::VulkanResourcesHelper(VulkanDevice *device)
        : m_device(device)
    {
        TkMemory::memoryClear(m_dummyTextures, sizeof(m_dummyTextures));
        TkMemory::memoryClear(m_staticSamplers, sizeof(m_staticSamplers));
    }

    void InitSampler(VkSamplerCreateInfo &createInfo, bool supportsMirrorClampToEdge, GfxSamplerFilter filter, GfxSamplerAddressMode addressU, GfxSamplerAddressMode addressV, GfxSamplerAddressMode addressW, GfxSamplerCompareFunction compareFunction)
    {
        createInfo.magFilter = VkUtils::toVulkanMagFilterMode(filter);
        createInfo.minFilter = VkUtils::toVulkanMinFilterMode(filter);
        createInfo.mipmapMode = VkUtils::toVulkanMipFilterMode(filter);
        createInfo.addressModeU = VkUtils::toVulkanWrapMode(addressU, supportsMirrorClampToEdge);
        createInfo.addressModeV = VkUtils::toVulkanWrapMode(addressV, supportsMirrorClampToEdge);
        createInfo.addressModeW = VkUtils::toVulkanWrapMode(addressW, supportsMirrorClampToEdge);
        createInfo.compareEnable = compareFunction != GfxSamplerCompareFunction::kNever ? VK_TRUE : VK_FALSE;
        createInfo.compareOp = VkUtils::toVulkanSamplerCompareFunction(compareFunction);
    }

    VkSampler *VulkanResourcesHelper::getStaticSamplers()
    {
        static_assert(GFX_STATIC_SAMPLERS_COUNT == 6, "Update static samplers setup.");
        if (!m_staticSamplers[0])
        {
            const bool supportsMirrorClampToEdge = VulkanDevice::optionalDeviceExtensions.hasMirrorClampToEdge;

            VkSamplerCreateInfo createInfo;
            VkUtils::zeroStruct(createInfo, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
            createInfo.mipLodBias = 0.0f;
            createInfo.minLod = 0.0f;
            createInfo.maxLod = kMaxF32;
            createInfo.maxAnisotropy = 1.0f;
            createInfo.anisotropyEnable = VK_FALSE;
            createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

            // Linear Clamp
            InitSampler(createInfo, supportsMirrorClampToEdge, GfxSamplerFilter::kTrilinear, GfxSamplerAddressMode::kClamp, GfxSamplerAddressMode::kClamp, GfxSamplerAddressMode::kClamp, GfxSamplerCompareFunction::kNever);
            VALIDATE_VK_RESULT(vkCreateSampler(m_device->device, &createInfo, nullptr, &m_staticSamplers[0]));

            // Point Clamp
            InitSampler(createInfo, supportsMirrorClampToEdge, GfxSamplerFilter::kPoint, GfxSamplerAddressMode::kClamp, GfxSamplerAddressMode::kClamp, GfxSamplerAddressMode::kClamp, GfxSamplerCompareFunction::kNever);
            VALIDATE_VK_RESULT(vkCreateSampler(m_device->device, &createInfo, nullptr, &m_staticSamplers[1]));

            // Linear Wrap
            InitSampler(createInfo, supportsMirrorClampToEdge, GfxSamplerFilter::kTrilinear, GfxSamplerAddressMode::kWrap, GfxSamplerAddressMode::kWrap, GfxSamplerAddressMode::kWrap, GfxSamplerCompareFunction::kNever);
            VALIDATE_VK_RESULT(vkCreateSampler(m_device->device, &createInfo, nullptr, &m_staticSamplers[2]));

            // Point Wrap
            InitSampler(createInfo, supportsMirrorClampToEdge, GfxSamplerFilter::kPoint, GfxSamplerAddressMode::kWrap, GfxSamplerAddressMode::kWrap, GfxSamplerAddressMode::kWrap, GfxSamplerCompareFunction::kNever);
            VALIDATE_VK_RESULT(vkCreateSampler(m_device->device, &createInfo, nullptr, &m_staticSamplers[3]));

            // Shadow
            InitSampler(createInfo, supportsMirrorClampToEdge, GfxSamplerFilter::kPoint, GfxSamplerAddressMode::kClamp, GfxSamplerAddressMode::kClamp, GfxSamplerAddressMode::kClamp, GfxSamplerCompareFunction::kLess);
            VALIDATE_VK_RESULT(vkCreateSampler(m_device->device, &createInfo, nullptr, &m_staticSamplers[4]));

            // Shadow PCF
            InitSampler(createInfo, supportsMirrorClampToEdge, GfxSamplerFilter::kTrilinear, GfxSamplerAddressMode::kClamp, GfxSamplerAddressMode::kClamp, GfxSamplerAddressMode::kClamp, GfxSamplerCompareFunction::kLess);
            VALIDATE_VK_RESULT(vkCreateSampler(m_device->device, &createInfo, nullptr, &m_staticSamplers[5]));
        }
        return m_staticSamplers;
    }

    VulkanTexture *VulkanResourcesHelper::getDummyTexture(SpirvShaderResourceType type)
    {
        i32 index;
        switch (type)
        {
        case SpirvShaderResourceType::kTexture1D:
            index = 0;
            break;
        case SpirvShaderResourceType::kTexture2D:
            index = 1;
            break;
        case SpirvShaderResourceType::kTexture3D:
            index = 2;
            break;
        case SpirvShaderResourceType::kTextureCube:
            index = 3;
            break;
        case SpirvShaderResourceType::kTexture1DArray:
            index = 4;
            break;
        case SpirvShaderResourceType::kTexture2DArray:
            index = 5;
            break;
        default:
            TK_ASSERT(false && "getDummyTexture 836");
            return nullptr;
        }

        auto texture = m_dummyTextures[index];
        if (!texture)
        {
            texture = (VulkanTexture *)m_device->createTexture("DummyTexture");
            if (!texture)
                return nullptr;
            GfxTextureDescription desc;
            const PixelFormat format = PixelFormat::kR8G8B8A8_UNorm;
            const GfxTextureFlags flags = GfxTextureFlags::kShaderResource | GfxTextureFlags::kUnorderedAccess;
            switch (type)
            {
            case SpirvShaderResourceType::kTexture1D:
                desc = GfxTextureDescription::new1D(1, 1, format, flags, 1);
                break;
            case SpirvShaderResourceType::kTexture2D:
                desc = GfxTextureDescription::new2D(1, 1, format, flags);
                break;
            case SpirvShaderResourceType::kTexture3D:
                desc = GfxTextureDescription::new3D(1, 1, 1, format, flags);
                break;
            case SpirvShaderResourceType::kTextureCube:
                desc = GfxTextureDescription::newCube(1, format, flags);
                break;
            case SpirvShaderResourceType::kTexture1DArray:
                desc = GfxTextureDescription::new1D(1, 1, format, flags, 4);
                break;
            case SpirvShaderResourceType::kTexture2DArray:
                desc = GfxTextureDescription::new2D(1, 1, format, flags, 4);
                break;
            default:;
            }
            if (!texture->init(desc) || !texture->view(0))
            {
                GFX_DEL_RESOURCE(texture);
                return nullptr;
            }
            m_dummyTextures[index] = texture;
        }

        return texture;
    }

    VulkanBuffer *VulkanResourcesHelper::getDummyBuffer(PixelFormat format)
    {
        if (!m_dummyBuffers)
        {
            m_dummyBuffers = (VulkanBuffer **)TkMemory::allocate(TkMemory::getDefault(), (i32)PixelFormat::MAX * sizeof(void *));
            TkMemory::memoryClear(m_dummyBuffers, (i32)PixelFormat::MAX * sizeof(void *));
        }
        auto &dummyBuffer = m_dummyBuffers[(i32)format];
        if (!dummyBuffer)
        {
            dummyBuffer = (VulkanBuffer *)m_device->createBuffer("DummyBuffer");
            if (!dummyBuffer || !dummyBuffer->init(GfxBufferDescription::buffer(GfxPixelFormat::sizeInBytes(format) * 256, GfxBufferFlags::kShaderResource | GfxBufferFlags::kUnorderedAccess, format)))
            {
                GFX_DEL_RESOURCE(dummyBuffer);
                return nullptr;
            }
        }
        return dummyBuffer;
    }

    VulkanBuffer *VulkanResourcesHelper::getDummyVertexBuffer()
    {
        if (!m_dummyVB)
        {
            m_dummyVB = (VulkanBuffer *)m_device->createBuffer("DummyVertexBuffer");
            auto *layout = GfxVertexLayout::get({{VertexElementTypes::kAttribute3, 0, 0, 0, PixelFormat::kR8G8B8A8_UNorm}});
            if (!m_dummyVB || !layout || !m_dummyVB->init(GfxBufferDescription::vertex(layout, sizeof(Color32), 1, &Color32::Transparent)))
            {
                GFX_DEL_RESOURCE(m_dummyVB);
                return nullptr;
            }
        }
        return m_dummyVB;
    }

    GfxConstantBuffer *VulkanResourcesHelper::getDummyConstantBuffer()
    {
        if (!m_dummyCB)
        {
            m_dummyCB = m_device->createConstantBuffer(256, "DummyConstantBuffer");
            if (!m_dummyCB)
                return nullptr;
        }
        return m_dummyCB;
    }

    void VulkanResourcesHelper::dispose()
    {
        GFX_DEL_RESOURCES(m_dummyTextures);
        GFX_DEL_RESOURCE(m_dummyVB);
        GFX_DEL_RESOURCE(m_dummyCB);
        if (m_dummyBuffers)
        {
            for (i32 i = 0; i < (i32)PixelFormat::MAX; i++)
            {
                if (VulkanBuffer *buffer = m_dummyBuffers[i])
                {
                    buffer->releaseData();
                    TkMemory::deletePtr(buffer);
                }
            }
            TkMemory::free(TkMemory::getDefault(), m_dummyBuffers);
            m_dummyBuffers = nullptr;
        }

        for (i32 i = 0; i < lengthOf(m_staticSamplers); i++)
        {
            auto &sampler = m_staticSamplers[i];
            if (sampler != VK_NULL_HANDLE)
            {
                m_device->deferredDeletionQueue.enqueueResource(VulkanDeferredDeletionQueue::Type::kSampler, sampler);
                sampler = VK_NULL_HANDLE;
            }
        }
    }

    VulkanStagingManager::VulkanStagingManager(VulkanDevice *device)
        : m_device(device)
    {
    }

    static inline u32 stagingAlignUpPow2(u32 v)
    {
        if (v <= 1) return 1;
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        return v + 1;
    }

    GfxBuffer *VulkanStagingManager::acquireBuffer(u32 size, GfxResourceUsage usage)
    {
        // Bucket sizing: round up to a power of two (min 64KB) so a small set
        // of physical staging buffers can serve a wide range of upload sizes
        // without exact-size matching causing pool fragmentation.
        const u32 kMinBucket = 64u * 1024u;
        const u32 bucketSize = TkMath::max(stagingAlignUpPow2(size), kMinBucket);

        // Try reuse free buffer
        {
            TkMutexGuard lock(m_locker);

            for (i32 i = 0; i < m_freeBuffers.count(); i++)
            {
                auto &freeBuffer = m_freeBuffers[i];
                const u32 freeSize = freeBuffer.buffer->getSize();
                // Reuse when the buffer is large enough and not wastefully large (<= 2x the bucket).
                if (freeSize >= size && freeSize <= bucketSize * 2 &&
                    freeBuffer.buffer->getDescription().usage == usage)
                {
                    const auto buffer = freeBuffer.buffer;
                    m_freeBuffers.removeAt(i);
                    return buffer;
                }
            }
        }

        // Allocate new buffer at bucket size
        auto buffer = m_device->createBuffer("Pooled Staging");
        if (!buffer || !buffer->init(GfxBufferDescription::buffer(bucketSize, GfxBufferFlags::kNone, PixelFormat::kUnknown, nullptr, 0, usage)))
        {
            LogWarn("Failed to create pooled staging buffer.");
            GFX_DEL_RESOURCE(buffer);
            return nullptr;
        }

        // Cache buffer
        {
            TkMutexGuard lock(m_locker);

            m_allBuffers.add(buffer);
#if TK_DEBUG
            m_allBuffersAllocSize += bucketSize;
            m_allBuffersTotalSize += bucketSize;
            m_allBuffersPeekSize = TkMath::max(m_allBuffersTotalSize, m_allBuffersPeekSize);
#endif
        }

        return buffer;
    }

    void VulkanStagingManager::releaseBuffer(VulkanCmdBuffer *cmdBuffer, GfxBuffer *&buffer)
    {
        TkMutexGuard lock(m_locker);

        if (cmdBuffer)
        {
            // Return to pending pool (need to wait until command buffer will be executed and buffer will be reusable)
            auto &item = m_pendingBuffers.addOne();
            item.buffer = buffer;
            item.cmdBuffer = cmdBuffer;
            item.fenceCounter = cmdBuffer->getFenceSignaledCounter();
        }
        else
        {
            // Return to pool
            m_freeBuffers.add({buffer, m_device ? m_device->getFrameIndex() : 0});
        }

        // Clear reference
        buffer = nullptr;
    }

    void VulkanStagingManager::processPendingFree()
    {
        TkMutexGuard lock(m_locker);

        // Find staging buffers that has been processed by the GPU and can be reused
        for (i32 i = m_pendingBuffers.count() - 1; i >= 0; i--)
        {
            auto &e = m_pendingBuffers[i];
            if (e.fenceCounter < e.cmdBuffer->getFenceSignaledCounter())
            {
                // Return to pool
                m_freeBuffers.add({e.buffer, m_device ? m_device->getFrameIndex() : 0});
                m_pendingBuffers.removeAt(i);
            }
        }

        // Free staging buffers that have remained unused beyond the streaming
        // retention window. Do not tie pool eviction to the much shorter GPU
        // deletion safety delay or video frame gaps will churn large buffers.
        const u64 frameIndex = m_device ? m_device->getFrameIndex() : 0;
        for (i32 i = m_freeBuffers.count() - 1; i >= 0; i--)
        {
            auto &e = m_freeBuffers.get()[i];
            if (e.frameNumber + VK_STAGING_BUFFER_NOT_USED_FRAME_TIMEOUT < frameIndex)
            {
                auto buffer = e.buffer;

                // Remove buffer from lists
                m_allBuffers.remove(buffer);
                m_freeBuffers.removeAt(i);

#if TK_DEBUG
                // Update stats
                m_allBuffersFreeSize += buffer->getSize();
                m_allBuffersTotalSize -= buffer->getSize();
#endif

                // Release memory
                buffer->releaseData();
                TkMemory::deletePtr(buffer);
            }
        }
    }

    void VulkanStagingManager::dispose()
    {
        TkMutexGuard lock(m_locker);

#if TK_DEBUG
        const TkString peekSizeText = TkConvertor::bytesToText(m_allBuffersPeekSize);
        const TkString allocSizeText = TkConvertor::bytesToText(m_allBuffersAllocSize);
        const TkString freeSizeText = TkConvertor::bytesToText(m_allBuffersFreeSize);
        LogInfo("Vulkan staging buffers peek memory usage: %s, allocs: %s, frees: %s", *peekSizeText, *allocSizeText, *freeSizeText);
#endif

        // Release buffers and clear memory
        for (auto buffer : m_allBuffers)
        {
            buffer->releaseData();
            TkMemory::deletePtr(buffer);
        }
        m_allBuffers.resize(0);
        m_pendingBuffers.resize(0);
    }

    VulkanDevice::VulkanDevice(GfxShaderProfile shaderProfile, VulkanAdapter *adapter)
        : GfxDevice(GfxBackendType::kVulkan, shaderProfile), m_renderPasses(64), m_framebuffers(64), m_layouts(256), adapter(adapter), deferredDeletionQueue(this), stagingManager(this), helperResources(this)
    {
    }

    GfxDevice *VulkanDevice::create()
    {
        LogInfo("VulkanDevice::create enter");
        VkResult result;

        // Initialize bindings
        LogInfo("VulkanDevice::create before volkInitialize");
        result = volkInitialize();
        if (result != VK_SUCCESS)
        {
            LogWarn("Graphics device init failed with error %s", VkUtils::getErrorString(result));
            return nullptr;
        }
        LogInfo("VulkanDevice::create after volkInitialize");
        if (!vkCreateInstance || !vkEnumerateInstanceExtensionProperties || !vkEnumerateInstanceLayerProperties)
        {
            LogWarn("Graphics device init failed because required Vulkan loader entry points are missing.");
            return nullptr;
        }

        // Engine registration
        VkApplicationInfo appInfo;
        VkUtils::zeroStruct(appInfo, VK_STRUCTURE_TYPE_APPLICATION_INFO);
        appInfo.pApplicationName = "ToolkitApp";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "ToolkitGraphics";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION;

        VkInstanceCreateInfo instInfo;
        VkUtils::zeroStruct(instInfo, VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);
#if TK_MACOS || TK_IOS
        instInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
        instInfo.pApplicationInfo = &appInfo;
        LogInfo("VulkanDevice::create before getInstanceLayersAndExtensions");
        getInstanceLayersAndExtensions(instanceExtensions, instanceLayers, SupportsDebugUtilsExt);
        LogInfo("VulkanDevice::create after getInstanceLayersAndExtensions extensions=%d layers=%d", instanceExtensions.count(), instanceLayers.count());
        instInfo.enabledExtensionCount = instanceExtensions.count();
        instInfo.ppEnabledExtensionNames = instInfo.enabledExtensionCount > 0 ? static_cast<const char *const *>(instanceExtensions.get()) : nullptr;
        instInfo.enabledLayerCount = instanceLayers.count();
        instInfo.ppEnabledLayerNames = instInfo.enabledLayerCount > 0 ? instanceLayers.get() : nullptr;
#if VK_USE_DEBUG_LAYER
        SupportsDebugCallbackExt = !SupportsDebugUtilsExt && VkUtils::hasExtension(instanceExtensions, VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

        // Create Vulkan instance
        LogInfo("VulkanDevice::create before vkCreateInstance");
        result = vkCreateInstance(&instInfo, nullptr, &instance);
        LogInfo("VulkanDevice::create after vkCreateInstance result=%d instance=%p", static_cast<i32>(result), instance);
        if (result == VK_ERROR_INCOMPATIBLE_DRIVER)
        {
            // Missing driver
#if TK_MACOS || TK_IOS
            TK_ASSERT(false && "Cannot find a compatible Metal driver.");
#else
            TK_ASSERT(false && "Cannot find a compatible Vulkan driver.");
#endif
            return nullptr;
        }
        if (result == VK_ERROR_EXTENSION_NOT_PRESENT)
        {
            // Extensions error
            u32 propertyCount;
            vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr);
            TkArray<VkExtensionProperties> properties;
            properties.resize(propertyCount);
            vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, properties.get());
            for (const char *extension : instanceExtensions)
            {
                bool found = false;
                for (u32 propertyIndex = 0; propertyIndex < propertyCount; propertyIndex++)
                {
                    if (!TkStringUtils::compareStrings(properties[propertyIndex].extensionName, extension))
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    LogWarn("Missing required Vulkan extension: %s", extension);
                }
            }
            TkString error = "Vulkan driver doesn't contain specified extensions:\nPlease make sure your layers path is set appropriately.";
            TK_ASSERT(false && *error);
            return nullptr;
        }
        if (result != VK_SUCCESS)
        {
            // Driver error
            LogWarn("Vulkan create instance failed with error code: %s", VkUtils::getErrorString(result));
            TK_ASSERT(false && "Vulkan failed to create instance\n\nDo you have a compatible Vulkan driver installed?");
            return nullptr;
        }

        // Setup bindings
        LogInfo("VulkanDevice::create before volkLoadInstance");
        volkLoadInstance(instance);
        LogInfo("VulkanDevice::create after volkLoadInstance");
        if (!vkEnumeratePhysicalDevices ||
            !vkGetPhysicalDeviceProperties ||
            !vkGetPhysicalDeviceQueueFamilyProperties ||
            !vkGetPhysicalDeviceFeatures ||
            !vkEnumerateDeviceExtensionProperties ||
            !vkEnumerateDeviceLayerProperties ||
            !vkGetPhysicalDeviceFormatProperties ||
            !vkGetPhysicalDeviceMemoryProperties ||
            !vkCreateDevice ||
            !vkDestroyInstance)
        {
            LogWarn("Graphics device init failed because required Vulkan instance entry points are missing.");
            if (vkDestroyInstance)
                vkDestroyInstance(instance, nullptr);
            instance = VK_NULL_HANDLE;
            return nullptr;
        }

        // Setup debug layer
#if VK_USE_DEBUG_LAYER
        LogInfo("VulkanDevice::create before setupDebugLayerCallback");
        setupDebugLayerCallback();
        LogInfo("VulkanDevice::create after setupDebugLayerCallback");
#endif

        // Enumerate all GPU devices and pick one
        i32 selectedAdapterIndex = -1;
        u32 gpuCount = 0;
        LogInfo("VulkanDevice::create before vkEnumeratePhysicalDevices count");
        result = vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);
        LogInfo("VulkanDevice::create after vkEnumeratePhysicalDevices count result=%d count=%u", static_cast<i32>(result), gpuCount);
        if (result != VK_SUCCESS)
        {
            LogWarn("Vulkan enumerate physical devices failed with error code: %s", VkUtils::getErrorString(result));
            vkDestroyInstance(instance, nullptr);
            instance = VK_NULL_HANDLE;
            return nullptr;
        }
        if (gpuCount <= 0)
        {
            LogWarn("No valid GPU found for Vulkan.");
            TK_ASSERT(false && "Vulkan failed to create instance\n\nDo you have a Vulkan-compatible GPU?");
            vkDestroyInstance(instance, nullptr);
            instance = VK_NULL_HANDLE;
            return nullptr;
        }
        TK_ASSERT(gpuCount >= 1);
        TkArray<VkPhysicalDevice> gpus;
        gpus.resize(gpuCount);
        LogInfo("VulkanDevice::create before vkEnumeratePhysicalDevices list");
        result = vkEnumeratePhysicalDevices(instance, &gpuCount, gpus.get());
        LogInfo("VulkanDevice::create after vkEnumeratePhysicalDevices list result=%d count=%u", static_cast<i32>(result), gpuCount);
        if (result != VK_SUCCESS)
        {
            LogWarn("Vulkan enumerate physical devices failed with error code: %s", VkUtils::getErrorString(result));
            vkDestroyInstance(instance, nullptr);
            instance = VK_NULL_HANDLE;
            return nullptr;
        }
        TkArray<VulkanAdapter> adapters;
        adapters.resize(gpuCount);
        for (u32 gpuIndex = 0; gpuIndex < gpuCount; gpuIndex++)
        {
            LogInfo("VulkanDevice::create before adapter properties index=%u", gpuIndex);
            VulkanAdapter &adapter = adapters[gpuIndex];
            adapter.device = gpus[gpuIndex];
            vkGetPhysicalDeviceProperties(adapter.device, &adapter.deviceProps);
            LogInfo("VulkanDevice::create after adapter properties index=%u", gpuIndex);
            adapter.desc = adapter.deviceProps.deviceName;

            const char *type;
            switch (adapter.deviceProps.deviceType)
            {
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                type = "Other";
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                type = "Integrated GPU";
                break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                type = "Discrete GPU";
                // Select the first discrete GPU device
                if (selectedAdapterIndex == -1)
                    selectedAdapterIndex = gpuIndex;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                type = "Virtual GPU";
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                type = "CPU";
                break;
            default:
                type = "Unknown";
            }

            LogInfo("Adapter %u: '%s', API %u.%u.%u, Driver %u.%u.%u", gpuIndex, *adapter.desc, VK_VERSION_MAJOR(adapter.deviceProps.apiVersion), VK_VERSION_MINOR(adapter.deviceProps.apiVersion), VK_VERSION_PATCH(adapter.deviceProps.apiVersion), VK_VERSION_MAJOR(adapter.deviceProps.driverVersion), VK_VERSION_MINOR(adapter.deviceProps.driverVersion), VK_VERSION_PATCH(adapter.deviceProps.driverVersion));
            LogInfo("	VendorId: 0x%x, Type: %s, Max Descriptor Sets Bound: %u, Timestamps: %d", adapter.deviceProps.vendorID, type, adapter.deviceProps.limits.maxBoundDescriptorSets, !!adapter.deviceProps.limits.timestampComputeAndGraphics);
        }

        // Select the adapter to use
        if (selectedAdapterIndex < 0)
            selectedAdapterIndex = 0;
        if (adapters.count() == 0 || selectedAdapterIndex >= adapters.count())
        {
            LogError("Failed to find valid Vulkan adapter!");
            return nullptr;
        }
        u32 vendorId = 0;
        if (vendorId != 0)
        {
            for (i32 i = 0; i < adapters.count(); i++)
            {
                if (adapters[i].getVendorId() == vendorId)
                {
                    selectedAdapterIndex = i;
                    break;
                }
            }
        }
        TK_ASSERT(adapters[selectedAdapterIndex].isValid());

        // Create device
        LogInfo("VulkanDevice::create before adapter copy selected=%d", selectedAdapterIndex);
        VulkanAdapter *adapterCopy = TkMemory::newObject<VulkanAdapter>(TkMemory::getDefault(), adapters[selectedAdapterIndex]);
        LogInfo("VulkanDevice::create after adapter copy %p", adapterCopy);
        LogInfo("VulkanDevice::create before VulkanDevice constructor");
        VulkanDevice *device = TkMemory::newObject<VulkanDevice>(TkMemory::getDefault(), GfxShaderProfile::kVulkan_SM5, adapterCopy);
        LogInfo("VulkanDevice::create after VulkanDevice constructor %p", device);
        return device;
    }

    VulkanDevice::~VulkanDevice()
    {
        // Ensure to be disposed
        VulkanDevice::dispose();
    }

    VulkanRenderPass *VulkanDevice::getOrCreateRenderPass(VulkanRenderTargetLayout &layout)
    {
        if (m_renderPasses.containsKey(layout))
        {
            VulkanRenderPass *renderPass = m_renderPasses.at(layout);
            return renderPass;
        }

        PROFILE_NAMED("Create Render Pass");
        VulkanRenderPass *renderPass = TkMemory::newObject<VulkanRenderPass>(TkMemory::getDefault(), this, layout);
        m_renderPasses.add(layout, renderPass);
        return renderPass;
    }

    VulkanFramebuffer *VulkanDevice::getOrCreateFramebuffer(VulkanFramebuffer::Key &key, VkExtent2D &extent, u32 layers)
    {
        if (m_framebuffers.containsKey(key))
        {
            VulkanFramebuffer *framebuffer = m_framebuffers.at(key);
            return framebuffer;
        }

        PROFILE_NAMED("Create Framebuffer");
        VulkanFramebuffer *framebuffer = TkMemory::newObject<VulkanFramebuffer>(TkMemory::getDefault(), this, key, extent, layers);
        m_framebuffers.add(key, framebuffer);
        return framebuffer;
    }

    VulkanPipelineLayout *VulkanDevice::getOrCreateLayout(VulkanDescriptorSetLayoutInfo &key)
    {
        if (m_layouts.containsKey(key))
        {
            VulkanPipelineLayout *layout = m_layouts.at(key);
            return layout;
        }

        PROFILE_NAMED("Create Pipeline layout");
        VulkanPipelineLayout *layout = TkMemory::newObject<VulkanPipelineLayout>(TkMemory::getDefault(), this, key);
        m_layouts.add(key, layout);
        return layout;
    }

    void VulkanDevice::onImageViewDestroy(VkImageView imageView)
    {
        for (auto i = m_framebuffers.Begin(); i.isNotEnd(); ++i)
        {
            if (i->value->hasReference(imageView))
            {
                TkMemory::deletePtr(i->value);
                m_framebuffers.remove(i);
                --i;
            }
        }
    }

    void VulkanDevice::setupPresentQueue(VkSurfaceKHR surface)
    {
        if (presentQueue)
            return;

        const auto supportsPresent = [surface](VkPhysicalDevice physicalDevice, VulkanQueue *queue)
        {
            VkBool32 supportsPresent = VK_FALSE;
            const u32 queueFamilyIndex = queue->getFamilyIndex();
            VALIDATE_VK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, &supportsPresent));
            if (supportsPresent)
            {
                LogInfo("Vulkan Queue Family %d: supports present", queueFamilyIndex);
            }
            return supportsPresent == VK_TRUE;
        };

        const auto gpu = adapter->device;
        bool graphics = supportsPresent(gpu, graphicsQueue);
        if (!graphics)
        {
            LogError("Vulkan Graphics Queue doesn't support present");
        }
        // TODO: test using Compute queue for present
        // bool compute = SupportsPresent(gpu, ComputeQueue);
        presentQueue = graphicsQueue;
    }

    PixelFormat VulkanDevice::getClosestSupportedPixelFormat(PixelFormat format, GfxTextureFlags flags, bool optimalTiling)
    {
        // Collect features to use
        VkFormatFeatureFlags wantedFeatureFlags = 0;
        if (enumHasAnyFlags(flags, GfxTextureFlags::kShaderResource))
            wantedFeatureFlags |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
        if (enumHasAnyFlags(flags, GfxTextureFlags::kRenderTarget))
            wantedFeatureFlags |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
        if (enumHasAnyFlags(flags, GfxTextureFlags::kDepthStencil))
            wantedFeatureFlags |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (enumHasAnyFlags(flags, GfxTextureFlags::kUnorderedAccess))
            wantedFeatureFlags |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;

        if (!isVkFormatSupported(VkUtils::toVulkanFormat(format), wantedFeatureFlags, optimalTiling))
        {
            // Special case for depth-stencil formats
            if (enumHasAnyFlags(flags, GfxTextureFlags::kDepthStencil))
            {
                const bool hasStencil = GfxPixelFormat::hasStencil(format);

                // Spec guarantees at least one depth-only, and one depth-stencil format to be supported
                if (hasStencil)
                {
                    if (isVkFormatSupported(VK_FORMAT_D32_SFLOAT_S8_UINT, wantedFeatureFlags, optimalTiling))
                        format = PixelFormat::kD32_Float;
                    else
                        format = PixelFormat::kD24_UNorm_S8_UInt;
                }
                else
                {
                    // The only format that could have failed is 32-bit depth, so we must use the alternative 16-bit. Spec guarantees it is always supported.
                    format = PixelFormat::kD16_UNorm;
                }
            }
            else
            {
                // Perform remapping to bigger format that might be supported (more likely)
                auto remap = format;
                switch (format)
                {
                case PixelFormat::kR11G11B10_Float:
                case PixelFormat::kR10G10B10A2_UNorm:
                    remap = PixelFormat::kR16G16B16A16_Float;
                    break;
                case PixelFormat::kR16_Float:
                    remap = PixelFormat::kR32_Float;
                    break;
                case PixelFormat::kR16G16_UNorm:
                case PixelFormat::kR16G16_Float:
                    remap = PixelFormat::kR32G32_Float;
                    break;
                case PixelFormat::kR32G32B32A32_Float:
                    // RGBA32 is essential
                    return PixelFormat::kUnknown;
                default:
                    // Ultimate performance eater
                    remap = PixelFormat::kR32G32B32A32_Float;
                    break;
                }
#if TK_DEBUG
                if (format != remap)
                {
                    LogWarn("Unsupported Vulkan format %d. Remapping to %d", static_cast<i32>(format), static_cast<i32>(remap));
                    format = getClosestSupportedPixelFormat(remap, flags, optimalTiling);
                }
#endif
            }
        }

        return format;
    }

    void getPipelineCachePath(TkString &path)
    {
        path = System::getWorkDir() / "VulkanPipeline.cache";
    }

    bool VulkanDevice::savePipelineCache()
    {
        if (pipelineCache == VK_NULL_HANDLE || !vkGetPipelineCacheData)
            return false;

        // Query data size
        size_t dataSize = 0;
        VkResult result = vkGetPipelineCacheData(device, pipelineCache, &dataSize, nullptr);
        LOG_VK_RESULT_WITH_RETURN(result);
        if (dataSize <= 0)
            return false;

        // Query data
        TkArray<byte> data;
        data.resize((i32)dataSize);
        result = vkGetPipelineCacheData(device, pipelineCache, &dataSize, data.get());
        LOG_VK_RESULT_WITH_RETURN(result);

        // Save data
        TkString path;
        getPipelineCachePath(path);

        //
        OpenFileParams openParams{.path = path, .mode = FileMode::kCreateAlways, .access = FileAccess::kWrite, .share = FileShare::kRead};
        auto handle = TkFileSystem::openFile(openParams);
        if (!handle)
            return false;
        bool r = TkFileSystem::writeBytes(handle, data);
        TkFileSystem::closeFile(handle);
        return r;
    }

#if VK_USE_VALIDATION_CACHE

    void getValidationCachePath(TkString &path)
    {
        path = System::getWorkDir() / "VulkanValidation.cache";
    }

    bool VulkanDevice::saveValidationCache()
    {
        if (validationCache == VK_NULL_HANDLE || !vkGetValidationCacheDataEXT)
            return false;

        // Query data size
        size_t dataSize = 0;
        VkResult result = vkGetValidationCacheDataEXT(device, validationCache, &dataSize, nullptr);
        LOG_VK_RESULT_WITH_RETURN(result);
        if (dataSize <= 0)
            return false;

        // Query data
        TkArray<byte> data;
        data.resize((i32)dataSize);
        result = vkGetValidationCacheDataEXT(device, validationCache, &dataSize, data.get());
        LOG_VK_RESULT_WITH_RETURN(result);

        // Save data
        TkString path;
        getValidationCachePath(path);
        OpenFileParams openParams{.path = path, .mode = FileMode::kCreateAlways, .access = FileAccess::kWrite, .share = FileShare::kRead};
        auto handle = TkFileSystem::openFile(openParams);
        if (!handle)
            return false;
        const bool result = TkFileSystem::writeBytes(handle, data);
        TkFileSystem::closeFile(handle);
        return result;
    }

#endif

    bool VulkanDevice::isVkFormatSupported(VkFormat vkFormat, VkFormatFeatureFlags wantedFeatureFlags, bool optimalTiling) const
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(adapter->device, vkFormat, &props);
        const VkFormatFeatureFlags featureFlags = optimalTiling ? props.optimalTilingFeatures : props.linearTilingFeatures;
        if ((featureFlags & wantedFeatureFlags) != wantedFeatureFlags)
            return false;

        // VkImageFormatProperties imageProps;
        // vkGetPhysicalDeviceImageFormatProperties(Adapter->device, vkFormat, , &imageProps);

        return true;
    }

    GfxContext *VulkanDevice::getMainContext()
    {
        return reinterpret_cast<GfxContext *>(mainContext);
    }

    GfxAdapter *VulkanDevice::getAdapter() const
    {
        return static_cast<GfxAdapter *>(adapter);
    }

    void *VulkanDevice::getNativePtr() const
    {
        // Return both Instance and device as pointer to void*[2]
        m_nativePtr[0] = (void *)instance;
        m_nativePtr[1] = (void *)device;
        return m_nativePtr;
    }

    static i32 getMaxSampleCount(VkSampleCountFlags counts)
    {
        if (counts & VK_SAMPLE_COUNT_64_BIT)
        {
            return VK_SAMPLE_COUNT_64_BIT;
        }
        if (counts & VK_SAMPLE_COUNT_32_BIT)
        {
            return VK_SAMPLE_COUNT_32_BIT;
        }
        if (counts & VK_SAMPLE_COUNT_16_BIT)
        {
            return VK_SAMPLE_COUNT_16_BIT;
        }
        if (counts & VK_SAMPLE_COUNT_8_BIT)
        {
            return VK_SAMPLE_COUNT_8_BIT;
        }
        if (counts & VK_SAMPLE_COUNT_4_BIT)
        {
            return VK_SAMPLE_COUNT_4_BIT;
        }
        if (counts & VK_SAMPLE_COUNT_2_BIT)
        {
            return VK_SAMPLE_COUNT_2_BIT;
        }
        return VK_SAMPLE_COUNT_1_BIT;
    }

    bool VulkanDevice::init()
    {
        LogInfo("Vulkan init: begin");
        totalMemory = 0;

        m_state = DeviceState::kCreated;
        if (!adapter || !adapter->isValid())
        {
            LogError("Invalid Vulkan adapter.");
            return false;
        }

        const auto gpu = adapter->device;

        // Get queues properties
        LogInfo("Vulkan init: before queue family query");
        u32 queueCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueCount, nullptr);
        TK_ASSERT(queueCount >= 1);
        queueFamilyProps.addDefault(queueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueCount, queueFamilyProps.get());

        // Query device features
        vkGetPhysicalDeviceFeatures(gpu, &physicalDeviceFeatures);

        // Get extensions and layers
        LogInfo("Vulkan init: before device extensions/layers");
        TkArray<const char *> deviceExtensions;
        TkArray<const char *> validationLayers;
        getDeviceExtensionsAndLayers(gpu, deviceExtensions, validationLayers);
        parseOptionalDeviceExtensions(deviceExtensions);

        // Setup device info
        VkDeviceCreateInfo deviceInfo;
        VkUtils::zeroStruct(deviceInfo, VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);
        deviceInfo.enabledExtensionCount = deviceExtensions.count();
        deviceInfo.ppEnabledExtensionNames = deviceExtensions.get();
        deviceInfo.enabledLayerCount = validationLayers.count();
        deviceInfo.ppEnabledLayerNames = deviceInfo.enabledLayerCount > 0 ? validationLayers.get() : nullptr;

        // Setup queues info
        TkArray<VkDeviceQueueCreateInfo> queueFamilyInfos;
        i32 graphicsQueueFamilyIndex = -1;
        i32 computeQueueFamilyIndex = -1;
        i32 transferQueueFamilyIndex = -1;
        LogInfo("Found %d queue families:", queueFamilyProps.count());
        u32 numPriorities = 0;
        for (i32 familyIndex = 0; familyIndex < queueFamilyProps.count(); familyIndex++)
        {
            const VkQueueFamilyProperties &curProps = queueFamilyProps[familyIndex];

            bool isValidQueue = false;
            if ((curProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
            {
                if (graphicsQueueFamilyIndex == -1)
                {
                    graphicsQueueFamilyIndex = familyIndex;
                    isValidQueue = true;
                }
                else
                {
                    // TODO: Support for multi-queue and choose the best queue
                }
            }

            if ((curProps.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT)
            {
                if (computeQueueFamilyIndex == -1 && graphicsQueueFamilyIndex != familyIndex)
                {
                    computeQueueFamilyIndex = familyIndex;
                    isValidQueue = true;
                }
            }

            if ((curProps.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT)
            {
                // Favor a non-gfx transfer queue
                if (transferQueueFamilyIndex == -1 && (curProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) != VK_QUEUE_GRAPHICS_BIT && (curProps.queueFlags & VK_QUEUE_COMPUTE_BIT) != VK_QUEUE_COMPUTE_BIT)
                {
                    transferQueueFamilyIndex = familyIndex;
                    isValidQueue = true;
                }
            }

            TkString queueTypeInfo;
            if ((curProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
                queueTypeInfo += " graphics";
            if ((curProps.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT)
                queueTypeInfo += " compute";
            if ((curProps.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT)
                queueTypeInfo += " transfer";
            if ((curProps.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) == VK_QUEUE_SPARSE_BINDING_BIT)
                queueTypeInfo += " sparse";

            if (!isValidQueue)
            {
                LogInfo("Skipping unnecessary queue family %d: %u queues%s", familyIndex, curProps.queueCount, *queueTypeInfo);
                continue;
            }

            const i32 queueIndex = queueFamilyInfos.count();
            queueFamilyInfos.addZeroed(1);
            VkDeviceQueueCreateInfo &curQueue = queueFamilyInfos[queueIndex];
            curQueue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            curQueue.queueFamilyIndex = familyIndex;
            curQueue.queueCount = curProps.queueCount;
            numPriorities += curProps.queueCount;
            LogInfo("- queue family %d: %u queues%s", familyIndex, curProps.queueCount, *queueTypeInfo);
        }
        TkArray<float> queuePriorities;
        queuePriorities.addDefault(numPriorities);
        float *currentPriority = queuePriorities.get();
        for (i32 index = 0; index < queueFamilyInfos.count(); index++)
        {
            VkDeviceQueueCreateInfo &queue = queueFamilyInfos[index];
            queue.pQueuePriorities = currentPriority;
            const VkQueueFamilyProperties &properties = queueFamilyProps[queue.queueFamilyIndex];
            for (i32 queueIndex = 0; queueIndex < (i32)properties.queueCount; queueIndex++)
                *currentPriority++ = 1.0f;
        }
        deviceInfo.queueCreateInfoCount = queueFamilyInfos.count();
        deviceInfo.pQueueCreateInfos = queueFamilyInfos.get();

        VkPhysicalDeviceFeatures enabledFeatures;
        VulkanPlatform::restrictEnabledPhysicalDeviceFeatures(physicalDeviceFeatures, enabledFeatures);
        deviceInfo.pEnabledFeatures = &enabledFeatures;

        // Create the device
        LogInfo("Vulkan init: before vkCreateDevice");
        const VkResult createDeviceResult = vkCreateDevice(gpu, &deviceInfo, nullptr, &device);
        if (createDeviceResult != VK_SUCCESS)
        {
            LogError("Vulkan logical device creation failed with error code: %s", VkUtils::getErrorString(createDeviceResult));
            device = VK_NULL_HANDLE;
            return false;
        }
        LogInfo("Vulkan init: after vkCreateDevice");

        // Optimize bindings
        LogInfo("Vulkan init: before volkLoadDevice");
        volkLoadDevice(device);
        if (!vkGetDeviceQueue ||
            !vkCreateCommandPool ||
            !vkAllocateCommandBuffers ||
            !vkCreateSemaphore ||
            !vkCreateFence ||
            !vkCreateSampler ||
            !vkAllocateMemory ||
            !vkFreeMemory ||
            !vkMapMemory ||
            !vkUnmapMemory ||
            !vkFlushMappedMemoryRanges ||
            !vkInvalidateMappedMemoryRanges ||
            !vkBindBufferMemory ||
            !vkBindImageMemory ||
            !vkGetBufferMemoryRequirements ||
            !vkGetImageMemoryRequirements ||
            !vkCreateBuffer ||
            !vkDestroyBuffer ||
            !vkCreateImage ||
            !vkDestroyImage ||
            !vkCmdCopyBuffer)
        {
            LogError("Vulkan logical device is missing required device entry points.");
            return false;
        }
        LogInfo("Vulkan init: after volkLoadDevice");

        // Create queues
        LogInfo("Vulkan init: before create queues");
        if (graphicsQueueFamilyIndex == -1)
        {
            LogError("Missing Vulkan graphics queue.");
            return false;
        }
        graphicsQueue = TkMemory::newObject<VulkanQueue>(TkMemory::getDefault(), this, graphicsQueueFamilyIndex);
        computeQueue = computeQueueFamilyIndex != -1 ? TkMemory::newObject<VulkanQueue>(TkMemory::getDefault(), this, computeQueueFamilyIndex) : graphicsQueue;
        transferQueue = transferQueueFamilyIndex != -1 ? TkMemory::newObject<VulkanQueue>(TkMemory::getDefault(), this, transferQueueFamilyIndex) : graphicsQueue;
        LogInfo("Vulkan init: after create queues graphics=%p compute=%p transfer=%p", graphicsQueue, computeQueue, transferQueue);

        // Init device limits
        LogInfo("Vulkan init: before limits/features");
        {
            physicalDeviceLimits = adapter->deviceProps.limits;
            GfxMSAALevel maxMsaa = GfxMSAALevel::kNone;
            if (physicalDeviceFeatures.sampleRateShading)
            {
                const i32 framebufferColorSampleCounts = getMaxSampleCount(physicalDeviceLimits.framebufferColorSampleCounts);
                const i32 framebufferDepthSampleCounts = getMaxSampleCount(physicalDeviceLimits.framebufferDepthSampleCounts);
                maxMsaa = (GfxMSAALevel)TkMath::clamp(TkMath::min<i32>(framebufferColorSampleCounts, framebufferDepthSampleCounts), 1, 8);
            }

            auto &limits = this->limits;
            limits.hasCompute = getShaderProfile() == GfxShaderProfile::kVulkan_SM5 && physicalDeviceLimits.maxComputeWorkGroupCount[0] >= GFX_MAX_CS_DISPATCH_THREAD_GROUPS && physicalDeviceLimits.maxComputeWorkGroupCount[1] >= GFX_MAX_CS_DISPATCH_THREAD_GROUPS;
#if GFX_ALLOW_TESSELLATION_SHADERS
            limits.hasTessellation = !!physicalDeviceFeatures.tessellationShader && physicalDeviceLimits.maxBoundDescriptorSets > (u32)VulkanDescriptorSet::kDomain;
#else
            limits.hasTessellation = false;
#endif
#if GFX_ALLOW_GEOMETRY_SHADERS
            limits.hasGeometryShaders = !!physicalDeviceFeatures.geometryShader && physicalDeviceLimits.maxBoundDescriptorSets > (u32)VulkanDescriptorSet::kGeometry;
#else
            limits.hasGeometryShaders = false;
#endif
            limits.hasInstancing = true;
            limits.hasVolumeTextureRendering = true;
            limits.hasDrawIndirect = physicalDeviceLimits.maxDrawIndirectCount >= 1;
            limits.hasAppendConsumeBuffers = false; // TODO: add Append Consume buffers support for Vulkan
            limits.hasSeparateRenderTargetBlendState = true;
            limits.hasDepthClip = physicalDeviceFeatures.depthClamp;
            limits.hasDepthAsSRV = true;
            limits.hasReadOnlyDepth = true;
            limits.hasMultisampleDepthAsSRV = !!physicalDeviceFeatures.sampleRateShading;
            limits.hasTypedUAVLoad = true;
            limits.maxMipLevelsCount = TkMath::min(static_cast<i32>(log2(physicalDeviceLimits.maxImageDimension2D)), GFX_MAX_TEXTURE_MIP_LEVELS);
            limits.maxTexture1DSize = physicalDeviceLimits.maxImageDimension1D;
            limits.maxTexture1DArraySize = physicalDeviceLimits.maxImageArrayLayers;
            limits.maxTexture2DSize = physicalDeviceLimits.maxImageDimension2D;
            limits.maxTexture2DArraySize = physicalDeviceLimits.maxImageArrayLayers;
            limits.maxTexture3DSize = physicalDeviceLimits.maxImageDimension3D;
            limits.maxTextureCubeSize = physicalDeviceLimits.maxImageDimensionCube;
            limits.maxSamplerAnisotropy = physicalDeviceLimits.maxSamplerAnisotropy;

            for (i32 i = 0; i < static_cast<i32>(PixelFormat::MAX); i++)
            {
                const auto format = static_cast<PixelFormat>(i);
                const auto vkFormat = VkUtils::toVulkanFormat(format);

                GfxMSAALevel msaa = GfxMSAALevel::kNone;
                FormatSupport support = FormatSupport::kNone;

                if (vkFormat != VK_FORMAT_UNDEFINED)
                {
                    VkFormatProperties properties;
                    TkMemory::memoryClear(&properties, sizeof(properties));
                    vkGetPhysicalDeviceFormatProperties(gpu, vkFormat, &properties);

                    // Query image format features support flags
#define CHECK_IMAGE_FORMAT(bit, feature)                                                                       \
    if (((properties.linearTilingFeatures & bit) == bit) || ((properties.optimalTilingFeatures & bit) == bit)) \
    support |= feature
                    if (properties.linearTilingFeatures != 0 || properties.optimalTilingFeatures != 0)
                        support |= FormatSupport::kTexture1D | FormatSupport::kTexture2D | FormatSupport::kTexture3D | FormatSupport::kTextureCube;
                    CHECK_IMAGE_FORMAT(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT, FormatSupport::kShaderLoad);
                    // VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT,
                    // VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT
                    CHECK_IMAGE_FORMAT(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT, FormatSupport::kRenderTarget);
                    CHECK_IMAGE_FORMAT(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT, FormatSupport::kBlendable);
                    CHECK_IMAGE_FORMAT(VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, FormatSupport::kDepthStencil);
                    // VK_FORMAT_FEATURE_BLIT_SRC_BIT
                    // VK_FORMAT_FEATURE_BLIT_DST_BIT
                    CHECK_IMAGE_FORMAT(VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT, FormatSupport::kShaderSample | FormatSupport::kShaderSampleComparison);
#undef CHECK_IMAGE_FORMAT

                    // Query buffer format features support flags
#define CHECK_BUFFER_FORMAT(bit, feature)         \
    if ((properties.bufferFeatures & bit) == bit) \
    support |= feature
                    if (properties.bufferFeatures != 0)
                        support |= FormatSupport::kBuffer;
                    // VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT
                    // VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT
                    // VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT
                    CHECK_BUFFER_FORMAT(VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT, FormatSupport::kInputAssemblyVertexBuffer);
#undef CHECK_BUFFER_FORMAT

                    // Unused bits
                    // VK_FORMAT_FEATURE_TRANSFER_SRC_BIT
                    // VK_FORMAT_FEATURE_TRANSFER_DST_BIT
                    // VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT
                    // VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT
                    // VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT
                    // VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT
                    // VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT
                    // VK_FORMAT_FEATURE_DISJOINT_BIT
                    // VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT
                    // VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG
                    // VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT_EXT

                    // Multi-sampling support
                    if (enumHasAnyFlags(support, FormatSupport::kTexture2D))
                        msaa = maxMsaa;
                }

                features[i] = FormatFeatures(format, msaa, support);
            }
        }

        // Setup memory limit and print memory info
        LogInfo("Vulkan init: before memory properties");
        {
            VkPhysicalDeviceMemoryProperties memoryProperties;
            vkGetPhysicalDeviceMemoryProperties(gpu, &memoryProperties);
            LogInfo("Max memory allocations: %u", adapter->deviceProps.limits.maxMemoryAllocationCount);
            LogInfo("Found %u device memory heaps:", memoryProperties.memoryHeapCount);
            for (u32 i = 0; i < memoryProperties.memoryHeapCount; i++)
            {
                const VkMemoryHeap &heap = memoryProperties.memoryHeaps[i];
                bool isGPUHeap = (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;
                LogInfo("-  memory heap %u: flags 0x%x, size %u MB (GPU: %d)", i, heap.flags, (u32)(heap.size / 1024 / 1024), isGPUHeap);
                if (isGPUHeap)
                    totalMemory += heap.size;
            }
            LogInfo("Found %u device memory types:", memoryProperties.memoryTypeCount);
            for (u32 i = 0; i < memoryProperties.memoryTypeCount; i++)
            {
                const VkMemoryType &type = memoryProperties.memoryTypes[i];
                TkString flagsInfo;
                if ((type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
                    flagsInfo += "local, ";
                if ((type.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
                    flagsInfo += "host visible, ";
                if ((type.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
                    flagsInfo += "host coherent, ";
                if ((type.propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) == VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
                    flagsInfo += "host cached, ";
                if ((type.propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) == VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
                    flagsInfo += "lazy, ";
                if (flagsInfo.hasChars())
                    flagsInfo = ", properties: " + flagsInfo.left(flagsInfo.length() - 2);
                LogInfo("-  memory type %u: flags 0x%x, heap %u%s", i, type.propertyFlags, type.heapIndex, flagsInfo.hasChars() ? *flagsInfo : "");
            }
        }

        // Initialize memory allocator
        LogInfo("Vulkan init: before vmaCreateAllocator");
        {
            VmaVulkanFunctions vulkanFunctions = {};
            // VMA_DYNAMIC_VULKAN_FUNCTIONS only needs the two loader entry points.
            // VMA resolves the rest from the instance/device, which keeps it in sync with volk.
            vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
            vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
            VmaAllocatorCreateInfo allocatorInfo = {};
            allocatorInfo.vulkanApiVersion = VK_API_VERSION;
            allocatorInfo.physicalDevice = gpu;
            allocatorInfo.instance = instance;
            allocatorInfo.device = device;
            allocatorInfo.pVulkanFunctions = &vulkanFunctions;
            const VkResult allocatorResult = vmaCreateAllocator(&allocatorInfo, &allocator);
            if (allocatorResult != VK_SUCCESS)
            {
                LogError("Vulkan memory allocator creation failed with error code: %s", VkUtils::getErrorString(allocatorResult));
                allocator = VK_NULL_HANDLE;
                return false;
            }
        }
        LogInfo("Vulkan init: after vmaCreateAllocator allocator=%p", allocator);

#if defined(VK_KHR_display) && 0
        // Enumerate displays and supported video resolutions
        u32 displaysCount;
        // TODO: for some reason vkGetPhysicalDeviceDisplayPropertiesKHR returns 0 displays (on NVIDIA 1070)
        if (vkGetPhysicalDeviceDisplayPropertiesKHR && vkGetPhysicalDeviceDisplayPropertiesKHR(gpu, &displaysCount, nullptr) == VK_SUCCESS && displaysCount != 0)
        {
            TkArray < VkDisplayPropertiesKHR, 4 >> displays;
            displays.resize(displaysCount);
            vkGetPhysicalDeviceDisplayPropertiesKHR(gpu, &displaysCount, displays.get());
            TK_ASSERT(displaysCount == displays.count());
            TkArray < VkDisplayModePropertiesKHR, 32 >> displayProperties;
            for (auto &display : displays)
            {
                LogInfo("Video output '%s' %ux%u", display.displayName, display.physicalResolution.width, display.physicalResolution.height);

                u32 propertiesCount = 0;
                vkGetDisplayModePropertiesKHR(gpu, display.display, &propertiesCount, nullptr);
                displayProperties.resize(propertiesCount);
                vkGetDisplayModePropertiesKHR(gpu, display.display, &propertiesCount, displayProperties.get());
                for (auto &displayProperty : displayProperties)
                {
                    //..
                }
            }
        }
#endif

        // Prepare stuff
        LogInfo("Vulkan init: before managers");
        fenceManager.init(this);
        uniformBufferUploader = TkMemory::newObject<VulkanUniformBufferUploader>(TkMemory::getDefault(), this);
        descriptorPoolsManager = TkMemory::newObject<VulkanDescriptorPoolsManager>(TkMemory::getDefault(), this);
        LogInfo("Vulkan init: before mainContext");
        mainContext = TkMemory::newObject<VulkanContext>(TkMemory::getDefault(), this, graphicsQueue);
        LogInfo("Vulkan init: after mainContext %p", mainContext);
        if (vkCreatePipelineCache)
        {
            TkArray<u8> data;
            TkString path;
            getPipelineCachePath(path);
            if (TkFileSystem::isFileExist(path))
            {
                LogInfo("Trying to load Vulkan pipeline cache file %s", *path);
                auto handle = TkFileSystem::openFile(path);
                TkFileSystem::readAsBytes(handle, data);
                TkFileSystem::closeFile(handle);
            }
            VkPipelineCacheCreateInfo pipelineCacheCreateInfo;
            VkUtils::zeroStruct(pipelineCacheCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO);
            pipelineCacheCreateInfo.initialDataSize = data.count();
            pipelineCacheCreateInfo.pInitialData = data.count() > 0 ? data.get() : nullptr;
            const VkResult result = vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache);
            LOG_VK_RESULT(result);
        }
#if VK_USE_VALIDATION_CACHE
        if (optionalDeviceExtensions.hasEXTValidationCache && vkCreateValidationCacheEXT && vkDestroyValidationCacheEXT)
        {
            TkArray<u8> data;
            TkString path;
            getValidationCachePath(path);
            if (TkFileSystem::isFileExist(path))
            {
                LogInfo("Trying to load Vulkan validation cache file %s", *path);
                auto handle = TkFileSystem::openFile(path);
                if (handle)
                {
                    TkFileSystem::readAsBytes(handle, data);
                    TkFileSystem::closeFile(handle);
                }
                if (data.hasItems())
                {
                    i32 *dataPtr = (i32 *)data.get();
                    if (*dataPtr > 0)
                    {
                        const i32 cacheSize = *dataPtr++;
                        const i32 cacheVersion = *dataPtr++;
                        const i32 cacheVersionExpected = VK_PIPELINE_CACHE_HEADER_VERSION_ONE;
                        if (cacheVersion == cacheVersionExpected)
                        {
                            dataPtr += VK_UUID_SIZE / sizeof(i32);
                        }
                        else
                        {
                            LogWarn("Bad validation cache file, version: %d, expected: %d", cacheVersion, cacheVersionExpected);
                            data.clear();
                        }
                    }
                    else
                    {
                        LogWarn("Bad validation cache file, header size: %d", *dataPtr);
                        data.clear();
                    }
                }
            }
            VkValidationCacheCreateInfoEXT validationCreateInfo;
            VkUtils::zeroStruct(validationCreateInfo, VK_STRUCTURE_TYPE_VALIDATION_CACHE_CREATE_INFO_EXT);
            validationCreateInfo.initialDataSize = data.count();
            validationCreateInfo.pInitialData = data.count() > 0 ? data.get() : nullptr;
            const VkResult result = vkCreateValidationCacheEXT(device, &validationCreateInfo, nullptr, &validationCache);
            LOG_VK_RESULT(result);
        }
#endif

        m_state = DeviceState::kReady;
        return GfxDevice::init();
    }

    void VulkanDevice::beginFrame()
    {
        // Base
        GfxDevice::beginFrame();

        // Flush resources
        deferredDeletionQueue.releaseResources();
        stagingManager.processPendingFree();
        descriptorPoolsManager->collect();
    }

    void VulkanDevice::dispose()
    {
        GfxDeviceLock lock(this);

        // Check if has been disposed already
        if (m_state == DeviceState::kDisposing || m_state == DeviceState::kDisposed)
            return;

        // Wait for rendering end
        waitForGPU();

        // Pre dispose
        preDispose();

        // Clear stuff
        m_framebuffers.clearDelete();
        m_renderPasses.clearDelete();
        m_layouts.clearDelete();
        helperResources.dispose();
        stagingManager.dispose();
        timestampQueryPools.clearDelete();
        GFX_DEL_RESOURCE(uniformBufferUploader);
        TkMemory::deletePtr(descriptorPoolsManager);
        TK_SAFE_DELETE(mainContext);
        if (transferQueue != graphicsQueue && computeQueue != transferQueue)
            TK_SAFE_DELETE(transferQueue);
        if (computeQueue != graphicsQueue)
            TK_SAFE_DELETE(computeQueue);
        TK_SAFE_DELETE(graphicsQueue);
        presentQueue = nullptr;
        fenceManager.dispose();
        deferredDeletionQueue.releaseResources(true);
        vmaDestroyAllocator(allocator);
        allocator = VK_NULL_HANDLE;
        if (pipelineCache != VK_NULL_HANDLE)
        {
            if (!savePipelineCache())
                LogWarn("Failed to save Vulkan pipeline cache");
            vkDestroyPipelineCache(device, pipelineCache, nullptr);
            pipelineCache = VK_NULL_HANDLE;
        }
#if VK_USE_VALIDATION_CACHE
        if (validationCache != VK_NULL_HANDLE)
        {
            if (!saveValidationCache())
                LogWarn("Failed to save Vulkan validation cache");
            vkDestroyValidationCacheEXT(device, validationCache, nullptr);
            validationCache = VK_NULL_HANDLE;
        }
#endif

        // Destroy device
        vkDestroyDevice(device, nullptr);
        device = VK_NULL_HANDLE;
        TK_SAFE_DELETE(adapter);

        // Shutdown Vulkan
#if VK_USE_DEBUG_LAYER
        removeDebugLayerCallback();
#endif
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;

        // Set current state
        m_state = DeviceState::kDisposed;
    }

    void VulkanDevice::waitForGPU()
    {
        if (device != VK_NULL_HANDLE)
        {
            PROFILE();
            VALIDATE_VK_RESULT(vkDeviceWaitIdle(device));
        }
    }

    GfxTexture *VulkanDevice::createTexture(const TkStringView &name)
    {
        return TkMemory::newObject<VulkanTexture>(TkMemory::getDefault(), this, name);
    }

    GfxShader *VulkanDevice::createShader(const TkStringView &name)
    {
        return TkMemory::newObject<VulkanShader>(TkMemory::getDefault(), this, name);
    }

    GfxPipelineState *VulkanDevice::createPipelineState()
    {
        return TkMemory::newObject<VulkanPipelineState>(TkMemory::getDefault(), this);
    }

    GfxTimerQuery *VulkanDevice::createTimerQuery()
    {
        return TkMemory::newObject<VulkanTimerQuery>(TkMemory::getDefault(), this);
    }

    GfxBuffer *VulkanDevice::createBuffer(const TkStringView &name)
    {
        return TkMemory::newObject<VulkanBuffer>(TkMemory::getDefault(), this, name);
    }

    GfxSampler *VulkanDevice::createSampler()
    {
        return TkMemory::newObject<VulkanSampler>(TkMemory::getDefault(), this);
    }

    GfxVertexLayout *VulkanDevice::createVertexLayout(const VertexElements &elements, bool explicitOffsets)
    {
        return TkMemory::newObject<VulkanVertexLayout>(TkMemory::getDefault(), this, elements, explicitOffsets);
    }

    GfxSwapChain *VulkanDevice::createSwapChain(GfxWindow *window)
    {
        return TkMemory::newObject<VulkanSwapChain>(TkMemory::getDefault(), this, window);
    }

    GfxConstantBuffer *VulkanDevice::createConstantBuffer(u32 size, const TkStringView &name)
    {
        return TkMemory::newObject<VulkanConstantBuffer>(TkMemory::getDefault(), this, size);
    }

    VulkanSemaphore::VulkanSemaphore(VulkanDevice *device)
        : m_device(device)
    {
        VkSemaphoreCreateInfo info;
        VkUtils::zeroStruct(info, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
        VALIDATE_VK_RESULT(vkCreateSemaphore(device->device, &info, nullptr, &m_semaphoreHandle));
    }

    VulkanSemaphore::~VulkanSemaphore()
    {
        TK_ASSERT(m_semaphoreHandle != VK_NULL_HANDLE);
        m_device->deferredDeletionQueue.enqueueResource(VulkanDeferredDeletionQueue::kSemaphore, m_semaphoreHandle);
        m_semaphoreHandle = VK_NULL_HANDLE;
    }

    VulkanFenceManager::~VulkanFenceManager()
    {
        TK_ASSERT(m_usedFences.isEmpty());
    }

    void VulkanFenceManager::dispose()
    {
        TkMutexGuard lock(m_device->m_fenceLock);
        TK_ASSERT(m_usedFences.isEmpty());
        for (VulkanFence *fence : m_freeFences)
            destroyFence(fence);
        m_freeFences.clear();
    }

    VulkanFence *VulkanFenceManager::allocateFence(bool createSignaled)
    {
        TkMutexGuard lock(m_device->m_fenceLock);
        VulkanFence *fence;
        if (m_freeFences.hasItems())
        {
            fence = m_freeFences.last();
            m_freeFences.removeLast();
            m_usedFences.add(fence);
            if (createSignaled)
                fence->isSignaled = true;
        }
        else
        {
            fence = TkMemory::newObject<VulkanFence>();
            fence->isSignaled = createSignaled;
            VkFenceCreateInfo info;
            VkUtils::zeroStruct(info, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
            info.flags = createSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
            VALIDATE_VK_RESULT(vkCreateFence(m_device->device, &info, nullptr, &fence->handle));
            m_usedFences.add(fence);
        }
        return fence;
    }

    bool VulkanFenceManager::waitForFence(VulkanFence *fence, u64 timeInNanoseconds) const
    {
        TK_ASSERT(m_usedFences.contains(fence));
        TK_ASSERT(!fence->isSignaled);
        const VkResult result = vkWaitForFences(m_device->device, 1, &fence->handle, true, timeInNanoseconds);
        LOG_VK_RESULT(result);
        if (result == VK_SUCCESS)
        {
            fence->isSignaled = true;
            return true;
        }
        return false;
    }

    void VulkanFenceManager::resetFence(VulkanFence *fence) const
    {
        if (fence->isSignaled)
        {
            VALIDATE_VK_RESULT(vkResetFences(m_device->device, 1, &fence->handle));
            fence->isSignaled = false;
        }
    }

    void VulkanFenceManager::releaseFence(VulkanFence *&fence)
    {
        TkMutexGuard lock(m_device->m_fenceLock);
        resetFence(fence);
        m_usedFences.remove(fence);
        m_freeFences.add(fence);
        fence = nullptr;
    }

    void VulkanFenceManager::waitAndReleaseFence(VulkanFence *&fence, u64 timeInNanoseconds)
    {
        TkMutexGuard lock(m_device->m_fenceLock);
        if (!fence->isSignaled)
            waitForFence(fence, timeInNanoseconds);
        resetFence(fence);
        m_usedFences.remove(fence);
        m_freeFences.add(fence);
        fence = nullptr;
    }

    bool VulkanFenceManager::checkFenceState(VulkanFence *fence) const
    {
        TK_ASSERT(m_usedFences.contains(fence));
        TK_ASSERT(!fence->isSignaled);
        const VkResult result = vkGetFenceStatus(m_device->device, fence->handle);
        if (result == VK_SUCCESS)
        {
            fence->isSignaled = true;
            return true;
        }
        return false;
    }

    void VulkanFenceManager::destroyFence(VulkanFence *fence) const
    {
        vkDestroyFence(m_device->device, fence->handle, nullptr);
        fence->handle = VK_NULL_HANDLE;
        TkMemory::deletePtr(fence);
    }

    GfxDevice *createDeviceVulkan()
    {
        return VulkanDevice::create();
    }

// TODO: expose it as a command line or engine parameter to end-user
#if GFX_ENABLE_DIAGNOSTICS
    // Default lowered from ErrorsAndWarningsPerf to ErrorsAndWarnings: the perf
    // category from KHRONOS validation is very chatty and inflates the layer's
    // working set noticeably. Errors/warnings are enough for normal dev work;
    // bump this to ErrorsAndWarningsPerf or higher when actively profiling.
    VulkanValidationLevel ValidationLevel = VulkanValidationLevel::ErrorsAndWarnings;
#else
    VulkanValidationLevel ValidationLevel = VulkanValidationLevel::Disabled;
#endif

#if VK_USE_DEBUG_LAYER

// TODO: expose it as a command line or engine parameter to end-user
#define VK_USE_KHRONOS_STANDARD_VALIDATION 1 // uses VK_LAYER_KHRONOS_validation
#define VK_USE_LUNARG_STANDARD_VALIDATION 1  // uses VK_LAYER_LUNARG_standard_validation

    static const char *GValidationLayers[] =
        {
            "VK_LAYER_GOOGLE_threading",
            "VK_LAYER_LUNARG_parameter_validation",
            "VK_LAYER_LUNARG_object_tracker",
            "VK_LAYER_LUNARG_core_validation",
            nullptr};

#endif

    static const char *GInstanceExtensions[] =
        {
#if (TK_MACOS || TK_IOS) && defined(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)
            VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
#endif
#if defined(VK_KHR_display) && 0
            VK_KHR_DISPLAY_EXTENSION_NAME,
#endif
            nullptr};

    static const char *GDeviceExtensions[] =
        {
#if (TK_MACOS || TK_IOS) && defined(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)
            VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
#endif
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#if VK_KHR_maintenance1
            VK_KHR_MAINTENANCE1_EXTENSION_NAME,
#endif
#if VK_USE_VALIDATION_CACHE
            VK_EXT_VALIDATION_CACHE_EXTENSION_NAME,
#endif
#if VK_KHR_sampler_mirror_clamp_to_edge
            VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME,
#endif
            nullptr};

    struct LayerExtension
    {
        VkLayerProperties layer;
        TkArray<VkExtensionProperties> extensions;

        LayerExtension()
        {
            TkMemory::memoryClear(&layer, sizeof(layer));
        }

        void getExtensions(TkArray<TkString> &result)
        {
            for (auto &e : extensions)
            {
                result.addUnique(e.extensionName);
            }
        }

        void getExtensions(TkArray<const char *> &result)
        {
            for (auto &e : extensions)
            {
                result.addUnique(e.extensionName);
            }
        }
    };

    static void enumerateInstanceExtensionProperties(const char *layerName, LayerExtension &outLayer)
    {
        VkResult result;
        do
        {
            u32 count = 0;
            result = vkEnumerateInstanceExtensionProperties(layerName, &count, nullptr);
            TK_ASSERT(result >= VK_SUCCESS);

            if (count > 0)
            {
                outLayer.extensions.clear();
                outLayer.extensions.addDefault(count);
                result = vkEnumerateInstanceExtensionProperties(layerName, &count, outLayer.extensions.get());
                TK_ASSERT(result >= VK_SUCCESS);
            }
        } while (result == VK_INCOMPLETE);
    }

    static void enumerateDeviceExtensionProperties(VkPhysicalDevice device, const char *layerName, LayerExtension &outLayer)
    {
        VkResult result;
        do
        {
            u32 count = 0;
            result = vkEnumerateDeviceExtensionProperties(device, layerName, &count, nullptr);
            TK_ASSERT(result >= VK_SUCCESS);

            if (count > 0)
            {
                outLayer.extensions.clear();
                outLayer.extensions.addDefault(count);
                result = vkEnumerateDeviceExtensionProperties(device, layerName, &count, outLayer.extensions.get());
                TK_ASSERT(result >= VK_SUCCESS);
            }
        } while (result == VK_INCOMPLETE);
    }

    static void trimDuplicates(TkArray<const char *> &array)
    {
        for (i32 i = array.count() - 1; i >= 0; i--)
        {
            bool found = false;
            for (i32 j = i - 1; j >= 0; j--)
            {
                if (!TkStringUtils::compareStrings(array[i], array[j]))
                {
                    found = true;
                    break;
                }
            }
            if (found)
            {
                array.removeAt(i);
            }
        }
    }

    static int findLayerIndex(const TkArray<LayerExtension> &list, const char *layerName)
    {
        for (i32 i = 1; i < list.count(); i++)
        {
            if (!TkStringUtils::compareStrings(list[i].layer.layerName, layerName))
            {
                return i;
            }
        }
        return -1;
    }

    static bool containsLayer(const TkArray<LayerExtension> &list, const char *layerName)
    {
        return findLayerIndex(list, layerName) != -1;
    }

    static bool findLayerExtension(const TkArray<LayerExtension> &list, const char *extensionName, const char *&foundLayer)
    {
        for (i32 extIndex = 0; extIndex < list.count(); extIndex++)
        {
            for (i32 i = 0; i < list[extIndex].extensions.count(); i++)
            {
                if (!TkStringUtils::compareStrings(list[extIndex].extensions[i].extensionName, extensionName))
                {
                    foundLayer = list[extIndex].layer.layerName;
                    return true;
                }
            }
        }
        return false;
    }

    static bool findLayerExtension(const TkArray<LayerExtension> &list, const char *extensionName)
    {
        const char *dummy = nullptr;
        return findLayerExtension(list, extensionName, dummy);
    }

    static bool listContains(const TkArray<const char *> &list, const char *name)
    {
        for (const char *element : list)
        {
            if (!TkStringUtils::compareStrings(element, name))
                return true;
        }
        return false;
    }

    static bool listContains(const TkArray<TkString> &list, const char *name)
    {
        for (const TkString &element : list)
        {
            if (element == name)
                return true;
        }
        return false;
    }

    void VulkanDevice::getInstanceLayersAndExtensions(TkArray<const char *> &outInstanceExtensions, TkArray<const char *> &outInstanceLayers, bool &outDebugUtils)
    {
        VkResult result;
        outDebugUtils = false;

        TkArray<LayerExtension> globalLayerExtensions;
        globalLayerExtensions.addDefault(1);
        enumerateInstanceExtensionProperties(nullptr, globalLayerExtensions[0]);

        TkArray<TkString> foundUniqueExtensions;
        TkArray<TkString> foundUniqueLayers;
        for (i32 i = 0; i < globalLayerExtensions[0].extensions.count(); i++)
        {
            foundUniqueExtensions.addUnique(TkString(globalLayerExtensions[0].extensions[i].extensionName));
        }

        {
            TkArray<VkLayerProperties> globalLayerProperties;
            do
            {
                u32 instanceLayerCount = 0;
                result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
                TK_ASSERT(result >= VK_SUCCESS);

                if (instanceLayerCount > 0)
                {
                    globalLayerProperties.addZeroed(instanceLayerCount);
                    result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, &globalLayerProperties[globalLayerProperties.count() - instanceLayerCount]);
                    TK_ASSERT(result >= VK_SUCCESS);
                }
            } while (result == VK_INCOMPLETE);

            globalLayerExtensions.addDefault(globalLayerProperties.count());
            for (i32 i = 0; i < globalLayerProperties.count(); i++)
            {
                LayerExtension *layer = &globalLayerExtensions[i + 1];
                auto &prop = globalLayerProperties[i];
                layer->layer = prop;
                enumerateInstanceExtensionProperties(prop.layerName, *layer);
                layer->getExtensions(foundUniqueExtensions);
                foundUniqueLayers.addUnique(TkString(prop.layerName));
            }
        }

#if VK_DEBUG
        if (foundUniqueLayers.hasItems())
        {
            LogInfo("Found instance layers:");
            TkSorting::quickSort(foundUniqueLayers);
            for (const TkString &name : foundUniqueLayers)
            {
                LogInfo("- %s", *name);
            }
        }

        if (foundUniqueExtensions.hasItems())
        {
            LogInfo("Found instance extensions:");
            TkSorting::quickSort(foundUniqueExtensions);
            for (const TkString &name : foundUniqueExtensions)
            {
                LogInfo("- %s", *name);
            }
        }
#endif

        // TODO: expose as a command line parameter or sth
        const bool useVkTrace = false;
        bool vkTrace = false;
        if (useVkTrace)
        {
            const char *VkTraceName = "VK_LAYER_LUNARG_vktrace";
            if (containsLayer(globalLayerExtensions, VkTraceName))
            {
                outInstanceLayers.add(VkTraceName);
                vkTrace = true;
            }
        }

#if VK_USE_DEBUG_LAYER
#if VK_ENABLE_API_DUMP
        if (!vkTrace)
        {
            const char *VkApiDumpName = "VK_LAYER_LUNARG_api_dump";
            if (containsLayer(globalLayerExtensions, VkApiDumpName))
            {
                outInstanceLayers.add(VkApiDumpName);
            }
            else
            {
                LogWarn("Unable to find Vulkan instance layer %s", VkApiDumpName);
            }
        }
#endif

        if (!vkTrace && ValidationLevel != VulkanValidationLevel::Disabled)
        {
            bool hasKhronosStandardValidationLayer = false, hasLunargStandardValidationLayer = false;
#if VK_USE_KHRONOS_STANDARD_VALIDATION
            const char *vkLayerKhronosValidation = "VK_LAYER_KHRONOS_validation";
            hasKhronosStandardValidationLayer = containsLayer(globalLayerExtensions, vkLayerKhronosValidation);
            if (hasKhronosStandardValidationLayer)
            {
                outInstanceLayers.add(vkLayerKhronosValidation);
            }
            else
            {
                LogWarn("Unable to find Vulkan instance validation layer %s", vkLayerKhronosValidation);
            }
#endif
#if VK_USE_LUNARG_STANDARD_VALIDATION
            if (!hasKhronosStandardValidationLayer)
            {
                const char *vkLayerLunargStandardValidation = "VK_LAYER_LUNARG_standard_validation";
                hasLunargStandardValidationLayer = containsLayer(globalLayerExtensions, vkLayerLunargStandardValidation);
                if (hasLunargStandardValidationLayer)
                {
                    outInstanceLayers.add(vkLayerLunargStandardValidation);
                }
                else
                {
                    LogWarn("Unable to find Vulkan instance validation layer %s", vkLayerLunargStandardValidation);
                }
            }
#endif
            if (!hasKhronosStandardValidationLayer && !hasLunargStandardValidationLayer)
            {
                for (u32 i = 0; GValidationLayers[i] != nullptr; i++)
                {
                    const char *validationLayer = GValidationLayers[i];
                    const bool validationFound = containsLayer(globalLayerExtensions, validationLayer);
                    if (validationFound)
                    {
                        outInstanceLayers.add(validationLayer);
                    }
                    else
                    {
                        LogWarn("Unable to find Vulkan instance validation layer %s", validationLayer);
                    }
                }
            }
        }

#if VK_EXT_debug_utils
        if (!vkTrace && ValidationLevel != VulkanValidationLevel::Disabled)
        {
            const char *foundDebugUtilsLayer = nullptr;
            outDebugUtils = findLayerExtension(globalLayerExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME, foundDebugUtilsLayer);
            if (outDebugUtils && *foundDebugUtilsLayer)
            {
                outInstanceLayers.add(foundDebugUtilsLayer);
            }
        }
#endif
#endif

        TkArray<const char *> platformExtensions;
        VulkanPlatform::getInstanceExtensions(platformExtensions, outInstanceLayers);

        for (const char *extension : platformExtensions)
        {
            if (findLayerExtension(globalLayerExtensions, extension))
            {
                outInstanceExtensions.add(extension);
            }
        }

        for (i32 i = 0; GInstanceExtensions[i] != nullptr; i++)
        {
            if (findLayerExtension(globalLayerExtensions, GInstanceExtensions[i]))
            {
                outInstanceExtensions.add(GInstanceExtensions[i]);
            }
        }

#if VK_EXT_debug_utils
        if (!vkTrace && findLayerExtension(globalLayerExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
        {
            outInstanceExtensions.add(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
#endif
        if (!vkTrace && ValidationLevel == VulkanValidationLevel::Disabled)
        {
            if (findLayerExtension(globalLayerExtensions, VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
            {
                outInstanceExtensions.add(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
            }
        }

        trimDuplicates(outInstanceLayers);
        if (outInstanceLayers.hasItems())
        {
            LogInfo("Using instance layers:");
            for (const char *layer : outInstanceLayers)
            {
                LogInfo("- %s", layer);
            }
        }
        else
        {
            LogInfo("Not using instance layers");
        }

        trimDuplicates(outInstanceExtensions);
        if (outInstanceExtensions.hasItems())
        {
            LogInfo("Using instance extensions:");
            for (const char *extension : outInstanceExtensions)
            {
                LogInfo("- %s", extension);
            }
        }
        else
        {
            LogInfo("Not using instance extensions");
        }
    }

    void VulkanDevice::getDeviceExtensionsAndLayers(VkPhysicalDevice gpu, TkArray<const char *> &outDeviceExtensions, TkArray<const char *> &outDeviceLayers)
    {
        TkArray<LayerExtension> deviceLayerExtensions;
        deviceLayerExtensions.addDefault(1);
        {
            u32 count = 0;
            TkArray<VkLayerProperties> properties;
            VALIDATE_VK_RESULT(vkEnumerateDeviceLayerProperties(gpu, &count, nullptr));
            properties.addZeroed(count);
            VALIDATE_VK_RESULT(vkEnumerateDeviceLayerProperties(gpu, &count, properties.get()));
            TK_ASSERT(count == properties.count());
            for (const VkLayerProperties &property : properties)
            {
                deviceLayerExtensions.addDefault(1);
                deviceLayerExtensions.last().layer = property;
            }
        }

        TkArray<TkString> foundUniqueLayers;
        TkArray<TkString> foundUniqueExtensions;

        for (i32 i = 0; i < deviceLayerExtensions.count(); i++)
        {
            if (i == 0)
            {
                enumerateDeviceExtensionProperties(gpu, nullptr, deviceLayerExtensions[i]);
            }
            else
            {
                foundUniqueLayers.addUnique(TkString(deviceLayerExtensions[i].layer.layerName));
                enumerateDeviceExtensionProperties(gpu, deviceLayerExtensions[i].layer.layerName, deviceLayerExtensions[i]);
            }

            deviceLayerExtensions[i].getExtensions(foundUniqueExtensions);
        }

#if VK_DEBUG
        if (foundUniqueLayers.hasItems())
        {
            LogInfo("Found device layers:");
            TkSorting::quickSort(foundUniqueLayers);
            for (const TkString &name : foundUniqueLayers)
            {
                LogInfo("- %s", *name);
            }
        }

        if (foundUniqueExtensions.hasItems())
        {
            LogInfo("Found device extensions:");
            TkSorting::quickSort(foundUniqueExtensions);
            for (const TkString &name : foundUniqueExtensions)
            {
                LogInfo("- %s", *name);
            }
        }
#endif

        // Add device layers for debugging
#ifdef VK_EXT_tooling_info
        if (listContains(foundUniqueExtensions, "VK_EXT_tooling_info"))
        {
            isDebuging = true;
        }
#endif
#if VK_USE_DEBUG_LAYER
        bool hasKhronosStandardValidationLayer = false, hasLunargStandardValidationLayer = false;
#if VK_USE_KHRONOS_STANDARD_VALIDATION
        const char *vkLayerKhronosValidation = "VK_LAYER_KHRONOS_validation";
        hasKhronosStandardValidationLayer = containsLayer(deviceLayerExtensions, vkLayerKhronosValidation);
        if (hasKhronosStandardValidationLayer)
        {
            outDeviceLayers.add(vkLayerKhronosValidation);
        }
#endif
#if VK_USE_LUNARG_STANDARD_VALIDATION
        if (!hasKhronosStandardValidationLayer)
        {
            const char *vkLayerLunargStandardValidation = "VK_LAYER_LUNARG_standard_validation";
            hasLunargStandardValidationLayer = containsLayer(deviceLayerExtensions, vkLayerLunargStandardValidation);
            if (hasLunargStandardValidationLayer)
            {
                outDeviceLayers.add(vkLayerLunargStandardValidation);
            }
        }
#endif
        if (!hasKhronosStandardValidationLayer && !hasLunargStandardValidationLayer)
        {
            for (u32 i = 0; GValidationLayers[i] != nullptr; i++)
            {
                const char *validationLayer = GValidationLayers[i];
                if (containsLayer(deviceLayerExtensions, validationLayer))
                {
                    outDeviceLayers.add(validationLayer);
                }
            }
        }
#endif

        // Find all extensions
        TkArray<const char *> availableExtensions;
        {
            for (i32 i = 0; i < deviceLayerExtensions[0].extensions.count(); i++)
            {
                availableExtensions.add(deviceLayerExtensions[0].extensions[i].extensionName);
            }

            for (i32 layerIndex = 0; layerIndex < outDeviceLayers.count(); layerIndex++)
            {
                i32 findLayerIndex;
                for (findLayerIndex = 1; findLayerIndex < deviceLayerExtensions.count(); findLayerIndex++)
                {
                    if (!TkStringUtils::compareStrings(deviceLayerExtensions[findLayerIndex].layer.layerName, outDeviceLayers[layerIndex]))
                    {
                        break;
                    }
                }

                if (findLayerIndex < deviceLayerExtensions.count())
                {
                    deviceLayerExtensions[findLayerIndex].getExtensions(availableExtensions);
                }
            }
        }
        trimDuplicates(availableExtensions);

        // Pick extensions to use
        TkArray<const char *> platformExtensions;
        VulkanPlatform::getDeviceExtensions(platformExtensions, outDeviceLayers);
        for (const char *extension : platformExtensions)
        {
            if (listContains(availableExtensions, extension))
            {
                outDeviceExtensions.add(extension);
                break;
            }
        }
        for (u32 i = 0; i < lengthOf(GDeviceExtensions) && GDeviceExtensions[i] != nullptr; i++)
        {
            if (listContains(availableExtensions, GDeviceExtensions[i]))
            {
                outDeviceExtensions.add(GDeviceExtensions[i]);
            }
        }

        if (outDeviceExtensions.hasItems())
        {
            LogInfo("Using device extensions:");
            for (const char *extension : outDeviceExtensions)
            {
                LogInfo("- %s", extension);
            }
        }

        if (outDeviceLayers.hasItems())
        {
            LogInfo("Using device layers:");
            for (const char *layer : outDeviceLayers)
            {
                LogInfo("- %s", layer);
            }
        }
    }

    void VulkanDevice::parseOptionalDeviceExtensions(const TkArray<const char *> &deviceExtensions)
    {
        TkMemory::memoryClear(&optionalDeviceExtensions, sizeof(optionalDeviceExtensions));
#if VK_KHR_maintenance1
        optionalDeviceExtensions.hasKHRMaintenance1 = VkUtils::hasExtension(deviceExtensions, VK_KHR_MAINTENANCE1_EXTENSION_NAME);
#endif
#if VK_KHR_maintenance2
        optionalDeviceExtensions.hasKHRMaintenance2 = VkUtils::hasExtension(deviceExtensions, VK_KHR_MAINTENANCE2_EXTENSION_NAME);
#endif
        optionalDeviceExtensions.hasMirrorClampToEdge = VkUtils::hasExtension(deviceExtensions, VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
#if VK_USE_VALIDATION_CACHE
        optionalDeviceExtensions.hasEXTValidationCache = VkUtils::hasExtension(deviceExtensions, VK_EXT_VALIDATION_CACHE_EXTENSION_NAME);
#endif
    }

}

#endif
