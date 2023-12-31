
struct VSInput
{
    [[vk::location(0)]] float3 Pos : POSITION0;
    [[vk::location(1)]] float3 Normal : NORMAL0;
    [[vk::location(2)]] float2 UV : TEXCOORD0;
    [[vk::location(3)]] float3 Tangent : TEXCOORD1;
};

struct VSOutput
{
    float4 Pos : SV_Position;
    [[vk::location(0)]] float3 WorldPos : POSITION0;
    [[vk::location(1)]] float3 Normal : NORMAL0;
    [[vk::location(2)]] float2 UV : TEXCOORD0;
    [[vk::location(3)]] float3 Tangent : TEXCOORD1;
};

struct PointLight
{
    float4 position; // ignore w
    float4 color; // w is intensity
};

struct GlobalUBO
{
    float4x4 projection;
    float4x4 view;
    float4x4 inView;
    float4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
};

cbuffer ubo : register(b0)
{
    GlobalUBO ubo;
}

struct PushConsts
{
    float4x4 modelMatrix;
    float4x4 normalMatrix;
};

[[vk::push_constant]] PushConsts pushConsts;

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    output.WorldPos = mul(pushConsts.modelMatrix, float4(input.Pos, 1.0)).xyz;
    output.Pos = mul(ubo.projection, mul(ubo.view, float4(output.WorldPos, 1.0)));
    output.Normal = mul((float3x3) pushConsts.modelMatrix, normalize(input.Normal));
    output.UV = input.UV;
    output.Tangent = mul((float3x3) pushConsts.modelMatrix, normalize(input.Tangent));
    
    return output;
}