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
    float2 SizeW : SIZE;
    float4 Color : COLOR;
    uint Type : TYPE;
};

VertexOut DrawVS(Particle vin)
{
    VertexOut vout;

    float t = vin.Age;

    vout.PosW = 0.5f * t * t * gAccelW + t * vin.InitialVelW + vin.InitialPosW;

    float opacity = 1.0f - smoothstep(0.0f, 1.0f, t / 1.0f);
    vout.Color = float4(1.0f, 1.0f, 1.0f, opacity);

    vout.SizeW = vin.SizeW;
    vout.Type = vin.Type;

    return vout;
}

#define PT_EMITTER 0
#define PT_FLARE 1

struct GeoOut
{
    float4 PosH : SV_Position;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD;
};

[maxvertexcount(4)]
void DrawGS(point VertexOut gin[1],
			inout TriangleStream<GeoOut> triStream)
{
	if(gin[0].Type != PT_EMITTER)
	{
        float3 look = normalize(gEyePosW.xyz - gin[0].PosW);
        float3 right = normalize(cross(float3(0, 1, 0), look));
        float3 up = cross(look, right);

        float halfWidth = 0.5f * gin[0].SizeW.x;
        float halfHeight = 0.5f * gin[0].SizeW.y;

        float4 v[4];
        v[0] = float4(gin[0].PosW + halfWidth * right - halfHeight * up, 1.0f);
        v[1] = float4(gin[0].PosW + halfWidth * right + halfHeight * up, 1.0f);
        v[2] = float4(gin[0].PosW - halfWidth * right - halfHeight * up, 1.0f);
        v[3] = float4(gin[0].PosW - halfWidth * right + halfHeight * up, 1.0f);

        GeoOut gout;

        gout.PosH = mul(v[0], gViewProj);
        gout.Tex = float2(0.0f, 1.0f);
        gout.Color = gin[0].Color;
        triStream.Append(gout);

        gout.PosH = mul(v[1], gViewProj);
        gout.Tex = float2(1.0f, 1.0f);
        gout.Color = gin[0].Color;
        triStream.Append(gout);

        gout.PosH = mul(v[2], gViewProj);
        gout.Tex = float2(0.0f, 0.0f);
        gout.Color = gin[0].Color;
        triStream.Append(gout);

        gout.PosH = mul(v[3], gViewProj);
        gout.Tex = float2(1.0f, 0.0f);
        gout.Color = gin[0].Color;
        triStream.Append(gout);
    }	
}

float4 DrawPS(GeoOut pin) : SV_TARGET
{
    return gTexArray.Sample(samLinear, float3(pin.Tex, 0)) * pin.Color;
}
