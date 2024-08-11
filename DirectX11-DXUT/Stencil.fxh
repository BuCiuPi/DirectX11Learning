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
    float4 Pos : POSITION;
    float3 NormalL : NORMAL;
    float2 Texture : TEXCOORD;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 Texture : TEXCOORD;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.PosW = mul(input.Pos, World);
    output.Pos = mul(mul(output.PosW, View), Projection);
    
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
    
    float toEyeW = normalize(gEyePosW - input.PosW.xyz);
    

    float4 texColor = modelTexture.Sample(samLinear, input.Texture);
    
    //if (texColor.a < 0.5f)
    //{
    //    texColor = float4(1.0f, 0.0f, 1.0f, 1.0f);
    //    //discard;

    //}
    clip(texColor.a - 0.5f);
    
    
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
    //ComputePointLight(gMaterial, gPointLight, input.PosW.xyz, input.NormalW, toEyeW, outAmbient, outDiffuse, outSpecular);
    //ambient += outAmbient;
    //diffuse += outDiffuse;
    //spec += outSpecular;

    //ComputeSpotLight(gMaterial, gSpotLight, input.PosW.xyz, input.NormalW, toEyeW, outAmbient, outDiffuse, outSpecular);
    //ambient += outAmbient;
    //diffuse += outDiffuse;
    //spec += outSpecular;
    
    float4 litColor = ambient + diffuse + spec;
    litColor.a = gMaterial.Diffuse.a;
    
    return litColor * texColor;
}