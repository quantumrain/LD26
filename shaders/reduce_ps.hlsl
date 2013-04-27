struct VS_OUTPUT
{
	float4 position	: SV_POSITION;
	float2 uv		: TEXCOORD0;
	float4 colour	: COLOR0;
};

Texture2D t0 : register(t0);
SamplerState s0;

float4 main(VS_OUTPUT input) : SV_TARGET
{
	return t0.Sample(s0, input.uv);
}