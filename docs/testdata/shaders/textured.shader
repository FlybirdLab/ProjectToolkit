META_CB_BEGIN(0, DrawGlobals)
cbuffer DrawGlobals : register(b0)
{
    float4x4 gWorldViewProj;
    float4 gColorScale;
};
META_CB_END

Texture2D gBaseColor : register(t0);
SamplerState gBaseSampler : register(s0);

struct VSInput
{
    float3 position : POSITION0;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

META_VS(true, FEATURE_LEVEL_SM4)
META_VS_IN_ELEMENT(POSITION, 0, R32G32B32_Float, 0, ALIGN, PER_VERTEX, 0, true)
META_VS_IN_ELEMENT(TEXCOORD, 0, R32G32_Float, 0, ALIGN, PER_VERTEX, 0, true)
META_VS_IN_ELEMENT(COLOR, 0, R32G32B32A32_Float, 0, ALIGN, PER_VERTEX, 0, true)
VSOutput VSMain(VSInput input)
{
    VSOutput output;
    output.position = mul(float4(input.position, 1.0f), gWorldViewProj);
    output.uv = input.uv;
    output.color = input.color;
    return output;
}

META_PS(true, FEATURE_LEVEL_SM4)
META_PERMUTATION_1(USE_VERTEX_COLOR=0)
META_PERMUTATION_1(USE_VERTEX_COLOR=1)
float4 PSMain(VSOutput input) : SV_Target0
{
    float4 color = gBaseColor.Sample(gBaseSampler, input.uv) * gColorScale;
#if USE_VERTEX_COLOR
    color *= input.color;
#endif
    return color;
}
