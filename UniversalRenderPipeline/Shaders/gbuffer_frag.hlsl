
struct VSOutput
{
    float4 Pos : SV_Position;
    [[vk::location(0)]] float3 WorldPos : POSITION0;
    [[vk::location(1)]] float3 Normal : NORMAL0;
    [[vk::location(2)]] float2 UV : TEXCOORD0;
    [[vk::location(3)]] float3 Tangent : TEXCOORD1;
    [[vk::location(4)]] float4 ShadowCoords : TEXCOORD2;
};

struct FSOutput
{
    float4 albedo : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 emmisive : SV_TARGET2;
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

struct PushConsts
{
    float4x4 modelMatrix;
    float4x4 normalMatrix;
};

[[vk::push_constant]] PushConsts pushConsts;

Texture2D textureColor : register(t0, space1);
SamplerState samplerColor : register(s0, space1);

Texture2D textureNormal : register(t1, space1);
SamplerState samplerNormal : register(s1, space1);

Texture2D textureRoughness : register(t2, space1);
SamplerState samplerRoughness : register(s2, space1);

Texture2D textureEmissive : register(t3, space1);
SamplerState samplerEmissive : register(s3, space1);

float3 calculateNormal(VSOutput input)
{
    float3 tangentNormal = textureNormal.Sample(samplerNormal, input.UV).rgb * 2.0f - float3(1.0f, 1.0f, 1.0f);
    
    float3 N = normalize(input.Normal);
    float3 T = normalize(input.Tangent);
    float3 B = normalize(cross(N, T));
    float3x3 TBN = transpose(float3x3(T, B, N));
    return normalize(mul(TBN, tangentNormal));
}

FSOutput main(VSOutput input) : SV_TARGET
{
    FSOutput output;
    
    float3 albedo = textureColor.Sample(samplerColor, input.UV).rgb;
    float3 emmisive = textureEmissive.Sample(samplerEmissive, input.UV).rgb;
    float3 normal = calculateNormal(input);
    float2 rm = textureRoughness.Sample(samplerRoughness, input.UV).gb;
    float roughness = rm.x;
    float metallic = rm.y;
    
    output.albedo = float4(albedo, roughness);
    output.normal = float4(normal * 0.5f + float3(0.5f, 0.5f, 0.5f), metallic);
    output.emmisive = float4(emmisive, 1.0f);

    return output;
}
