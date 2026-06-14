#ifndef LIGHT_3D_COMMON_HLSLI
#define LIGHT_3D_COMMON_HLSLI

#define MAX_SHADER_LIGHT 100


#define MAX_WALL 64

cbuffer LightCB : register(b4)
{
    float4 lightCount;

    float4 lightPosRange[MAX_SHADER_LIGHT];

    float4 lightColor[MAX_SHADER_LIGHT];

    float4 lightDirInner[MAX_SHADER_LIGHT];

    float4 lightOuter[MAX_SHADER_LIGHT];

    // =====================
    // ï«èÓïÒ
    // =====================

    // xyz = ï«íÜêS
    // w   = îºåa
    float4 wallPosSize[MAX_WALL];

    // ç°å„égÇ§
    float4 wallAxisX[MAX_WALL];

    float4 wallAxisZ[MAX_WALL];

    float4 wallCount;
};

float CalcSpotLightRate(
    float3 worldPos,
    float3 lightPos,
    float range,
    float3 spotDir,
    float innerCos,
    float outerCos)
{
    float3 toPixel =
        worldPos - lightPos;

    float dist =
        length(toPixel);

    float3 toPixelDir =
        normalize(toPixel);

    float spotCos =
        dot(spotDir, toPixelDir);

    float spotRate =
        saturate(
            (spotCos - outerCos) /
            max(innerCos - outerCos, 0.001f)
        );

    float distanceRate =
        saturate(1.0f - dist / range);

    spotRate =
        spotRate * spotRate;

    distanceRate =
        distanceRate * distanceRate;

    return spotRate * distanceRate;
}

#endif
