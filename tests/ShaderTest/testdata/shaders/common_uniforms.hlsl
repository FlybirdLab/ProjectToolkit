// Constant buffer used by test_triangle.hlsl. Lives in a sibling file so the
// DependencyScanner has at least one #include edge to walk.

#ifndef TKTEST_COMMON_UNIFORMS_HLSL
#define TKTEST_COMMON_UNIFORMS_HLSL

cbuffer FrameCB : register(b0)
{
    float4 gTintColor;
};

#endif // TKTEST_COMMON_UNIFORMS_HLSL
