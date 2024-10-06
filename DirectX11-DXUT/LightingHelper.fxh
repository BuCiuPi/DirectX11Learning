struct DirectionalLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float3 Direction;
    float pad;
};

struct PointLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;

    float3 Position;
    float Range;
	
    float3 Attenuation;
    float pad;
};

struct SpotLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;

    float3 Position;
    float Spot;

    float3 Direction;
    float Range;
	
    float3 Attenuation;
    float pad;
};

struct CapsuleLight
{
    float4 Diffuse;

    float3 Position;
    float Range;

    float3 Direction;
    float Len;
};

struct FourCapsuleLight
{
    float4 DiffuseR;
    float4 DiffuseG;
    float4 DiffuseB;

    float4 PositionX;
    float4 PositionY;
    float4 PositionZ;

    float4 Range;

    float4 DirectionX;
    float4 DirectionY;
    float4 DirectionZ;

    float4 Len;
};

struct Material
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Reflect;
};

void ComputeDirectionalLight(Material mat, DirectionalLight L, float3 normal, float3 toEye,
							out float4 ambient,
							out float4 diffuse,
							out float4 spec)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	
    float3 lightVec = -L.Direction;
    ambient = mat.Ambient * L.Ambient;
	
    float diffuseFactor = dot(lightVec, normal);
	
	[flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFractor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
		
        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        spec = specFractor * mat.Specular * L.Specular;
    }
}

float3 CalcDirectionalLightHLSLCookBook(float3 position, float3 gEyePos, Material material, float3 normal, DirectionalLight DirLight)
{
    float NDotL = dot(DirLight.Direction, normal);
    float3 finalColor = DirLight.Diffuse.rgb * saturate(NDotL);

    float3 ToEye = gEyePos - position;
    ToEye = normalize(ToEye);

    float3 HalfVector = normalize(ToEye + DirLight.Direction);
    float NDotH = saturate(dot(HalfVector, normal));
    finalColor += DirLight.Diffuse.rgb * pow(NDotH, material.Specular.a);

    return finalColor * material.Diffuse.rgb;
}

float3 CalcPointLightHLSLCookBook(float3 positon, float3 gEyePos, Material material, float3 normal, PointLight pointLight)
{
    float3 toLight = pointLight.Position - positon;
    float3 toEye = gEyePos - positon;

    float distToLight = length(toLight);
    toLight /= distToLight;

    float NDotL = (dot(toLight, normal));
    float3 finalColor = pointLight.Diffuse * NDotL;

    // specular
    toEye = normalize(toEye);
    float3 HalfVector = normalize(toEye + toLight);
    float NDotH = saturate(dot(HalfVector, normal));
    finalColor += pointLight.Diffuse.rgb * pow(NDotH, material.Specular.a) * material.Diffuse.a;

    float DistToLightNor = 1.0 - saturate(distToLight / pointLight.Range);
    float Attn = DistToLightNor * DistToLightNor;
    finalColor *= material.Diffuse * Attn;

    return finalColor;
}

float3 CalcSpotLightHLSLCookBook(float3 position, float3 gEyePos, Material material, float3 normal, SpotLight spotLight)
{
    float3 ToLight = spotLight.Position - position;
    float3 ToEye = gEyePos - position;
    float distToLight = length(ToLight);

    ToLight /= distToLight;
    float NdotL = saturate(dot(ToLight, normal));
    float3 finalColor = spotLight.Diffuse * NdotL;

    ToEye = normalize(ToEye);
    float3 halfVector = normalize(ToEye * ToLight);
    float NdotH = saturate(dot(halfVector, normal));
    finalColor += spotLight.Diffuse * pow(NdotH, material.Specular.a);

    float cosAng = dot(normalize(spotLight.Direction), -ToLight);
    float conAtt = saturate((cosAng - spotLight.Attenuation.y) * spotLight.Attenuation.x);
    conAtt *= conAtt;

    float distToLightNorm = 1.0 - saturate(distToLight * 1 / spotLight.Range);
    float attn = distToLightNorm * distToLightNorm;
    finalColor *= material.Diffuse * attn * conAtt;

    return finalColor;
}

