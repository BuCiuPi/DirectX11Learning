#include "LightingHelper.fxh"
Texture2D modelTexture : register(t0);
TextureCube skyTexture : register(t1);
Texture2D normalTexture : register(t2);
SamplerState samLinear : register(s0);
SamplerState samAnisotropic : register(s1);

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
    float3 TangentL : TANGENT;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float2 Texture : TEXCOORD;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.PosW = mul(float4(input.Pos, 1.0f), World).xyz;
    output.TangentW = mul(input.TangentL, (float3x3) World);;

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
    
    float3 toEye = gEyePosW - input.PosW;

	// Cache the distance to the eye from this surface point.
    float distToEye = length(toEye);

	// Normalize.
    toEye /= distToEye;

    
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    float4 textureColor = modelTexture.Sample(samLinear, input.Texture);

    float4 outAmbient;
    float4 outDiffuse;
    float4 outSpecular;

    float3 normalMapSample = normalTexture.Sample(samLinear, input.Texture).rgb;
    float3 bumpedNormalW = normalMapSample.z > 0 ? NormalSampleToWorldSpace(normalMapSample, input.NormalW, input.TangentW) : input.NormalW;

    float4 litColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    [unroll]
    for (int i = 0; i < 3; ++i)
    {
		
        ComputeDirectionalLight(gMaterial, gDirLight[i], bumpedNormalW, toEye, outAmbient, outDiffuse, outSpecular);

        ambient += outAmbient;
        diffuse += outDiffuse;
        spec += outSpecular;
        litColor = (textureColor.a != 0.0f ? textureColor : 1.0f) * (ambient + diffuse) + spec;

        float3 incident = -toEye;
        float3 reflectionVector = reflect(incident, bumpedNormalW);
        float4 reflectionColor = skyTexture.Sample(samAnisotropic, reflectionVector);

        litColor += gMaterial.Reflect * reflectionColor;
    }
    
    litColor.a = gMaterial.Diffuse.a * textureColor.a != 0.0f ? textureColor.a : 1.0f;
    
    return float4(litColor.xyz, 1.0f);
}