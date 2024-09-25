#include "LightingHelper.fxh"
cbuffer cbPerFrame : register (b1)
{
    DirectionalLight gDirLights[3];
    float3 gEyePosW;
}

cbuffer cbPerObject : register(b0)
{
    matrix gWorld;
    matrix gWorldViewProj;
    matrix gWorldInvTranspose;
    matrix gTexTransform;

    Material gMaterial;
}

cbuffer cbSkinned : register(b2)
{
    matrix gBoneTransform[78];
}

Texture2D gDiffuseMap : register(t0);

SamplerState gSamLinear : register(s0);

struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
    float4 TangentL : TANGENT;
};

struct SkinnedVertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
    float3 Weights : WEIGHTS;
    uint4 BoneIndices : BONEINDICES;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 Tex : TEXCOORD;
};

VertexOut VS(SkinnedVertexIn vin)
{
    VertexOut vout;

    float weight[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    weight[0] = vin.Weights.x;
    weight[1] = vin.Weights.y;
    weight[2] = vin.Weights.z;
    weight[3] = 1.0f - weight[0] - weight[1] - weight[2];

    float3 PosL = float3(0.0f, 0.0f, 0.0f);
    float3 NormalL = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 4; ++i)
    {
		
        PosL += weight[i] * mul(float4(vin.PosL, 1.0f), gBoneTransform[vin.BoneIndices[i]]).xyz;
        NormalL += weight[i] * mul(vin.NormalL, (float3x3)gBoneTransform[vin.BoneIndices[i]]);
    }

    vout.PosW = mul(float4(PosL, 1.0f), gWorld);
    vout.NormalW = mul(NormalL, (float3x3)gWorldInvTranspose);

    vout.PosH = mul(float4(PosL, 1.0f), gWorldViewProj);
    vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    pin.NormalW = normalize(pin.NormalW);

    float3 ToEye = gEyePosW - pin.PosW;
    float distToEye = length(ToEye);
    ToEye /= distToEye;

    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

    float4 outAmbient;
    float4 outDiffuse;
    float4 outSpecular;

    [unroll]
    for (int i = 0; i < 3; ++i)
    {
        ComputeDirectionalLight(gMaterial, gDirLights[i], pin.NormalW, ToEye, outAmbient, outDiffuse, outSpecular);
        ambient += outAmbient;
        diffuse += outDiffuse;
        spec += outSpecular;
    }

    float4 textureColor = gDiffuseMap.Sample(gSamLinear, pin.Tex);
    
    float4 litColor = textureColor * (ambient + diffuse) + spec;
    litColor.a = gMaterial.Diffuse.a;

    return litColor;
}
