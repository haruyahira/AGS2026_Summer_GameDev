#ifndef MODEL_3D_COMMON_HLSLI
#define MODEL_3D_COMMON_HLSLI

struct VertexToPixel
{
    float4 Position : SV_POSITION;

    float2 TexCoords0 : TEXCOORD0;

    float3 WorldPos : TEXCOORD1;

    float3 Normal : TEXCOORD2;

    float4 Diffuse : COLOR0;
};

#endif