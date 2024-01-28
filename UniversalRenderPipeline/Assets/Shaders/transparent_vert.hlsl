// Copyright 2020 Google LLC

struct VSInput
{
[[vk::location(0)]] float3 Pos : POSITION0;
[[vk::location(1)]] float3 Normal : NORMAL0;
[[vk::location(2)]] float2 UV : TEXCOORD0;
[[vk::location(3)]] float3 Tangent : TEXCOORD1;
[[vk::location(4)]] float3 Color : COLOR0;
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
    float4 ambientLightColor;
    float4 viewPos;
    float4 camereInfo;
    float4 size;
    PointLight pointLights[10];
    int numLights;
};

cbuffer ubo : register(b0)
{
    GlobalUBO ubo;
}

struct VSOutput
{
    float4 Pos : SV_Position;
    [[vk::location(0)]] float3 WorldPos : POSITION0;
    [[vk::location(1)]] float3 Normal : NORMAL0;
    [[vk::location(2)]] float2 UV : TEXCOORD0;
    [[vk::location(3)]] float3 Tangent : TEXCOORD1;
    [[vk::location(4)]] float3 Color : COLOR0;
    [[vk::location(5)]] float4 ShadowCoords : TEXCOORD2;
};

struct PushConsts
{
    float4x4 modelMatrix;
    float4x4 normalMatrix;
};

[[vk::push_constant]] PushConsts pushConsts;

VSOutput main (VSInput input)
{
	VSOutput output = (VSOutput)0;
    float4 position = mul(pushConsts.modelMatrix, float4(input.Pos, 1.0));
    output.WorldPos = position.xyz;
    output.Pos = mul(ubo.projection, mul(ubo.view, position));
    float3x3 mat = (float3x3) pushConsts.modelMatrix;
    output.Normal = normalize(mul(mat, normalize(input.Normal)));
    output.UV = input.UV;
    output.Tangent = normalize(mul(mat, normalize(input.Tangent.xyz)));
    output.Color = input.Color;
    output.ShadowCoords = float4(1.0, 1.0, 1.0, 1.0f);
    
	return output;
}