float3 CalcCapsuleLightHLSLCookBook(float3 position, float3 gEyePos, Material material, float3 normal, CapsuleLight capsuleLight)
{
    float3 ToEye = gEyePos - position;

    // projection the position onto capsule center line
    float3 ToCapsuleStart = position - capsuleLight.Position;
    float distOnLine = dot(ToCapsuleStart, capsuleLight.Direction) / capsuleLight.Len;
    distOnLine = saturate(distOnLine) * capsuleLight.Len;
    float3 PointOnLine = capsuleLight.Position + capsuleLight.Direction * distOnLine;

    // get direction from position to Light
    float3 Tolight = PointOnLine - position;
    float distToLight = length(Tolight);

    Tolight /= distToLight; // normalize 
    float NDotL = saturate(dot(Tolight, normal));
    float3 finalColor = material.Diffuse * NDotL;

    ToEye = normalize(ToEye);
    float3 halfVector = normalize(ToEye + Tolight);
    float NDotH = saturate(dot(halfVector, normal));
    finalColor += pow(NDotH, material.Specular.a);

    float DistToLightNorm = 1.0 - saturate(distToLight * 1 / capsuleLight.Range);
    float attn = DistToLightNorm * DistToLightNorm;
    finalColor *= capsuleLight.Diffuse.rgb * attn * capsuleLight.Diffuse.a;

    return finalColor;
}

float3 GetDirToLight(float3 position, PointLight pointLight)
{
    float3 ToLight = pointLight.Position - position;
    return ToLight;
}

float3 GetColorFromCube(TextureCube cubeTexture, SamplerState cubeSampler, float3 sampleDirection, float3 colorLight)
{
    return colorLight * cubeTexture.Sample(cubeSampler, sampleDirection);
}

float3 CalcPointLightProjectedHLSLCookBook(float3 position, float3 gEyePos, Material material, float3 normal, PointLight pointLight)
{
    float3 ToLight = GetDirToLight(position, pointLight);
    float3 ToEye = gEyePos - position;
    float distToLight = length(ToLight);

    ToLight /= distToLight;
    float NDotL = saturate(dot(ToLight, normal));
    float3 finalColor = pointLight.Diffuse.xyz * NDotL;

    ToEye = normalize(ToEye);
    float3 halfVector = normalize(ToEye + ToLight);
    float NDotH = saturate(dot(halfVector, normal));
    finalColor += pointLight.Diffuse.xyz * pow(NDotH, material.Specular);

    float distToLightNorm = 1.0 - saturate(distToLight * 1 / pointLight.Range);
    float attn = distToLightNorm * distToLightNorm;
    finalColor *= material.Diffuse * attn;

    return finalColor;
}

float4 dot4x4(float4 to_capsule_start_x, float4 to_capsule_start_y, float4 to_capsule_start_z, float4 direction_x, float4 direction_y, float4 direction_z)
{
    return to_capsule_start_x * direction_x + to_capsule_start_y * direction_y + to_capsule_start_z * direction_z;
}

float4 dot4x1(float4 to_capsule_start_x, float4 to_capsule_start_y, float4 to_capsule_start_z, float3 val)
{
    return to_capsule_start_x * val.xxxx + to_capsule_start_y * val.yyyy + to_capsule_start_z * val.zzzz;
}

