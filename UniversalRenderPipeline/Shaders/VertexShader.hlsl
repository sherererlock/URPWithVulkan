
struct Varyings {
	float4 positionCS : SV_POSITION;
	[[vk::location(0)]] float2 screenUV : VAR_SCREEN_UV;
};

Varyings main(uint vertexID : SV_VertexID)
{
	Varyings output;
	output.positionCS = float4(
		vertexID <= 1 ? -1.0 : 3.0,
		vertexID == 1 ? 3.0 : -1.0,
		0.0, 1.0
		);

	output.screenUV = float2(
		vertexID <= 1 ? 0.0 : 2.0,
		vertexID == 1 ? 2.0 : 0.0
		);

	return output;
}