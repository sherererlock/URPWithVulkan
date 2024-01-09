
struct ShadowUBO
{
    float4x4 lightVP;
    float4 shadowBias; // (normalbias, w, 0, 0)
};

cbuffer shadowUbo : register(b0, space2)
{
    ShadowUBO shadowUbo;
}

Texture2D textureShadow : register(t1, space2);
SamplerState samplerShadow : register(s1, space2);

float getShadow(float4 ShadowCoords)
{
    ShadowCoords.xyz /= ShadowCoords.w;
    float depth = ShadowCoords.z;
    float2 uv = ShadowCoords.xy * 0.5f + float2(0.5f, 0.5f);
    bool inmap = uv.x <= 1.0f && uv.x >= 0.0f && uv.y <= 1.0f && uv.y >= 0.0f;
    float depthInMap = inmap ? textureShadow.Sample(samplerShadow, uv).r : 1.0f;

    return depth < depthInMap ? 1.0f : 0.0f;

}