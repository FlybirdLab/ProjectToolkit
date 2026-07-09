// Sandbox triangle, single-source HLSL.

struct VSOutput
{
    float4 Position : SV_Position;
    float4 Color    : COLOR0;
};

VSOutput VSMain(uint vertexId : SV_VertexID)
{
    static const float2 positions[3] =
    {
        float2( 0.00,  0.65),
        float2( 0.65, -0.55),
        float2(-0.65, -0.55)
    };

    static const float4 colors[3] =
    {
        float4(1.00, 0.12, 0.10, 1.0),
        float4(0.10, 0.80, 0.25, 1.0),
        float4(0.15, 0.35, 1.00, 1.0)
    };

    VSOutput output;
    output.Position = float4(positions[vertexId], 0.0, 1.0);
    output.Color = colors[vertexId];
    return output;
}

float4 PSMain(VSOutput input) : SV_Target0
{
    return input.Color;
}
