cbuffer cbFixed : register(b0)
{
	// Net constant acceleration used to accerlate the particles.
    float3 gAccelW = { 0.0f, 7.8f, 0.0f };
    float cfill;
};

cbuffer cbPerFrame : register(b1)
{
    matrix gViewProj;

    float3 gEyePosW;
    float gGameTime;

    float3 gEmitPosW;
    float gTimeStep;

    float3 gEmitDirW;
    float gFill;
}

Texture2DArray gTexArray : register(t0);
SamplerState samLinear : register(s0);

struct Particle
{
    float3 InitialPosW : POSITION;
    float3 InitialVelW : VELOCITY;
    float2 SizeW : SIZE;
    float Age : AGE;
    uint Type : TYPE;
};

struct VertexOut
{
    float3 PosW : POSITION;
    uint Type : TYPE;
};

VertexOut DrawVS(Particle vin)
{
    VertexOut vout;

    float t = vin.Age;

    vout.PosW = 0.5f * t * t * gAccelW + t * vin.InitialVelW + vin.InitialPosW;

    vout.Type = vin.Type;

    return vout;
}


#define PT_EMITTER 0
#define PT_FLARE 1

struct GeoOut
{
    float4 PosH : SV_Position;
    float2 Tex : TEXCOORD;
};

[maxvertexcount(2)]
void DrawGS(point VertexOut gin[1],
			inout LineStream<GeoOut> lineStream)
{
    if (gin[0].Type != PT_EMITTER)
    {
        float3 p0 = gin[0].PosW;
        float3 p1 = gin[0].PosW + 0.07*gAccelW;

        GeoOut gout;

        gout.PosH = mul(float4(p0, 1.0f), gViewProj);
        gout.Tex = float2(0.0f, 0.0f);
        lineStream.Append(gout);

        gout.PosH = mul(float4(p1, 1.0f), gViewProj);
        gout.Tex = float2(1.0f, 1.0f);
        lineStream.Append(gout);
    }
}

float4 DrawPS(GeoOut pin) : SV_TARGET
{
    return gTexArray.Sample(samLinear, float3(pin.Tex, 0));
}
