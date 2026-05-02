META_CB_BEGIN(0, TestGlobals)
cbuffer TestGlobals : register(b0)
{
    float4x4 gWorldViewProj;
    float4 gTint;
};
META_CB_END

struct VSInput
{
    float3 position : POSITION0;
    float2 uv : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

float4 MakeColor(float2 uv)
{
    return float4(uv.x, uv.y, 1.0f - uv.x, 1.0f);
}

META_VS(true, FEATURE_LEVEL_SM4)
META_VS_IN_ELEMENT(POSITION, 0, R32G32B32_Float, 0, ALIGN, PER_VERTEX, 0, true)
META_VS_IN_ELEMENT(TEXCOORD, 0, R32G32_Float, 0, ALIGN, PER_VERTEX, 0, true)
META_PERMUTATION_1(USE_TINT=0)
META_PERMUTATION_1(USE_TINT=1)
VSOutput VSMain(VSInput input)
{
    VSOutput output;
    output.position = mul(float4(input.position, 1.0f), gWorldViewProj);
    output.uv = input.uv;
    output.color = MakeColor(input.uv);
    return output;
}

META_PS(true, FEATURE_LEVEL_SM4)
META_PERMUTATION_1(USE_TINT=0)
META_PERMUTATION_1(USE_TINT=1)
float4 PSMain(VSOutput input) : SV_Target0
{
#if USE_TINT
    return input.color * gTint;
#else
    return input.color;
#endif
}
