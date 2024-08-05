
#include "LightingHelper.fxh"

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    matrix WorldInvTranspose;
    Material gMaterial;
}

cbuffer PerFrameBuffer : register(b1)
{
    DirectionalLight gDirLight;
    PointLight gPointLight;
    SpotLight gSpotLight;
    float3 gEyePosW;
}

struct VS_INPUT
{
    float4 Pos : POSITION;
    float4 Color : COLOR;
    float3 NormalL : NORMAL;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 PosW : POSITION;
    float4 Color : COLOR0;
    float3 NormalW : NORMAL;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.PosW = mul(input.Pos, World);
    output.Pos = mul(mul(output.PosW, View), Projection);
    
    output.Color = input.Color;
    output.NormalW = mul(input.NormalL, (float3x3) WorldInvTranspose);

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_Target
{
    input.NormalW = normalize(input.NormalW);
    
    float toEyeW = normalize(gEyePosW - input.PosW.xyz);
    
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    float4 outAmbient;
    float4 outDiffuse;
    float4 outSpecular;

    ComputeDirectionalLight(gMaterial, gDirLight, input.NormalW, toEyeW, outAmbient, outDiffuse, outSpecular);
    ambient += outAmbient;
    diffuse += outDiffuse;
    spec += outSpecular;
    
    ComputePointLight(gMaterial, gPointLight, input.PosW.xyz, input.NormalW, toEyeW, outAmbient, outDiffuse, outSpecular);
    ambient += outAmbient;
    diffuse += outDiffuse;
    spec += outSpecular;

    ComputeSpotLight(gMaterial, gSpotLight, input.PosW.xyz, input.NormalW, toEyeW, outAmbient, outDiffuse, outSpecular);
    ambient += outAmbient;
    diffuse += outDiffuse;
    spec += outSpecular;
    
    float4 litColor = ambient + diffuse + spec;
    litColor.a = gMaterial.Diffuse.a;
    
    return litColor;
}