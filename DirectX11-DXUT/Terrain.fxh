#include "LightingHelper.fxh"
Texture2DArray gLayerMapArray : register(t0);
Texture2D gBlendMap : register(t1);
Texture2D gHeightMap : register(t2);

SamplerState samLinear : register(s0);
SamplerState samHeightMap : register(s1);

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    Material gMaterial;
}

cbuffer WavePerFrameBuffer : register(b1)
{
    DirectionalLight gDirLight[3];
    float3 gEyePosW;

    float gMinDist;
    float gMaxDist;

	// Exponents for power of 2 tessellation.  The tessellation
	// range is [2^(gMinTess), 2^(gMaxTess)].  Since the maximum
	// tessellation is 64, this means gMaxTess can be at most 6
	// since 2^6 = 64.
    float gMinTess;
    float gMaxTess;
	
    float gTexelCellSpaceU;
    float gTexelCellSpaceV;
    float gWorldCellSpace;
    float2 gTexScale = 50.0f;
	
    float4 gWorldFrustumPlanes[6];
}

struct VS_INPUT
{
    float3 Pos : POSITION;
    float2 Tex : TEXCOORD0;
    float2 BoundsY : TEXCOORD1;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float3 PosW : POSITION;
    float2 Tex : TEXCOORD0;
    float2 BoundsY : TEXCOORD1;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;

    output.PosW = input.Pos;

    output.PosW.y = gHeightMap.SampleLevel(samHeightMap, input.Tex, 0).r;
    output.Pos = mul(mul(float4(output.PosW, 1.0f), View), Projection);

    output.Tex = input.Tex;
    output.BoundsY = input.BoundsY;

    return output;
}

float CalcTessFactor(float3 p)
{
    float d = distance(p, gEyePosW);

    float s = saturate((d - gMinDist) / (gMaxDist - gMinDist));
    return pow(2, (lerp(gMaxTess, gMinTess, s)));
}

bool AabbBehindPlaneTest(float3 center, float3 extends, float4 plane)
{
    float3 n = abs(plane.xyz);

    float r = dot(extends, n);

    float s = dot(float4(center, 1.0f), plane);

    return (s + r) < 0.0f;
}

bool AabbOutsideFrustumTes(float3 center, float3 extends, float4 frustumPlanes[6])
{
    for (int i = 0; i < 6; ++i)
    {
        if (AabbBehindPlaneTest(center, extends, frustumPlanes[i]))
        {
            return true;
        }
    }

    return false;
}

struct PatchTess
{
    float EdgeTess[4] : SV_TessFactor;
    float InsideTess[2] : SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<VS_OUTPUT, 4> patch, uint patchID : SV_PrimitiveID)
{
    PatchTess pt;

    float minY = patch[0].BoundsY.x;
    float maxY = patch[0].BoundsY.y;

    float3 vMin = float3(patch[2].PosW.x, minY, patch[2].PosW.z);
    float3 vMax = float3(patch[1].PosW.x, maxY, patch[1].PosW.z);

    float3 boxCenter = 0.5f * (vMin + vMax);
    float3 boxExtends = 0.5f * (vMax - vMin);

    if (AabbOutsideFrustumTes(boxCenter, boxExtends, gWorldFrustumPlanes))
    {
        pt.EdgeTess[0] = 0.0f;
        pt.EdgeTess[1] = 0.0f;
        pt.EdgeTess[2] = 0.0f;
        pt.EdgeTess[3] = 0.0f;

        pt.InsideTess[0] = 0.0f;
        pt.InsideTess[1] = 0.0f;

        return pt;
    }
    else
    {
        float3 e0 = 0.5f * (patch[0].PosW + patch[2].PosW);
        float3 e1 = 0.5f * (patch[0].PosW + patch[1].PosW);
        float3 e2 = 0.5f * (patch[1].PosW + patch[3].PosW);
        float3 e3 = 0.5f * (patch[2].PosW + patch[3].PosW);
        float3 c = 0.25f * (patch[0].PosW + patch[1].PosW + patch[2].PosW + patch[3].PosW);

        pt.EdgeTess[0] = CalcTessFactor(e0);
        pt.EdgeTess[1] = CalcTessFactor(e1);
        pt.EdgeTess[2] = CalcTessFactor(e2);
        pt.EdgeTess[3] = CalcTessFactor(e3);

        pt.InsideTess[0] = CalcTessFactor(c);
        pt.InsideTess[1] = pt.InsideTess[0];

        return pt;
    }

    return pt;
}

