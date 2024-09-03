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
    float2 Tex : TEXCOORD;
    float TessFactor : TESS;
};

#define minTess 25.0f
#define maxTess 1.0f
#define minTessFactor 1
#define maxTessFactor 5
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
    output.Tex = mul(float4(input.Texture, 0.0f, 1.0f), gTexTransform).xy;

    float d = distance(output.PosW, gEyePosW);

    float tess = saturate((minTess - d) / (minTess - maxTess));
    output.TessFactor = minTessFactor + tess * (maxTessFactor - minTessFactor);

    return output;
}

struct PatchTess
{
    float EdgeTess[3] : SV_TessFactor;
    float InsideTess : SV_InsideTessFactor;
};

PatchTess PatchHS(InputPatch<VS_OUTPUT, 3> patch, uint patchID : SV_PrimitiveID)
{
    PatchTess pt;

    pt.EdgeTess[0] = 0.5f * (patch[1].TessFactor + patch[2].TessFactor);
    pt.EdgeTess[1] = 0.5f * (patch[2].TessFactor + patch[0].TessFactor);
    pt.EdgeTess[2] = 0.5f * (patch[0].TessFactor + patch[1].TessFactor);

    pt.InsideTess = pt.EdgeTess[0];

    return pt;
}

struct HullOut
{
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float2 Tex : TEXCOORD;
};

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchHS")]
HullOut HS(InputPatch<VS_OUTPUT, 3> p, uint i : SV_OutputControlPointID, uint patchID : SV_PrimitiveID)
{
    HullOut hout;
    hout.PosW = p[i].PosW;
    hout.NormalW = p[i].NormalW;
    hout.TangentW = p[i].TangentW;
    hout.Tex = p[i].Tex;

    return hout;
}

struct DomainOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float2 Tex : TEXCOORD;
};

#define heighScale 0.07f

[domain("tri")]
DomainOut DS(PatchTess patchTess, float3 bary : SV_DomainLocation, const OutputPatch<HullOut, 3> tri)
{
    DomainOut dout;

    dout.PosW = 	bary.x * tri[0].PosW + 		bary.y * tri[1].PosW + 		bary.z * tri[2].PosW;
    dout.NormalW = 	bary.x * tri[0].NormalW +	bary.y * tri[1].NormalW + 	bary.z * tri[2].NormalW;
    dout.TangentW = bary.x * tri[0].TangentW + 	bary.y * tri[1].TangentW + 	bary.z * tri[2].TangentW;
    dout.Tex = 		bary.x * tri[0].Tex + 		bary.y * tri[1].Tex + 		bary.z * tri[2].Tex;

    dout.NormalW = normalize(dout.NormalW);


    const float MipInterval = 20.0f;
    float mipLevel = clamp((distance(dout.PosW, gEyePosW) - MipInterval) / MipInterval, 0.0f, 6.0f);

    float h = normalTexture.SampleLevel(samLinear, dout.Tex, mipLevel).a;

    dout.PosW += (heighScale * (h - 1.0f)) * dout.NormalW;

    dout.PosH = mul(mul(float4(dout.PosW, 1.0f), View), Projection);

    return dout;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(DomainOut input) : SV_Target
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

    float4 textureColor = modelTexture.Sample(samLinear, input.Tex);

    float4 outAmbient;
    float4 outDiffuse;
    float4 outSpecular;

    float3 normalMapSample = normalTexture.Sample(samLinear, input.Tex).rgb;
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