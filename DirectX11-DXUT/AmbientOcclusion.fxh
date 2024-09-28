#include "LightingHelper.fxh"

cbuffer cbObject : register(b0)
{
	matrix MVP;
	Material gMaterial;
}

cbuffer WavePerFrameBuffer : register(b1)
{
	DirectionalLight gDirLight[3];
	float3 gEyePosW;
}

Texture2D modelTexture : register(t0);
SamplerState samLinear : register(s0);

struct VS_INPUT
{
	float3 Pos : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex : TEXCOORD;
	float AmbientAccess : AMBIENT;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float3 PosW : POSITION;
	float3 NormalW : NORMAL;
	float2 Tex : TEXCOORD;
	float AmbientAccess : AMBIENT;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT) 0;
	output.PosW = input.Pos;
	output.Pos = mul(float4(output.PosW, 1.0f), MVP);
    
	output.NormalW = input.NormalL;
	output.Tex = input.Tex;
	output.AmbientAccess = input.AmbientAccess;

	return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_Target
{
	input.NormalW = normalize(input.NormalW);
    
	float3 toEyeW = normalize(gEyePosW - input.PosW.xyz);
    
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float4 outAmbient;
	float4 outDiffuse;
	float4 outSpecular;

    [unroll]
	for (int i = 0; i < 3; ++i)
	{
		ComputeDirectionalLight(gMaterial, gDirLight[i], input.NormalW, toEyeW, outAmbient, outDiffuse, outSpecular);
		ambient += outAmbient;
		diffuse += outDiffuse;
		spec += outSpecular;
	}

	float4 textureColor = modelTexture.Sample(samLinear, input.Tex);
    
	float4 litColor = textureColor * (ambient + diffuse) + spec;
	litColor.a = gMaterial.Diffuse.a;
    
	return float4(input.AmbientAccess, input.AmbientAccess, input.AmbientAccess , 1.0f);
	//return litColor;
}