
struct VSOutput
{
    float4 Pos : SV_POSITION;
    [[vk::location(0)]] float2 UV : TEXCOORD0;
};

Texture2D textureColor : register(t0);
SamplerState samplerColor : register(s0);

float4 main(VSOutput input) :SV_TARGET
{
    return textureColor.Sample(samplerColor, input.UV);
}
