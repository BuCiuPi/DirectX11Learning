#include "LightingHelper.fxh"
cbuffer cbPerFrame : register(b1)
{
    DirectionalLight gDirLights[3];
    float3 gEyePosW;
    float boneID;
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
    matrix gBoneTransforms[161];
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
    float4 Color : COLOR;
};

VertexOut VS(SkinnedVertexIn vin)
{
    VertexOut vout;

    float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    weights[0] = vin.Weights.x;
    weights[1] = vin.Weights.y;
    weights[2] = vin.Weights.z;
    weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

    float3 posL;

    for (int i = 0; i < 3; ++i)
    {
        //posL += weights[i] * mul(float4(vin.PosL, 1.0f), gBoneTransforms[vin.BoneIndices[i]]).xyz;


        if (vin.BoneIndices[i] == boneID && vin.BoneIndices[i] != 0)
        {
            vout.Color = float4(weights[i], (1.0f * weights[i] - 0.5f) / 0.5f, 1.0f - weights[i], 1.0f);
        }
    }

    float4x4 boneTransform = 
    {
         0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,
    };
    float weisum = 1.0f - weights[3];

    boneTransform = gBoneTransforms[int(vin.BoneIndices.x)] * (vin.Weights.x / weisum);
    boneTransform += gBoneTransforms[int(vin.BoneIndices.y)] * (vin.Weights.y / weisum);
    boneTransform += gBoneTransforms[int(vin.BoneIndices.z)] * (vin.Weights.z / weisum);
    //boneTransform += gBoneTransforms[int(vin.BoneIndices.w)] * weights[3];

    posL = mul(float4(vin.PosL, 1.0f), boneTransform).xyz;

    vout.PosW = mul(float4(posL, 1.0f), gWorld).xyz;
    vout.PosH = mul(float4(posL, 1.0f), gWorldViewProj);
    vout.NormalW = vin.NormalL;
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

    return pin.Color;
}
