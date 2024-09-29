#include "LightingHelper.fxh"

cbuffer cbPerObjectVS : register(b0)
{
    matrix gWorld;
    matrix gWorldViewProj;

    Material gMaterial;
}

cbuffer cbDirLightPS : register(b1)
{
    float3 AmbientDown;
    float3 AmbientRange;
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
};

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    float3 vNormalWorldSpace;

    output.Pos = mul(float4(input.Pos, 1.0f), gWorldViewProj);
    output.UV = input.UV;
    output.Normal = mul(input.Normal, (float3x3) gWorld);

    return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET0
{
    float3 DiffuseColor = DiffuseTexture.Sample(SamLinear, input.UV).rgb;
    //DiffuseColor *= DiffuseColor;
    //DiffuseColor += .4f;

    //float3 LinearLower = AmbientDown * AmbientDown;
    //float3 LinearRange = AmbientRange * AmbientRange;

    float3 LinearLower =  AmbientDown;
    float3 LinearRange =  AmbientRange;

    float3 AmbientColor = CalcAmbient(input.Normal, DiffuseColor, LinearLower, LinearRange - LinearLower);
    return float4(AmbientColor, 1.0f);
}