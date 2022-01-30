
Texture2D tex[6] : register(t0);
SamplerState samLinear : register(s0);

cbuffer ConstantBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 vOutputColor;
}

struct VS_INPUT
{
	float4 Pos : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 worldPos : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
};

PS_INPUT TerrainVS(VS_INPUT input)
{

	PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul(input.Pos, World); //Model to world
	output.worldPos = output.Pos; //Assign world pos for lighting calculations
	output.Pos = mul(output.Pos, View); //World to view
	output.Pos = mul(output.Pos, Projection); //View to projection

	output.Tex = input.Tex;
	output.Norm = input.Norm;

	return output;
}

float4 TerrainPS(PS_INPUT input) : SV_TARGET
{

	float4 lightDirt = tex[0].Sample(samLinear,input.Tex);
	float4 darkDirt = tex[1].Sample(samLinear, input.Tex);
	float4 grass = tex[2].Sample(samLinear, input.Tex);
	float4 stone = tex[3].Sample(samLinear, input.Tex);
	float4 snow = tex[4].Sample(samLinear, input.Tex);
	float4 blendMap = tex[5].Sample(samLinear, input.Tex);
	

	float4 vColor = lightDirt;
	vColor = lerp(vColor, darkDirt, blendMap.r);
	vColor = lerp(vColor, grass, blendMap.g);
	vColor = lerp(vColor, stone, blendMap.b);
	vColor = lerp(vColor, snow, blendMap.a);



	//return float4(1.0f,input.Pos.y,0.0f,1.0f);
	return vColor;
}