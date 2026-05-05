struct VSOut
{
    float4 position : SV_Position;
    float4 color : COLOR0;
};

META_VS(true, FEATURE_LEVEL_SM5)
VSOut VSMain(uint id : SV_VertexID)
{
    const float2 positions[3] =
    {
        float2(0.0, 0.65),
        float2(0.65, -0.55),
        float2(-0.65, -0.55),
    };
    const float4 colors[3] =
    {
        float4(1.0, 0.12, 0.10, 1.0),
        float4(0.10, 0.80, 0.25, 1.0),
        float4(0.15, 0.35, 1.0, 1.0),
    };

    VSOut output;
    output.position = float4(positions[id], 0.0, 1.0);
    output.color = colors[id];
    return output;
}

META_PS(true, FEATURE_LEVEL_SM5)
float4 PSMain(VSOut input) : SV_Target0
{
    return input.color;
}
