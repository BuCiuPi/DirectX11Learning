cbuffer cbSettings : register(b0)
{
    float4 gWeight[3];
}

Texture2D gInput : register(t0);
RWTexture2D<float4> gOutput : register(u0);

#define N 256
#define gBlurRadius 5
#define CacheSize (N+2*gBlurRadius)
groupshared float4 gCache[CacheSize];

[numthreads(N, 1, 1)]
void HorzBlurCS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    float w;
    float h;
    gInput.GetDimensions(w, h);
    
    if (groupThreadID.x < gBlurRadius)
    {
        int x = max(dispatchThreadID.x - gBlurRadius, 0);
        gCache[groupThreadID.x] = gInput[int2(x, dispatchThreadID.y)];
    }
    
    if (groupThreadID.x >= N - gBlurRadius)
    {
        int x = min(dispatchThreadID.x + gBlurRadius, w - 1);
        gCache[groupThreadID.x + 2 * gBlurRadius] = gInput[int2(x, dispatchThreadID.y)];

    }
    
    gCache[groupThreadID.x + gBlurRadius] = gInput[min(dispatchThreadID.xy, float2(w - 1, h - 1))];
    
    GroupMemoryBarrierWithGroupSync();
    
    float4 blurColor = float4(0, 0, 0, 0);
    
    [unroll]
    for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
    {
        int k = groupThreadID.x + gBlurRadius + i;
        blurColor += ((float[4]) (gWeight[(i + gBlurRadius) / 4]))[(i + gBlurRadius)%4] * gCache[k];
    }
    
    gOutput[dispatchThreadID.xy] = blurColor;
}

[numthreads(1, N, 1)]
void VertBlurCS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    float w;
    float h;
    gInput.GetDimensions(w, h);
    
    if (groupThreadID.y < gBlurRadius)
    {
        int y = max(dispatchThreadID.y - gBlurRadius, 0);
        gCache[groupThreadID.y] = gInput[int2(dispatchThreadID.x, y)];
    }
    
    if (groupThreadID.y >= N - gBlurRadius)
    {
        int y = min(dispatchThreadID.y + gBlurRadius, h - 1);
        gCache[groupThreadID.y + 2 * gBlurRadius] = gInput[int2( dispatchThreadID.x, y)];
    }
    
    gCache[groupThreadID.y + gBlurRadius] = gInput[min(dispatchThreadID.xy, float2(w - 1, h - 1))];
    
    GroupMemoryBarrierWithGroupSync();
    
    float4 blurColor = float4(0, 0, 0, 0);
    
    [unroll]
    for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
    {
        int k = groupThreadID.y + gBlurRadius + i;
        blurColor += ((float[4]) (gWeight[(i + gBlurRadius) / 4]))[(i + gBlurRadius)%4] * gCache[k];
    }
    
    gOutput[dispatchThreadID.xy] = blurColor;
}
    
