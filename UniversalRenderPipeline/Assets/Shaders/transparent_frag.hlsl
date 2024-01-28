// Copyright 2020 Google LLC

#include "common.hlsl"

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

Texture2D textureTexture : register(t1);
SamplerState samplerTexture : register(s1);

[[vk::input_attachment_index(0)]][[vk::binding(2)]] SubpassInput samplerPositionDepth;

float4 main (VSOutput input) : SV_TARGET
{
	// Sample depth from deferred depth buffer and discard if obscured
	float depth = samplerPositionDepth.SubpassLoad().a;
    if ((depth != 0.0) && (linearDepth(input.Pos.z, ubo.camereInfo.x, ubo.camereInfo.y) > depth))
	{
        clip(-1);
    };
    
    return textureTexture.Sample(samplerTexture, input.UV);
}
