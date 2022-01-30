
Texture2D tex : register(t0);
SamplerState samLinear : register(s0);


cbuffer PostProcessConstantBuffer
{
	float4 colour;
};

struct VS_INPUT
{
	float4 Pos : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
};


PS_INPUT QuadVS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	
	output.Pos = input.Pos;
	output.Tex = input.Tex;
	output.Norm = input.Norm;

	return output;
}

float4 QuadPS(PS_INPUT input) : SV_TARGET
{
	//Frame buffer
	float4 vColor = tex.Sample(samLinear,input.Tex);
	
	
	//float4 redTint = float4(0.0f, 0.1f, 0.0f, 1.0f);

	float4 redTint = colour;

	vColor.rgba = vColor.rgba + redTint;

	return vColor;
}

