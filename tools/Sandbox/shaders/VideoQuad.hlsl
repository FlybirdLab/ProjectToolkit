// Sandbox video fullscreen quad shader.

struct VSOutput
{
    float4 Position : SV_Position;
    float2 UV       : TEXCOORD0;
};

VSOutput VSMain(uint vertexId : SV_VertexID)
{
    // Fullscreen triangle technique: 3 vertices, no vertex buffer needed.
    // The triangle covers more than the screen; the visible UV range is [0, 1].
    VSOutput output;
    output.UV       = float2((vertexId << 1) & 2, vertexId & 2);
    output.Position = float4(output.UV * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
    return output;
}

Texture2D    _Frame   : register(t0);
// s0-s5 are reserved by GraphicsKit for built-in static samplers.
SamplerState _Sampler : register(s6);

float4 PSMain(VSOutput input) : SV_Target0
{
    return _Frame.Sample(_Sampler, input.UV);
}
