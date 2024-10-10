cbuffer cbObject : register(b0)
{
    matrix gWorld;
    matrix gWorldViewProj;
}

Texture2D DiffuseTexture : register(t0);
SamplerState samLinear : register(s0);

struct VS_INPUT
{
    float3 Pos : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 Tex : TEXCOORD;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.PosW = input.Pos;
    output.Pos = mul(float4(output.PosW, 1.0f), gWorldViewProj);
    
    output.NormalW = input.NormalL;
    output.Tex = input.Tex;

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_Target
{
    float4 textureColor = DiffuseTexture.Sample(samLinear, input.Tex);
    return textureColor;
}