
struct Varyings {
	float4 positionCS : SV_POSITION;
	float2 screenUV : VAR_SCREEN_UV;
};

float4 main(Varyings input) : SV_TARGET
{
	return float4(input.screenUV.x, input.screenUV.y, 0.0f, 1.0f);
}