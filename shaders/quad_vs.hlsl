struct VS_INPUT
{
	float2 position	: POSITION;
	float2 uv		: TEXCOORD0;
	float4 colour	: COLOR0;
};

struct VS_OUTPUT
{
	float4 position	: SV_POSITION;
	float2 uv		: TEXCOORD0;
	float4 colour	: COLOR0;
};

float4 cam : register(c0);

VS_OUTPUT main(VS_INPUT v)
{
	VS_OUTPUT output;

	output.position	= float4((v.position + cam.xy) * cam.zw, 0.0f, 1.0f);
	output.uv		= v.uv;
	output.colour	= v.colour;

	return output;
}