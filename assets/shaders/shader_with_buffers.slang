struct VertexInput
{
    float3 inPosition;
    float3 inColor;
};

struct UniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};
ConstantBuffer<UniformBuffer> ubo;

struct VertexOutput {
    float3 color;
    float4 pos : SV_Position;
};

[shader("vertex")]
VertexOutput vertMain(VertexInput inVert) {
    VertexOutput output;
    output.pos = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(inVert.inPosition, 1.0))));
    output.color = inVert.inColor;
    return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target
{
    return float4(inVert.color, 1.0);
}