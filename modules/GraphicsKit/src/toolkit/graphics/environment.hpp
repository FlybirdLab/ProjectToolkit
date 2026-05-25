#pragma once

#define TK_GRAPHICS_BACKEND_NONE 0
#define TK_GRAPHICS_BACKEND_NULL 0
#define TK_GRAPHICS_BACKEND_DIRECTX11 1
#define TK_GRAPHICS_BACKEND_DIRECTX12 1
#define TK_GRAPHICS_BACKEND_VULKAN 1
#define TK_GRAPHICS_BACKEND_OPENGL 0
#define TK_GRAPHICS_BACKEND_OPENGLES 0
#define TK_GRAPHICS_BACKEND_METAL 0

#define GFX_API_NULL TK_GRAPHICS_BACKEND_NULL
#define GFX_API_DIRECTX11 TK_GRAPHICS_BACKEND_DIRECTX11
#define GFX_API_DIRECTX12 TK_GRAPHICS_BACKEND_DIRECTX12
#define GFX_API_VULKAN TK_GRAPHICS_BACKEND_VULKAN
#define GFX_API_OPENGL TK_GRAPHICS_BACKEND_OPENGL
#define GFX_API_OPENGLES TK_GRAPHICS_BACKEND_OPENGLES
#define GFX_API_METAL TK_GRAPHICS_BACKEND_METAL

// Maximum allowed amount of shader program permutations
#define SHADER_PERMUTATIONS_MAX_COUNT 16
// Maximum allowed amount of parameters for permutation
#define SHADER_PERMUTATIONS_MAX_PARAMS_COUNT 4

// Maximum amount of binded render targets at the same time
#define GFX_MAX_RT_BINDED 6

// Maximum amount of binded shader resources at the same time
#define GFX_MAX_SR_BINDED 32

// Maximum amount of binded constant buffers at the same time
#define GFX_MAX_CB_BINDED 4

// Maximum amount of binded unordered access resources at the same time
#define GFX_MAX_UA_BINDED 4

// Maximum amount of binded texture sampler resources at the same time
#define GFX_MAX_SAMPLER_BINDED 16

// Amount of initial slots used for global samplers (static, 4 common samplers + 2 comparision samplers)
#define GFX_STATIC_SAMPLERS_COUNT 6

// Maximum amount of binded vertex buffers at the same time
#define GFX_MAX_VB_BINDED 4

// Maximum amount of vertex shader input elements in a layout
#define GFX_MAX_VS_ELEMENTS 16

// Maximum amount of thread groups per dimension for compute dispatch
#define GFX_MAX_CS_DISPATCH_THREAD_GROUPS 65535

// Alignment of the shader data structures (16-byte boundaries) to improve memory copies efficiency.
#define GFX_SHADER_DATA_ALIGNMENT 16

// Enable/disable assertion for graphics layers
#define GFX_ENABLE_ASSERTION 1
#define GFX_ENABLE_ASSERTION_LOW_LAYERS (TK_DEBUG)

// Enable/disable dynamic textures quality streaming
#define GFX_ENABLE_TEXTURES_STREAMING 1

// Enable/disable creating Shader Resource View for window backbuffer surface
#ifndef GFX_USE_WINDOW_SRV
#define GFX_USE_WINDOW_SRV 1
#endif

// True if allow graphics profile events and markers
#ifndef GFX_ALLOW_PROFILE_EVENTS
#define GFX_ALLOW_PROFILE_EVENTS (TK_DEBUG)
#endif

#ifndef GFX_ALLOW_TESSELLATION_SHADERS
#define GFX_ALLOW_TESSELLATION_SHADERS 1
#endif

#ifndef GFX_ALLOW_GEOMETRY_SHADERS
#define GFX_ALLOW_GEOMETRY_SHADERS 1
#endif

// Enable/disable creating GPU resources on separate threads (otherwise only the main thread can be used)
#define GFX_ENABLE_ASYNC_RESOURCES_CREATION 1

// Enable/disable force shaders recompilation
#define GFX_FORCE_RECOMPILE_SHADERS 0

// Define default back buffer(s) format
#ifndef GFX_BACK_BUFFER_PIXEL_FORMAT
#define GFX_BACK_BUFFER_PIXEL_FORMAT PixelFormat::kB8G8R8A8_UNorm
#endif

// Default depth buffer pixel format
#ifndef GFX_DEPTH_BUFFER_PIXEL_FORMAT
#define GFX_DEPTH_BUFFER_PIXEL_FORMAT PixelFormat::kD32_Float
#endif

// Enable/disable gpu resources naming
#ifndef GFX_ENABLE_RESOURCE_NAMING
#define GFX_ENABLE_RESOURCE_NAMING (TK_DEBUG)
#endif

// True if use debug tools and flow for shaders
#ifndef GFX_USE_SHADERS_DEBUG_LAYER
#define GFX_USE_SHADERS_DEBUG_LAYER (TK_DEBUG)
#endif

// Maximum size of the texture that is supported by the RHI.
#define GFX_MAX_TEXTURE_SIZE 16384
#define GFX_MAX_TEXTURE_MIP_LEVELS 15
#define GFX_MAX_TEXTURE_ARRAY_SIZE 1024

#if !TK_DEBUG
#undef GFX_ENABLE_ASSERTION
#define GFX_ENABLE_ASSERTION 0
#endif

#define GFX_CB_STRUCT(_declaration) TK_ALIGN_BEGIN(GFX_SHADER_DATA_ALIGNMENT) TK_PACK_BEGIN() struct _declaration PACK_END() TK_ALIGN_END(GFX_SHADER_DATA_ALIGNMENT)
