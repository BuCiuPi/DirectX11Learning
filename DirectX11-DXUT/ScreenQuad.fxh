Texture2D modelTexture : register(t0);
SamplerState samLinear : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    matrix WorldInvTranspose;
    matrix gTexTransform;
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

    float4 texColor = modelTexture.Sample(samLinear, input.Texture);
    
    return texColor;
}