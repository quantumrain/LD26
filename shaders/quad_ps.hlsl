struct VS_OUTPUT
{
	float4 position	: SV_POSITION;
	float2 uv		: TEXCOORD0;
	float4 colour	: COLOR0;
};

float4 tint : register(c0);

float4 main(VS_OUTPUT input) : SV_TARGET
{
	return saturate(input.colour) * tint;
}