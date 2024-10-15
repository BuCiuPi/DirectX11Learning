#include "common.hlsl"

cbuffer cbPerObjectVS : register(b0)
{
    matrix gWorld;
    matrix gWorldViewProjection;
}

cbuffer cbPerObjectPS : register(b0)
{
    float3 gEyePosition;

    float SpecExp;
    float SpecIntensity;
}

Texture2D DiffuseTexture : register(t0);
SamplerState LinearSampler : register(s0);

struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
    float3 Normal : TEXCOORD1;
};

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT Output;
    float3 vNormalWorldSpace;

    Output.Position = mul(float4(input.Position, 1.0f), gWorldViewProjection);
    Output.UV = input.UV;
    Output.Normal = mul(input.Normal, (float3x3) gWorld);
    return Output;
}

struct PS_GBUFFER_OUT
{
    float4 ColorSpecInt : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float4 SpecPow : SV_TARGET2;
};

PS_GBUFFER_OUT PackGBuffer(float3 BaseColor, float3 Normal, float SpecIntensity, float SpecPower)
{
    PS_GBUFFER_OUT Output;

    float SpecPowerNorm = max(0.0001, (SpecPower - g_SpecPowerRange.x) / g_SpecPowerRange.y);

    Output.ColorSpecInt = float4(BaseColor.rgb, SpecIntensity);
    Output.Normal = float4(Normal * 0.5 + 0.5, 0.0);
    Output.SpecPow = float4(SpecPowerNorm, 0.0, 0.0, 0.0);

    return Output;
}

PS_GBUFFER_OUT PS(VS_OUTPUT In)
{
    float3 DiffuseColor = DiffuseTexture.Sample(LinearSampler, In.UV);
    //DiffuseColor *= DiffuseColor;

    //return float4(DiffuseColor, 1.0f);
    return PackGBuffer(DiffuseColor, normalize(In.Normal), SpecIntensity, SpecExp);
}
