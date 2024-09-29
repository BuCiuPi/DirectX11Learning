#include "LightingHelper.fxh"

cbuffer cbPerObjectVS : register(b0)
{
    matrix gWorld;
    matrix gWorldViewProj;

    Material gMaterial;
}

cbuffer cbLightPS : register(b1)
{
    float3 AmbientDown;
    float3 AmbientRange;

    DirectionalLight gDirLight;

    float3 gEyePosition;
}

Texture2D DiffuseTexture : register(t0);
SamplerState SamLinear : register(s0);

struct VS_INPUT
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float3 WorldPos : TEXCOORD2;
};

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    float3 vNormalWorldSpace;

    output.WorldPos = mul(float4(input.Pos, 1.0f), gWorld).xyz;
    output.Pos = mul(float4(input.Pos, 1.0f), gWorldViewProj);
    output.UV = input.UV;
    output.Normal = mul(input.Normal, (float3x3) gWorld);

    return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET0
{
    float3 DiffuseColor = DiffuseTexture.Sample(SamLinear, input.UV).rgb;
    DiffuseColor += .3f;

    float3 LinearLower = AmbientDown * AmbientDown;
    float3 LinearRange = AmbientRange * AmbientRange;
    float3 AmbientColor = CalcAmbient(input.Normal, DiffuseColor, LinearLower, LinearRange - LinearLower);

    float3 DirectionalDiffuse = CalcDirectionalLightHLSLCookBook(input.WorldPos, gEyePosition, gMaterial, input.Normal, gDirLight);

    float3 finalColor = AmbientColor + DirectionalDiffuse * DiffuseColor;
    return float4(finalColor, 1.0f);
}