cbuffer cbPerObject : register (b0)
{
    matrix gWorldView;
    matrix gWorldInvTransposeView;
    matrix gWorldViewProj;
    matrix gTexTransform;
}

Texture2D gDiffuseMap : register(t0);

SamplerState samLinear;

struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosV : POSITION;
    float3 NormalV : NORMAL;
    float2 Tex : TEXCORRD0;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    vout.PosV = mul(float4(vin.PosL, 1.0f), gWorldView).xyz;
    vout.NormalV = mul(vin.NormalL, (float3x3) gWorldInvTransposeView);

    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
    vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    pin.NormalV = normalize(pin.NormalV);

    //float4 texColor = gDiffuseMap.Sample(samLinear, pin.Tex);
    //clip(texColor.a - 0.1f);

    return float4(pin.NormalV, pin.PosV.z);
}
