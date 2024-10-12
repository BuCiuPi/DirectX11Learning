TextureCube skyTexture : register(t0);
Texture2D texture2 : register(t1);
SamplerState samLinear : register(s0);

cbuffer cbBuffer : register(b0)
{
    matrix gMVP;
}

struct VS_INPUT
{
    float3 Pos : POSITION;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float3 PosL : POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.PosL = input.Pos;
    output.PosH = mul(float4(input.Pos, 1.0f), gMVP).xyww;

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_Target
{
    return skyTexture.Sample(samLinear, input.PosL);
}