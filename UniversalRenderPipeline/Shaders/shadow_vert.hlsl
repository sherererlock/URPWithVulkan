
struct VSInput
{
    [[vk::location(0)]] float3 Pos : POSITION0;
};

struct GlobalUBO
{
    float4x4 lightVP;
};

cbuffer ubo : register(b0)
{
    GlobalUBO ubo;
}

struct PushConsts
{
    float4x4 modelMatrix;
};

[[vk::push_constant]] PushConsts pushConsts;

float4 main(VSInput input) : SV_POSITION
{
    float3 WorldPos = mul(pushConsts.modelMatrix, float4(input.Pos, 1.0)).xyz;
    float4 position = mul(ubo.lightVP, float4(WorldPos, 1.0f));
    
    return position;
}