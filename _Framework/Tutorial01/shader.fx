//--------------------------------------------------------------------------------------
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

// the lighting equations in this code have been taken from https://www.3dgep.com/texturing-lighting-directx-11/
// with some modifications by David White

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 vOutputColor;
}

Texture2D txDiffuse[3] : register(t0);
SamplerState samLinear : register(s0);

#define MAX_LIGHTS 1
// Light types.
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

struct _Material
{
	float4  Emissive;       // 16 bytes
							//----------------------------------- (16 byte boundary)
	float4  Ambient;        // 16 bytes
							//------------------------------------(16 byte boundary)
	float4  Diffuse;        // 16 bytes
							//----------------------------------- (16 byte boundary)
	float4  Specular;       // 16 bytes
							//----------------------------------- (16 byte boundary)
	float   SpecularPower;  // 4 bytes
	bool    UseTexture;     // 4 bytes
	bool	UseBumpMap;		//4 bytes
	bool	UseDisplacementMap; // 4 bytes
	//float   Padding;        // 8 bytes
							//----------------------------------- (16 byte boundary)
};  // Total:               // 80 bytes ( 5 * 16 )

cbuffer MaterialProperties : register(b1)
{
	_Material Material;
};

struct Light
{
	float4      Position;               // 16 bytes
										//----------------------------------- (16 byte boundary)
	float4      Direction;              // 16 bytes
										//----------------------------------- (16 byte boundary)
	float4      Color;                  // 16 bytes
										//----------------------------------- (16 byte boundary)
	float       SpotAngle;              // 4 bytes
	float       ConstantAttenuation;    // 4 bytes
	float       LinearAttenuation;      // 4 bytes
	float       QuadraticAttenuation;   // 4 bytes
										//----------------------------------- (16 byte boundary)
	int         LightType;              // 4 bytes
	bool        Enabled;                // 4 bytes
	int2        Padding;                // 8 bytes
										//----------------------------------- (16 byte boundary)
};  // Total:                           // 80 bytes (5 * 16)

cbuffer LightProperties : register(b2)
{
	float4 EyePosition;                 // 16 bytes
										//----------------------------------- (16 byte boundary)
	float4 GlobalAmbient;               // 16 bytes
										//----------------------------------- (16 byte boundary)
	Light Lights[MAX_LIGHTS];           // 80 * 8 = 640 bytes
}; 

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float4 worldPos : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float3 eyeVectorTS : EYETS;
	float3 lightVectorTS: LIGHTTS;
	float3 lightDirectionToVertexTS : LIGHTDIRECTIONTS;
	float3 EyeToVertex : EYETOVERTEXTS;
};


float4 DoDiffuse(Light light, float3 L, float3 N)
{
	float NdotL = max(0, dot(N, L));
	return light.Color * NdotL;
}

float4 DoSpecular(Light lightObject, float3 vertexToEye, float3 lightDirectionToVertex, float3 Normal)
{
	float4 lightDir = float4(normalize(-lightDirectionToVertex),1);
	vertexToEye = normalize(vertexToEye);

	float lightIntensity = saturate(dot(Normal, lightDir));
	float4 specular = float4(0, 0, 0, 0);
	if (lightIntensity > 0.0f)
	{
		float3  reflection = normalize(2 * lightIntensity * Normal - lightDir);
		specular = pow(saturate(dot(reflection, vertexToEye)), Material.SpecularPower); // 32 = specular power
	}

	return specular;
}

float DoAttenuation(Light light, float d)
{
	return 1.0f / (light.ConstantAttenuation + light.LinearAttenuation * d + light.QuadraticAttenuation * d * d);
}

struct LightingResult
{
	float4 Diffuse;
	float4 Specular;
    float4 Direction;
};

LightingResult DoPointLight(Light light, float3 vertexToEye, float4 vertexPos, float3 N)
{
	LightingResult result;

	float3 LightDirectionToVertex = (vertexPos - light.Position).xyz;
	float distance = length(LightDirectionToVertex);
	LightDirectionToVertex = LightDirectionToVertex  / distance;

	float3 vertexToLight = (light.Position - vertexPos).xyz;
	distance = length(vertexToLight);
	vertexToLight = vertexToLight / distance;

	float attenuation = DoAttenuation(light, distance);
	attenuation = 1;


	result.Diffuse = DoDiffuse(light, vertexToLight, N) * attenuation;
	result.Specular = DoSpecular(light, vertexToEye, LightDirectionToVertex, N) * attenuation;
    result.Direction = float4(normalize(-LightDirectionToVertex), 1);

	return result;
}

