#include "LightingHelper.fxh"
Texture2D modelTexture : register(t0);
SamplerState samLinear : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    matrix WorldInvTranspose;
    matrix gTexTransform;
    Material gMaterial;
}

cbuffer WavePerFrameBuffer : register(b1)
{
    DirectionalLight gDirLight[3];
    float3 gEyePosW;
}

struct VS_INPUT
{
    float3 Pos : POSITION;
    float3 NormalL : NORMAL;
    float2 Texture : TEXCOORD;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 Texture : TEXCOORD;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.PosW = mul(float4(input.Pos, 1.0f), World).xyz;
    output.Pos = mul(mul(float4(output.PosW, 1.0f), View), Projection);
    
    output.NormalW = mul(input.NormalL, (float3x3) WorldInvTranspose);
    output.Texture = mul(float4(input.Texture, 0.0f, 1.0f), gTexTransform).xy;

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
    
    float4 litColor = ambient + diffuse + spec;
    litColor.a = gMaterial.Diffuse.a;
    
    return litColor;
}