#include "common.hlsl"
#include "LightingHelper.fxh"

cbuffer cbDirLight : register(b1)
{
    float3 gAmbientDown;
    float pad;
    float3 gAmbientUp;
    float pad2;
    float3 gLightDir;
    float pad3;
    float3 gDirLightColor;
    float pad4;
}

static const float2 arrBasePos[4] =
{
    float2(-1.0, 1.0),
    float2(1.0, 1.0),
    float2(-1.0, -1.0),
    float2(1.0, -1.0),
};

struct VS_OUTPUT
{
    float4 Positon : SV_Position;
    float2 cpPos : TEXCOORD0;
    float2 id : TEXCOORD1;
};

VS_OUTPUT VS(uint VertexID : SV_VERTEXID)
{
    VS_OUTPUT output;

    output.Positon = float4(arrBasePos[VertexID].xy, 0.0, 1.0);
    output.cpPos = output.Positon.xy;
    output.id = VertexID.xx;

    return output;
}

float3 CalcDeferredDirectionalLight(float3 position, float3 gEyePos, GMaterial material)
{
    float toLight = normalize(gLightDir);
    float NDotL = dot(toLight, material.normal);
    float3 finalColor = gDirLightColor * saturate(NDotL);

    float3 ToEye = gEyePos - position;
    ToEye = normalize(ToEye);

    float3 HalfVector = normalize(ToEye + toLight);
    float NDotH = saturate(dot(HalfVector, material.normal));
    finalColor += gDirLightColor * pow(NDotH, material.specPow) * material.specIntensity;

    //return pow(NDotH, material.specPow);
    return finalColor * material.diffuseColor;
}

float4 PS(VS_OUTPUT In) : SV_TARGET
{
    SURFACE_DATA gbd = UnpackGBuffer_Loc(In.Positon.xy);

    GMaterial mat;
    MaterialFromGBuffer(gbd, mat);

    float position = CalcWorldPos(In.cpPos, gbd.LinearDepth);

    float3 finalColor = float3(0.0f, 0.0f, 0.0f);
    finalColor = CalcAmbient(mat.normal, mat.diffuseColor.xyz, gAmbientDown, gAmbientUp);
    finalColor += CalcDeferredDirectionalLight(position, EyePosition, mat);

    return float4(finalColor, 1.0f);
}