LightingResult ComputeLighting(float4 vertexPos, float3 N)
{
	float3 vertexToEye = normalize(EyePosition - vertexPos).xyz;

    LightingResult totalResult = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };

	[unroll]
	for (int i = 0; i < MAX_LIGHTS; ++i)
	{
        LightingResult result = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }; //Diffuse,Specular, direction

		if (!Lights[i].Enabled) 
			continue;
		
		result = DoPointLight(Lights[i], vertexToEye, vertexPos, N);
		
		totalResult.Diffuse += result.Diffuse;
        totalResult.Specular += result.Specular;
        totalResult.Direction += result.Direction;
    }

	totalResult.Diffuse = saturate(totalResult.Diffuse);
    totalResult.Specular = saturate(totalResult.Specular);
    totalResult.Direction = saturate(totalResult.Direction);

	return totalResult;
}

LightingResult DoPointLightTangent(Light light, float3 vertexToEye, float4 vertexPos, float3 N, float3 lightVectorTS, float3 lightDirectionToVertexTS)
{
	LightingResult result;

	float distance = length(lightDirectionToVertexTS);

	float attenuation = DoAttenuation(light, distance);
	attenuation = 1;

	result.Diffuse = DoDiffuse(light, lightVectorTS, N) * attenuation;
	result.Specular = DoSpecular(light, vertexToEye, lightDirectionToVertexTS, N) * attenuation;
	result.Direction = float4(normalize(-lightDirectionToVertexTS), 1);

	return result;
}

