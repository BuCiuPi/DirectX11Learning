cbuffer cbPerObject : register(b0)
{
    matrix gWorld;
    matrix gView;
    matrix gProj;
    matrix gTexTransform;
};

Texture2D gDiffuseMap : register(t0);

SamplerState samLinear : register(s0);

struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float2 Tex : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    vout.PosH = mul(mul(mul(float4(vin.PosL, 1.0f), gWorld), gView), gProj);
    vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

    return vout;
}

float4 PS(VertexOut input) : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
