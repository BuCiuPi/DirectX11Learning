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
    float3 black = float3(0.0f, 0.0f, 0.0f);

    float3 ambient = AmbientDown + up * AmbientRange;

    return ambient * color;
}