LightingResult ComputeLightingTangent(float4 vertexPos, float3 N, float3 eyeVectorTS, float3 lightVectorTS, float3 lightDirectionToVertexTS)
{

	LightingResult totalResult = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };

	[unroll]
	for (int i = 0; i < MAX_LIGHTS; ++i)
	{
		LightingResult result = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }; //Diffuse,Specular, direction

		if (!Lights[i].Enabled)
			continue;

		result = DoPointLightTangent(Lights[i], eyeVectorTS, vertexPos, N, lightVectorTS, lightDirectionToVertexTS);

		totalResult.Diffuse += result.Diffuse;
		totalResult.Specular += result.Specular;
		totalResult.Direction += result.Direction;
	}

	totalResult.Diffuse = saturate(totalResult.Diffuse);
	totalResult.Specular = saturate(totalResult.Specular);
	totalResult.Direction = saturate(totalResult.Direction);

	return totalResult;
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0; 
    output.Pos = mul( input.Pos, World ); //Model to world
	output.worldPos = output.Pos; //Assign world pos for lighting calculations
    output.Pos = mul( output.Pos, View ); //World to view
    output.Pos = mul( output.Pos, Projection ); //View to projection
	
	output.Tex = input.Tex;

	float3 vertexToEye = EyePosition - output.worldPos.xyz;// Eye vector
	float3 vertexToLight = Lights[0].Position - output.worldPos.xyz; //Light vector
	float3 LightDirectionToVertex = output.worldPos.xyz - Lights[0].Position;
	float3 EyeToVertex = output.worldPos.xyz - EyePosition;

	//Build TBN matrix
	output.tangent = normalize(mul(input.tangent, World).xyz);
	output.binormal = normalize(mul(input.binormal, World).xyz);
	output.Norm = normalize(mul(input.Norm, World).xyz);
	
	//Tangent to world
	float3x3 TBN = float3x3(output.tangent, output.binormal, output.Norm);

	//World to tangent
	float3x3 TBN_inv = transpose(TBN);

	output.eyeVectorTS = normalize(mul(vertexToEye.xyz, TBN_inv));
	output.lightVectorTS = normalize(mul(vertexToLight.xyz, TBN_inv));

	output.EyeToVertex = mul(EyeToVertex.xyz, TBN_inv);
	output.lightDirectionToVertexTS = normalize(mul(LightDirectionToVertex.xyz, TBN_inv));

	
	
	if (Material.UseDisplacementMap)
	{
		output.Norm = mul(input.Norm, TBN_inv);
	}

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float4 PS(PS_INPUT IN) : SV_TARGET
{
	LightingResult lit;


	float4 texColor = {1, 1, 1, 1 };


	if (Material.UseTexture)
	{
		texColor = txDiffuse[0].Sample(samLinear, IN.Tex);
	}


	if (Material.UseBumpMap)
	{
		//Load normal map
		float4 bumpMap = txDiffuse[1].Sample(samLinear, IN.Tex);

		bumpMap = float4(normalize(bumpMap.xyz), 1);

		//Decompress normal map range from color component [0,1] to vector component [-1,1]
		//Goes from texel to normal vector
		bumpMap = (bumpMap * 2.0f) - 1.0f;

		//Fix: Inverse normal map due to incorrect texture UVs
		//bumpMap.y *= -1;

		IN.Norm = float4(normalize(bumpMap.xyz), 1);

		lit = ComputeLightingTangent(IN.worldPos, normalize(IN.Norm),IN.eyeVectorTS, IN.lightVectorTS,IN.lightDirectionToVertexTS);
		//lit = ComputeLighting(IN.worldPos, normalize(IN.Norm));
	}
	else
	{
		lit = ComputeLighting(IN.worldPos, normalize(IN.Norm));
	}
	


	float4 emissive = Material.Emissive;
	float4 ambient = Material.Ambient * GlobalAmbient;
	float4 diffuse = Material.Diffuse * lit.Diffuse;
	float4 specular = Material.Specular * lit.Specular;


	

	////Maximum parallax offset length that can be allowed

	//Depth of surface
	float4 fHeightMapScale = 0.1f; 

	//Orientation of the eye vector
	//1) Why is it -length?
	//2) Why the division with z?
	//float fParallaxLimit = -length(IN.eyeVectorTS.xy) / IN.eyeVectorTS.z;
	float fParallaxLimit = -length(IN.EyeToVertex.xy) / IN.EyeToVertex.z;
	fParallaxLimit *= fHeightMapScale;

	////Direction of offset vector
	float2 vOffsetDir = normalize(IN.eyeVectorTS.xy); //Normalised vector in the direction of offset
	float2 vMaxOffset = vOffsetDir * fParallaxLimit; //The direction is scaled by the maximum paralax offset

	//Number of samples
	int nNumSamples = (int)lerp(20, 4, dot(IN.EyeToVertex, IN.Norm)); //Lerp between a user specified minimum and maximum number of samples.
	float fStepSize = 1.0 / (float)nNumSamples;

	//For dynamic branching need to use SampleGrad instruction instead of Sample instruction.
	float2 dx = ddx(IN.Tex); 
	float2 dy = ddy(IN.Tex);

	//Init variables needed for dynamic loop 
	float fCurrRayHeight = 1.0; //Top of virtual volume
	//Bottom of virual volume
	float2 vCurrOffset = float2(0, 0);
	float2 vLastOffset = float2(0, 0);

	float fLastSampledHeight = 1;
	float fCurrSampledHeight = 1;

	int nCurrSample = 0;

	//Finds the intersection of the eye vector with the height 
	//- map as efficiently as possible
	while (nCurrSample < nNumSamples)
	{
		//Sample texture coordinates along our parallax offset vector
		fCurrSampledHeight = txDiffuse[2].SampleGrad(samLinear, IN.Tex + vCurrOffset, dx, dy).r;
		
		
		//If eye vector has a smaller height value than the height map
		//Then the intersection is in between the current and last sample
		if (fCurrSampledHeight > fCurrRayHeight)
		{
			float delta1 = fCurrSampledHeight - fCurrRayHeight;
			float delta2 = (fCurrRayHeight + fStepSize) - fLastSampledHeight;

			float ratio = delta1 / (delta1 + delta2);

			vCurrOffset = (ratio)* vLastOffset + (1.0 - ratio) * vCurrOffset;

			

			nCurrSample = nNumSamples + 1;

			
		}
		else
		{
			nCurrSample++;

			fCurrRayHeight -= fStepSize;

			vLastOffset = vCurrOffset;
			vCurrOffset += fStepSize * vMaxOffset;

			fLastSampledHeight = fCurrSampledHeight;


		}
	}

	//Solve for the linearly approximated intersection point between the last two samples
	float2 vFinalCoords = IN.Tex + vCurrOffset;

	float4 vFinalNormal = txDiffuse[1].Sample(samLinear, vFinalCoords);
	
	float4 vFinalColor = txDiffuse[0].Sample(samLinear, vFinalCoords);

	// Expand the final normal vector from [0,1] to [-1,1] range.
	vFinalNormal = vFinalNormal * 2.0f - 1.0f;

	vFinalNormal = float4(normalize(vFinalNormal.xyz), 1);

	float3 vAmbient = vFinalColor.rgb * 0.1f;
	float3 vDiffuse = vFinalColor.rgb * max(0.0f, dot(IN.lightVectorTS, vFinalNormal.xyz)) * 0.5f;

	vFinalColor.rgb = vAmbient + vDiffuse;


    float4 finalColor = (emissive + ambient + diffuse + specular) * texColor;

   
	if (Material.UseDisplacementMap)
	{
		return  vFinalColor;
	}
	else
	{
		return finalColor;
	}
}

//--------------------------------------------------------------------------------------
// PSSolid - render a solid color
//--------------------------------------------------------------------------------------
float4 PSSolid(PS_INPUT input) : SV_Target
{
	return vOutputColor;
}