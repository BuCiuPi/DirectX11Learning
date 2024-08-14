#include "LightingHelper.fxh"

Texture2DArray modelTexture : register(t0);
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




struct VertexIn
{
    float3 PosW : POSITION;
    float2 SizeW : SIZE;
};

struct VertexOut
{
    float3 CenterW : POSITION;
    float2 SizeW : SIZE;
};

struct GeoOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 Tex : TEXCOORD;
    uint PrimID : SV_PrimitiveID;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VertexOut VS(VertexIn input)
{
    VertexOut output;
    output.CenterW = input.PosW;
    output.SizeW = input.SizeW;
    
    return output;
}

[maxvertexcount(4)]
void GS(point VertexOut gin[1],
        uint primID : SV_PrimitiveID,
        inout TriangleStream<GeoOut> triStream)
{
	//
	// Compute the local coordinate system of the sprite relative to the world
	// space such that the billboard is aligned with the y-axis and faces the eye.
	//
    const float2 gTexC[4] =
    {
        float2(0.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 1.0f),
		float2(1.0f, 0.0f)
    };

    float3 up = float3(0.0f, 1.0f, 0.0f);
    float3 look = gEyePosW - gin[0].CenterW;
    look.y = 0.0f; // y-axis aligned, so project to xz-plane
    look = normalize(look);
    float3 right = cross(up, look);

	//
	// Compute triangle strip vertices (quad) in world space.
	//
    float halfWidth = 0.5f * gin[0].SizeW.x;
    float halfHeight = 0.5f * gin[0].SizeW.y;
	
    float4 v[4];
    v[0] = float4(gin[0].CenterW + halfWidth * right - halfHeight * up, 1.0f);
    v[1] = float4(gin[0].CenterW + halfWidth * right + halfHeight * up, 1.0f);
    v[2] = float4(gin[0].CenterW - halfWidth * right - halfHeight * up, 1.0f);
    v[3] = float4(gin[0].CenterW - halfWidth * right + halfHeight * up, 1.0f);

	//
	// Transform quad vertices to world space and output 
	// them as a triangle strip.
	//
    GeoOut gout;
	[unroll]
    for (int i = 0; i < 4; ++i)
    {
        gout.PosH = mul(mul(v[i], View), Projection);
        gout.PosW = v[i].xyz;
        gout.NormalW = look;
        gout.Tex = gTexC[i];
        gout.PrimID = primID;
		
        triStream.Append(gout);
    }
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(GeoOut input) : SV_Target
{
    input.NormalW = normalize(input.NormalW);
    
    float toEyeW = normalize(gEyePosW - input.PosW.xyz);
    
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float3 uvm = float3(input.Tex, input.PrimID % 4);
    float4 texColor = modelTexture.Sample(samLinear, uvm);
    clip(texColor.a - 0.5f);
    
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