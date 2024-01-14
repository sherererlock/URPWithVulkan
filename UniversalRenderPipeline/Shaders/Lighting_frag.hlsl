struct VSOutput
{
    float4 Pos : SV_POSITION;
    [[vk::location(0)]] float2 UV : TEXCOORD0;
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

struct CameraExtentUBO
{
    float4 leftTop;
    float4 left2Right;
    float4 top2bottom;
};

cbuffer cameraubo : register(b1)
{
    CameraExtentUBO cameraubo;
}

struct ShadowUBO
{
    float4x4 lightVP;
    float4 shadowBias; // (normalbias, w, 0, 0)
};

cbuffer shadowUbo : register(b2)
{
    ShadowUBO shadowUbo;
}

Texture2D textureColor : register(t0, space1);
SamplerState samplerColor : register(s0, space1);

Texture2D textureNormal : register(t1, space1);
SamplerState samplerNormal : register(s1, space1);

Texture2D textureEmissive : register(t2, space1);
SamplerState samplerEmissive : register(s2, space1);

Texture2D textureDepth : register(t3, space1);
SamplerState samplerDepth : register(s3, space1);

Texture2D textureShadow : register(t4, space1);
SamplerState samplerShadow : register(s4, space1);

#include "shadow.hlsl"
#include "common.hlsl"
#include "Lighting.hlsl"

float4 main(VSOutput input) : SV_TARGET
{
    float2 uv = input.UV;

    float4 gbuffer1 = textureColor.Sample(samplerColor, uv);
    float4 gbuffer2 = textureNormal.Sample(samplerNormal, uv);
    float4 gbuffer3 = textureEmissive.Sample(samplerEmissive, uv);
    float depth = textureDepth.Sample(samplerDepth, uv).r;
    
    float3 albedo = pow(gbuffer1.rgb, float3(2.2f, 2.2f, 2.2f));
    float3 normal = normalize(gbuffer2.rgb * 2.0f - 1.0f);
    float3 emissive = gbuffer3.rgb;
    
    float roughness = gbuffer1.a;
    float metallic = gbuffer2.a;
    
    float3 F0 = lerp(float3(0.4f, 0.4f, 0.4f), albedo, metallic);
    
    float ldepth = linearDepth(depth, ubo.camereInfo.x, ubo.camereInfo.y);
    float3 worldPos = ReconstructWorldPos(uv, ldepth, cameraubo.leftTop.xyz, cameraubo.left2Right.xyz, cameraubo.top2bottom.xyz, ubo.camereInfo.z);
    
    float3 biasPos = worldPos + normal * shadowUbo.shadowBias.x;
    float4 shadowCoords = mul(shadowUbo.lightVP, float4(biasPos, 1.0f));
    float shadow = getShadow(shadowCoords, textureShadow, samplerShadow);
    
    float3 viewDir = normalize(ubo.viewPos.xyz - worldPos);
    float3 Lo = DirectLighting(normal, viewDir, albedo, F0, roughness, metallic, shadow, worldPos);
    
    Lo = pow(Lo, float3(0.45f, 0.45f, 0.45f));
    
    return float4(Lo + emissive, 1.0f);
}

/*

VertexIndex     UV          Pos
0               (0, 0)      (0, 0, 0, 1)
1               (1, 0)      (1, 0, 0, 1)
2               (0, 1)      (0, 1, 0, 1)

*/