struct HullOut
{
    float3 PosW : POSITION;
    float2 Tex : TEXCOORD0;
};

[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
[maxtexxfactor(64.0f)]
HullOut HS(InputPatch<VS_OUTPUT, 4> p, uint i : SV_OutputControlPointID, uint patchID : SV_PrimitiveID)
{
    HullOut hout;

    hout.PosW = p[i].PosW;
    hout.Tex = p[i].Tex;

    return hout;
}

struct DomainOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float2 Tex : TEXCOORD0;
    float2 TiledTex : TEXCOORD1;
};

[domain("quad")]
DomainOut DS(PatchTess patchTess, float2 uv : SV_DomainLocation, const OutputPatch<HullOut, 4> quad)
{
    DomainOut dout;

    dout.PosW = lerp(lerp(quad[0].PosW, quad[1].PosW, uv.x), lerp(quad[2].PosW, quad[3].PosW, uv.x), uv.y);
    dout.Tex = lerp(lerp(quad[0].Tex, quad[1].Tex, uv.x), lerp(quad[2].Tex, quad[3].Tex, uv.x), uv.y);

    dout.TiledTex = dout.Tex * gTexScale;
    dout.PosW.y = gHeightMap.SampleLevel(samHeightMap, dout.Tex, 0).r;

    dout.PosH = mul(mul(float4(dout.PosW, 1.0f), View), Projection);

    return dout;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(DomainOut input) : SV_Target
{
    float2 leftTex = input.Tex + float2(-gTexelCellSpaceU, 0.0f);
    float2 rightTex = input.Tex + float2(gTexelCellSpaceU, 0.0f);
    float2 bottomTex = input.Tex + float2(0.0f, gTexelCellSpaceV);
    float2 TopTex = input.Tex + float2(0.0f, -gTexelCellSpaceV);

    float leftY = gHeightMap.SampleLevel(samHeightMap, leftTex, 0).r;
    float rightY = gHeightMap.SampleLevel(samHeightMap, rightTex, 0).r;
    float bottomY = gHeightMap.SampleLevel(samHeightMap, bottomTex, 0).r;
    float topY = gHeightMap.SampleLevel(samHeightMap, TopTex, 0).r;

    float3 tangent = normalize(float3(2.0f * gWorldCellSpace, rightY - leftY, 0.0f));
    float3 bitan = normalize(float3(0.0f, bottomY - topY, -2.0f * gWorldCellSpace));
    float3 normalW = cross(tangent, bitan);
    
    float3 toEye = gEyePosW - input.PosW;

	// Cache the distance to the eye from this surface point.
    float distToEye = length(toEye);

	// Normalize.
    toEye /= distToEye;

    
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);


    float4 outAmbient;
    float4 outDiffuse;
    float4 outSpecular;

    float4 c0 = gLayerMapArray.Sample(samLinear, float3(input.TiledTex, 0.0f));
    float4 c1 = gLayerMapArray.Sample(samLinear, float3(input.TiledTex, 1.0f));
    float4 c2 = gLayerMapArray.Sample(samLinear, float3(input.TiledTex, 2.0f));
    float4 c3 = gLayerMapArray.Sample(samLinear, float3(input.TiledTex, 3.0f));
    float4 c4 = gLayerMapArray.Sample(samLinear, float3(input.TiledTex, 4.0f));

    float4 t = gBlendMap.Sample(samLinear, input.Tex);

    float4 textureColor = c0;

    textureColor = lerp(textureColor, c1, t.r);
    textureColor = lerp(textureColor, c2, t.g);
    textureColor = lerp(textureColor, c3, t.b);
    textureColor = lerp(textureColor, c4, t.a);

    float4 litColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

		
        ComputeDirectionalLight(gMaterial, gDirLight[0], normalW, toEye, outAmbient, outDiffuse, outSpecular);

        ambient += outAmbient;
        diffuse += outDiffuse;
        spec += outSpecular;
        litColor += textureColor * (ambient + diffuse) + spec;
    
    litColor.a = gMaterial.Diffuse.a * textureColor.a;

    return float4(litColor.xyz, 1.0f);
}