[Flags]
enum VertexFlags
{
    None,
    UseOnlyColor,
    UseOnlyTexture,
    RedAll
};

struct VertexInput
{
    float3 inPosition;
    float3 inColor;
    // float2 texCoord;
};

struct UniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};
ConstantBuffer<UniformBuffer> ubo;

struct PushConstant
{
    VertexFlags flags;
};
[push_constant]
PushConstant pc;

struct VertexOutput {
    float3 color;
    float4 pos : SV_Position;
    // float2 texCoord;
};
// Sampler2D texture;

[shader("vertex")]
VertexOutput vertMain(VertexInput inVert) {
    VertexOutput output;
    output.pos = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(inVert.inPosition, 1.0))));
    output.color = inVert.inColor;
    // output.texCoord = inVert.texCoord;
    return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target
{
    if (pc.flags & VertexFlags::UseOnlyColor)
    {
        return float4(inVert.color, 1.0);
    }
    else if (pc.flags & VertexFlags::RedAll)
    {
        return float4(1., 0., 0., 1.0);
    }
    else
    {
        return float4(1., 1., 1., 1.0);
    }
    // else if (pc.flags & VertexFlags::UseOnlyTexture)
    // {
    //     return texture.Sample(inVert.texCoord);
    // }
    // else
    // {
    //     return float4(inVert.color * texture.Sample(inVert.texCoord).rgb, 1.0);
    // }
}