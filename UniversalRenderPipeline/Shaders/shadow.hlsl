
struct ShadowUBO
{
    float4x4 lightVP;
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
    float depthInMap = textureShadow.Sample(samplerShadow, uv).r;

    return depth < depthInMap ? 1.0f : 0.0f;

}