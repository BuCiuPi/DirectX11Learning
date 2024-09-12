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

Texture1D gRandomTex : register(t0);
SamplerState samLinear : register(s0);

float3 RandUnitVec3(float offset)
{
    float u = (gGameTime + offset);

    float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;

    return normalize(v);
}

float3 RandVec3(float offset)
{
    float u = (gGameTime + offset);

    float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;

    return v;
}

#define PT_EMITTER 0
#define PT_FLARE 1

struct Particle
{
    float3 InitialPosW : POSITION;
    float3 InitialVelW : VELOCITY;
    float2 SizeW : SIZE;
    float Age : AGE;
    uint Type : TYPE;
};

Particle StreamOutVS(Particle vin)
{
    return vin;
}


[maxvertexcount(6)]
void StreamOutGS(point Particle gin[1],
				inout PointStream<Particle> ptStream)
{
    gin[0].Age += gTimeStep;

    if (gin[0].Type == PT_EMITTER)
    {
        if (gin[0].Age > 0.002f)
        {
            for (int i = 0; i < 5; ++i)
            {
                float3 vRandom = 35.0f * RandVec3((float) i / 5.0f);
                vRandom.y = 20.0f;

                Particle p;
                p.InitialPosW = gEmitPosW.xyz + vRandom;
                p.InitialVelW = float3(0.0f, -20.0f, 0.0f);
                p.SizeW = float2(1.0f, 1.0f);
                p.Age = 0.0f;
                p.Type = PT_FLARE;

                ptStream.Append(p);
            }


            gin[0].Age = 0.0f;
        }

        ptStream.Append(gin[0]);
    }
    else
    {
        if (gin[0].Age <= 3.0f)
        {
            ptStream.Append(gin[0]);
        }
    }
}