float3 CalcFourCapsuleLight(float3 position, float3 gEyePos, Material material, float3 normal, FourCapsuleLight fourLight)
{
    float3 ToEye = gEyePos - position;

    float4 ToCapsuleStartX = position.xxxx - fourLight.PositionX;
    float4 ToCapsuleStartY = position.yyyy - fourLight.PositionY;
    float4 ToCapsuleStartZ = position.zzzz - fourLight.PositionZ;

    float4 DistOnLine = dot4x4(ToCapsuleStartX, ToCapsuleStartY, ToCapsuleStartZ, fourLight.DirectionX, fourLight.DirectionY, fourLight.DirectionZ);
    float4 CapsuleLenSafe = max(fourLight.Len, 1.e-6);
    DistOnLine = fourLight.Len * saturate(DistOnLine / CapsuleLenSafe);

    float4 PointOnLineX = fourLight.PositionX + fourLight.DirectionX * DistOnLine;
    float4 PointOnLineY = fourLight.PositionY + fourLight.DirectionY * DistOnLine;
    float4 PointOnLineZ = fourLight.PositionZ + fourLight.DirectionZ * DistOnLine;

    float4 ToLightX = PointOnLineX - position.xxxx;
    float4 ToLightY = PointOnLineY - position.yyyy;
    float4 ToLightZ = PointOnLineZ - position.zzzz;

    float4 DistToLightSqr = dot4x4(ToLightX, ToLightY, ToLightZ, ToLightX, ToLightY, ToLightZ);
    float4 DistToLight = sqrt(DistToLightSqr);

    ToLightX /= DistToLight;
    ToLightY /= DistToLight;
    ToLightZ /= DistToLight;

    float4 NDotL = saturate(dot4x1(ToLightX, ToLightY, ToLightZ, normal));

    // calc specular

    ToEye = normalize(ToEye);
    float4 halfVectorX = ToEye.xxxx + ToLightX;
    float4 halfVectorY = ToEye.yyyy + ToLightY;
    float4 halfVectorZ = ToEye.zzzz + ToLightZ;

    float4 halfVectorSize = sqrt(dot4x4(halfVectorX, halfVectorY, halfVectorZ, halfVectorX, halfVectorY, halfVectorZ));
    float4 NDotH = saturate(dot4x1(halfVectorX / halfVectorSize, halfVectorY / halfVectorSize, halfVectorZ / halfVectorSize, normal));

    float4 SpecValue = pow(NDotH, material.Specular.a);

    float4 DistToLightNorm = 1.0 - saturate(DistToLight * 1/fourLight.Range);
    float4 attn = DistToLightNorm * DistToLightNorm;
    
    float4 pixelIntensity = (NDotL + SpecValue) * attn;
    float3 finalColor = float3(dot(fourLight.DiffuseR, pixelIntensity), dot(fourLight.DiffuseG, pixelIntensity), dot(fourLight.DiffuseB, pixelIntensity));
    finalColor *= material.Diffuse;

    return finalColor;
}

void ComputePointLight(Material mat, PointLight L, float3 pos, float3 normal, float3 toEye,
							out float4 ambient,
							out float4 diffuse,
							out float4 spec)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	
    float3 lightVec = L.Position - pos;
	
    float d = length(lightVec);
	
    if (d > L.Range)
    {
        return;
    }
	
    lightVec /= d;
	
    ambient = mat.Ambient * L.Ambient;
	
    float diffuseFactor = dot(lightVec, normal);
	
	[flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFractor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
		
        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        spec = specFractor * mat.Specular * L.Specular;
    }
	
    float att = 1.0f / dot(L.Attenuation, float3(1.0f, d, d * d));

    diffuse *= att;
    spec *= att;
}

void ComputeSpotLight(Material mat, SpotLight L, float3 pos, float3 normal, float3 toEye,
							out float4 ambient,
							out float4 diffuse,
							out float4 spec)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	
    float3 lightVec = L.Position - pos;
	
    float d = length(lightVec);
	
    if (d > L.Range)
    {
        return;
    }
	
    lightVec /= d;
	
    ambient = mat.Ambient * L.Ambient;
	
    float diffuseFactor = dot(lightVec, normal);
	
	[flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v = reflect(-lightVec, normal);
        float specFractor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
		
        diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        spec = specFractor * mat.Specular * L.Specular;
    }
    float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);
    float att = spot / dot(L.Attenuation, float3(1.0f, d, d * d));
	
    ambient *= spot;
    diffuse *= att;
    spec *= att;
}


float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
    float3 normalT = 2.0f * normalMapSample - 1.0f;

    float3 N = unitNormalW;
    float3 T = normalize(tangentW - dot(tangentW, N) * N);
    float3 B = cross(N, T);

    float3x3 TBN = float3x3(T, B, N);

    float3 bumpedNormalW = mul(normalT, TBN);

    return bumpedNormalW;
}

static const float SMAP_SIZE = 2048.0f;
static const float SMAP_DX = 1.0f / SMAP_SIZE;

float CalcShadowFactor(SamplerComparisonState samShadow, Texture2D shadowMap, float4 shadowPosH)
{
    shadowPosH.xyz /= shadowPosH.w;

    float depth = shadowPosH.z;

    const float dx = SMAP_DX;

    float percentLit = 0.0f;
    const float2 offset[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };

	[unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += shadowMap.SampleCmpLevelZero(samShadow, shadowPosH.xy + offset[i], depth).r;
    }

    return percentLit /= 9.0f;
}

float3 CalcAmbient(float3 normal, float3 color, float3 AmbientDown, float3 AmbientRange)
{
    float up = normal.y * 0.5f + 0.5f;

    float3 ambient = AmbientDown + up * AmbientRange;

    return ambient * color;
}

