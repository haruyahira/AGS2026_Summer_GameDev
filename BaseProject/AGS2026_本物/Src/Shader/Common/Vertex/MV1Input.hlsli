#ifndef MV1_INPUT_HLSLI
#define MV1_INPUT_HLSLI

struct VS_INPUT
{
    float4 Position : POSITION;

    float3 Normal : NORMAL0;

    float4 Diffuse : COLOR0;

    float4 Specular : COLOR1;

    float4 TexCoords0 : TEXCOORD0;

    float4 TexCoords1 : TEXCOORD1;

    int4 BlendIndices0 : BLENDINDICES0;

    float4 BlendWeight0 : BLENDWEIGHT0;

    int4 BlendIndices1 : BLENDINDICES1;

    float4 BlendWeight1 : BLENDWEIGHT1;
};

#endif