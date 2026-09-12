// =========================================================================
// Uniform Buffer Definitions
// =========================================================================
struct CameraUBO
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
};

// Slang unified binding:
// - Vulkan target gets: layout(set=0, binding=0) uniform ...
// - OpenGL target gets: layout(binding=0) uniform ...
[[vk::binding(0, 0)]]
ConstantBuffer<CameraUBO> camera;


// =========================================================================
// Vertex Stage
// =========================================================================
struct VertexInput
{
    // Location attributes translate cleanly to both SPIR-V locations and GLSL layout locations
    [[vk::location(0)]] float3 position;
    [[vk::location(1)]] float4 color;
    [[vk::location(2)]] float2 uv;
    [[vk::location(3)]] float3 normal;
};

struct VertexOutput
{
    // System-value semantic for positions remains universal in Slang
    float4 position : SV_Position;

    [[vk::location(0)]] float4 color;
    [[vk::location(1)]] float2 uv;
    [[vk::location(2)]] float3 normal;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;

    // NOTE: Slang defaults to row-major matrices. 
    // If your C++ engine maps column-major (like GLM), pass 
    // '-matrix-layout-column-major' as a slangc flag in your CMake file.
    float4 worldPosition = mul(camera.model, float4(input.position, 1.0));
    float4 viewPosition  = mul(camera.view, worldPosition);
    output.position      = mul(camera.projection, viewPosition);

    output.color  = input.color;
    output.uv     = input.uv;
    output.normal = input.normal;

    return output;
}


// =========================================================================
// Fragment Stage
// =========================================================================
struct FragmentInput
{
    [[vk::location(0)]] float4 color;
    [[vk::location(1)]] float2 uv;
    [[vk::location(2)]] float3 normal;
};

[shader("fragment")]
float4 fragmentMain(FragmentInput input) : SV_Target
{
    // Simply pass the interpolated vertex color through
    return input.color;
}
