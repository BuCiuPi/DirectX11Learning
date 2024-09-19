cbuffer cbDynamic : register(b0)
{
    float2 gTexel;
    float2 fill;
    float4 gWeights[3];
}

Texture2D gNormalDepthMap : register(t0);
Texture2D gInputImage : register(t1);

SamplerState samClampLinear : register(s0);

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
    vout.PosH = float4(vin.PosL, 1.0f);
    vout.Tex = vin.Tex;

    return vout;
}

#define gBlurRadius 5

float4 PS(VertexOut pin) : SV_Target
{
    float2 texOffset = gTexel;

    float totalWeight = ((float[4]) (gWeights[(5 + gBlurRadius) / 4]))[(5 + gBlurRadius) % 4];
    float4 color = totalWeight * gInputImage.SampleLevel(samClampLinear, pin.Tex, 0.0f);

    float4 centerNormalDepth = gNormalDepthMap.SampleLevel(samClampLinear, pin.Tex, 0.0f);

    for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
    {
        if (i == 0)
        {
            continue;
        }

        float2 tex = pin.Tex + i * texOffset;
        float4 neighborNormalDepth = gNormalDepthMap.SampleLevel(samClampLinear, tex, 0.0f);

        if (dot(neighborNormalDepth.xyz, centerNormalDepth.xyz) >= 0.8f
            && abs(neighborNormalDepth.a - centerNormalDepth.a) <= 0.2f)
        {
            float weight = ((float[4]) (gWeights[(i + gBlurRadius) / 4]))[(i + gBlurRadius) % 4];

            color += weight * gInputImage.SampleLevel(samClampLinear, tex, 0.0f);

            totalWeight += weight;
        }
    }

    return color / totalWeight;
}
