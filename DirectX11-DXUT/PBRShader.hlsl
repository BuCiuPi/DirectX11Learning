cbuffer cbObjectBuffer : register(b0)
{
    matrix worldMatrix;
}

cbuffer cbFrameBuffer : register(b1)
{
    matrix viewMatrix;
    matrix projectionMatrix;
    float4 lightPosition[4];
    float4 lightColor[4];
    float4 camPos;
    float4 customData;
}

struct VertexInputType
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD1;
};

SamplerState textureSample : register(s0);

Texture2D DiffuseTexture : register(t0);
TextureCube irradianceMap : register(t1);
TextureCube preFilterMap : register(t2);
Texture2D brdfLUT : register(t3);

//Texture2D normalMap : register(t4);
//Texture2D roughnessMap : register(t5);
//Texture2D metallicMap : register(t6);

PixelInputType VS(VertexInputType input)
{
    PixelInputType output;

    output.uv = input.uv;
    output.position = mul(float4(input.position, 1.0f), worldMatrix);
    output.position.w = 1.0f;
    output.worldPos = output.position.xyz;
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.normal = mul(normalize(input.normal), worldMatrix);
    output.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    return output;
}

static const float PI = 3.14159265359f;

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 * (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

float3 fresnelSchlickRoughness(float cosTheta, float F0, float roughtness)
{
    return F0 + (max(float3(1.0f - roughtness, 1.0f - roughtness, 1.0f - roughtness), F0) - F0) * pow(1.0f - cosTheta, 5.0f);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NDotV, float roughness)
{
    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;

    float num = NDotV;
    float denom = NDotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(float3 N ,float3 V, float3 L, float roughness)
{
    float NDotV = max(dot(N, V), 0.0f);
    float NDotL = max(dot(N, L), 0.0f);

    float ggx2 = GeometrySchlickGGX(NDotV, roughness);
    float ggx1 = GeometrySchlickGGX(NDotL, roughness);
	
    return ggx1 * ggx2;
}

float4 PS(PixelInputType input) : SV_TARGET
{
    float worldPos = input.worldPos;
    //float3 albedo = input.color.rgb;
    float3 albedo = DiffuseTexture.Sample(textureSample, input.uv);
    float3 Normal = input.normal/* * normalMap.Sample(textureSample, input.uv).rgb*/;
    //float roughness = roughnessMap.Sample(textureSample, input.uv).r;
    float roughness = customData.x;
    //float metallic = metallicMap.Sample(textureSample, input.uv).r;
    float metallic = 0.0f;

    float ao = 1.0f;

    float3 N = normalize(Normal);
    float3 V = normalize(camPos.xyz - worldPos);
    float3 R = reflect(-V, N);

    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedo, metallic);

    float3 Lo = float3(0.0f, 0.0f, 0.0f);

    //for (int i = 0; i < 4; ++i)
    //{
        float3 L = normalize(lightPosition[2].xyz - worldPos);
        float3 H = normalize(V + L);

        float distance = length(lightPosition[2].xyz - worldPos);
        float attenuation = 1.0f - saturate(distance/50.0f);
        float3 radiance = lightColor[3].xyz * attenuation * attenuation;

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float F = fresnelSchlick(max(dot(H, V), 0.0f), F0);

        float ks = F;
        float kD = float3(1.0f, 1.0f, 1.0f) - ks;
        kD *= 1.0 - metallic;

        float3 numerator = NDF * G * F;
        float denominator = 4.0f * max(dot(N, V), 0.0) * max(dot(N, L), 0.0f);
        float specular = numerator / max(denominator, 0.001f);

        float NDotL = max(dot(N, L), 0.0f);
        Lo += (kD * albedo / PI * specular) * radiance * NDotL;
	//}

    float3 F1 = fresnelSchlickRoughness(max(dot(N, V), 0.0f), F0, roughness);
    
    float ks1 = F1;
    float3 kD1 = 1.0 - ks1;
    kD1 *= 1.0 - metallic;

    float3 irradiance = irradianceMap.Sample(textureSample, N).rgb;
    float3 diffuse = irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    float3 prefilteredColor = preFilterMap.SampleLevel(textureSample, R, roughness * MAX_REFLECTION_LOD).rgb;
    float2 envBRDF = brdfLUT.Sample(textureSample, float2(max(dot(N, V), 0.0f), roughness)).rg;
    float3 specular1 = prefilteredColor * (F1 * envBRDF.x + envBRDF.y);

    float3 ambient = (kD1 * diffuse + specular1) * ao;

    float3 color = ambient + Lo;

    color = color / (color + float3(1.0f, 1.0f, 1.0f));
    color = pow(color, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));

    return float4(color, 1.0f);
}
