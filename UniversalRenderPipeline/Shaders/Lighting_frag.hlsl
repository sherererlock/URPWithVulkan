
#include "macros.hlsl"

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

#ifndef CALC_POSITOIN
Texture2D texturePosition : register(t5, space1);
SamplerState samplerPosition : register(s5, space1);
#endif

#include "shadow.hlsl"
#include "common.hlsl"
#include "Lighting.hlsl"

float4 main(VSOutput input) : SV_TARGET
{
    float2 uv = input.UV;
    float4 gbuffer3 = textureEmissive.Sample(samplerEmissive, uv);
    if (gbuffer3.a != 1.0f)
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    float4 gbuffer1 = textureColor.Sample(samplerColor, uv);
    float4 gbuffer2 = textureNormal.Sample(samplerNormal, uv);
    float depth = textureDepth.Sample(samplerDepth, uv).r;
    
    float3 albedo = pow(gbuffer1.rgb, float3(2.2f, 2.2f, 2.2f));
    float3 normal = normalize(gbuffer2.xyz * 2.0f - float3(1.0f, 1.0f, 1.0f));
    //float3 normal = normalize(gbuffer2.xyz);
    float3 emissive = gbuffer3.rgb;
    
    float roughness = gbuffer1.a;
    float metallic = gbuffer2.a;
   
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metallic);
    
#ifndef CALC_POSITOIN
    float4 gbuffer4 = texturePosition.Sample(samplerPosition, uv);
    float3 worldPos = gbuffer4.xyz;
#else
    float ldepth = linearDepth(depth, ubo.camereInfo.x, ubo.camereInfo.y);
    float3 viewPos = ReconstructWorldPos(uv, ldepth, cameraubo.leftTop.xyz, cameraubo.left2Right.xyz, cameraubo.top2bottom.xyz, ubo.camereInfo.z);
    float3 worldPos = viewPos + ubo.viewPos.xyz;
#endif
    
    float ldepth = linearDepth(depth, ubo.camereInfo.x, ubo.camereInfo.y);
    float3 viewPos = ReconstructWorldPos(uv, ldepth, cameraubo.leftTop.xyz, cameraubo.left2Right.xyz, cameraubo.top2bottom.xyz, ubo.camereInfo.z);
    float3 worldPos1 = viewPos + ubo.viewPos.xyz;
    
    //return float4(worldPos1, 1.0f);
    
    float3 biasPos = worldPos + normal * shadowUbo.shadowBias.x;
    float4 shadowCoords = mul(shadowUbo.lightVP, float4(biasPos, 1.0f));
    float shadow = getShadow(shadowCoords, textureShadow, samplerShadow);
    
    float3 viewDir = normalize(ubo.viewPos.xyz - worldPos);
    float3 Lo = DirectLighting(normal, viewDir, albedo, F0, roughness, metallic, 1.0f, worldPos);
    
    Lo = pow(Lo, float3(0.45f, 0.45f, 0.45f));
    
    return float4(Lo + emissive, 1.0f);
}

/*

VertexIndex     UV          Pos
0               (0, 0)      (0, 0, 0, 1)
1               (1, 0)      (1, 0, 0, 1)
2               (0, 1)      (0, 1, 0, 1)

*/