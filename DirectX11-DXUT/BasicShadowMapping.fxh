#include "LightingHelper.fxh"

cbuffer cbPerObject : register(b0)
{
    matrix gWorld;
    matrix gView;
    matrix gProj;
    matrix gWorldInvTranspose;
    matrix gTexTransform;
    matrix gShadowTransform;
    Material gMaterial;
};

cbuffer cbPerFrame : register(b1)
{
	DirectionalLight gDirLights[3];
	float3 gEyePosW;
};


Texture2D gDiffuseMap : register(t0);
TextureCube gCubeMap : register(t1);

Texture2D gShadowMap : register(t2);


SamplerState samLinear : register(s0);
SamplerComparisonState samShadow : register(s1);

struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 Tex : TEXCOORD0;
	float4 ShadowPosH : TEXCOORD1;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

	vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3) gWorldInvTranspose);


    vout.PosH = mul(mul(mul(float4(vin.PosL, 1.0f), gWorld), gView), gProj);
    vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	vout.ShadowPosH = mul(float4(vin.PosL, 1.0f), gShadowTransform);

    return vout;
}

float4 PS(VertexOut input) : SV_Target
{
    input.NormalW = normalize(input.NormalW);

	float3 toEye = gEyePosW - input.PosW;
	float distToEye = length(toEye);
	toEye /= distToEye;

	float4 texColor = float4(1, 1, 1, 1);
	texColor = gDiffuseMap.Sample(samLinear, input.Tex);

	float4 litColor = texColor;

	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 shadow = float3(1.0f, 1.0f, 1.0f);
	shadow[0] = CalcShadowFactor(samShadow, gShadowMap, input.ShadowPosH);

    [unroll]

	float4 A, D, S;
	ComputeDirectionalLight(gMaterial, gDirLights[0], input.NormalW, toEye, A, D, S);

	ambient += A;
	diffuse += shadow.x * D;
	specular += shadow.x * S;

	litColor = texColor * (ambient + diffuse) + specular;

	float3 incident = -toEye;
	float3 reflectionVector = reflect(incident, input.NormalW);
	float4 reflectionColor = gCubeMap.Sample(samLinear, reflectionVector);

	litColor += gMaterial.Reflect * reflectionColor;

	litColor.a = gMaterial.Diffuse.a * texColor.a;

    return float4(litColor.xyz, 1.0f);
}