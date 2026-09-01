struct CameraUBO
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
};

[[vk::binding(0, 0)]]
ConstantBuffer<CameraUBO> camera;


struct VertexInput
{
    [[vk::location(0)]] float3 position;
    [[vk::location(1)]] float4 color;
    [[vk::location(2)]] float2 uv;
    [[vk::location(3)]] float3 normal;
};

struct VertexOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float4 color;
    [[vk::location(1)]] float2 uv;
    [[vk::location(2)]] float3 normal;
};


[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;

    float4 worldPosition =
        mul(camera.model, float4(input.position, 1.0));

    float4 viewPosition =
        mul(camera.view, worldPosition);

    output.position =
        mul(camera.projection, viewPosition);

    output.color = input.color;
    output.uv = input.uv;
    output.normal = input.normal;

    return output;
}


struct FragmentInput
{
    [[vk::location(0)]] float4 color;
    [[vk::location(1)]] float2 uv;
    [[vk::location(2)]] float3 normal;
};


[shader("fragment")]
float4 fragmentMain(FragmentInput input) : SV_Target
{
    return input.color;
}