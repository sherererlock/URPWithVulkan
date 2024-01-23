// Copyright 2020 Google LLC



struct VSOutput
{
    float4 Pos : SV_Position;
    [[vk::location(0)]] float3 WorldPos : POSITION0;
    [[vk::location(1)]] float3 Normal : NORMAL0;
    [[vk::location(2)]] float2 UV : TEXCOORD0;
    [[vk::location(3)]] float3 Tangent : TEXCOORD1;
    [[vk::location(4)]] float4 ShadowCoords : TEXCOORD2;
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

[[vk::input_attachment_index(0)]][[vk::binding(1)]] SubpassInput samplerPositionDepth;
Texture2D textureTexture : register(t2);
SamplerState samplerTexture : register(s2);

float linearDepth(float depth)
{
	float z = depth * 2.0f - 1.0f;
    return (2.0f * ubo.camereInfo.x * ubo.camereInfo.y) / (ubo.camereInfo.y + ubo.camereInfo.x - z * (ubo.camereInfo.y - ubo.camereInfo.x));
}

float4 main (VSOutput input) : SV_TARGET
{
	// Sample depth from deferred depth buffer and discard if obscured
	float depth = samplerPositionDepth.SubpassLoad().a;
	if ((depth != 0.0) && (linearDepth(input.Pos.z) > depth))
	{
		clip(-1);
	};

	return textureTexture.Sample(samplerTexture, input.UV);
}