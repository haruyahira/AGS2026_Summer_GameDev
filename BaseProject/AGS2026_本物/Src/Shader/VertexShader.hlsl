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

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;

    float2 TexCoords0 : TEXCOORD0;

    float3 WorldPos : TEXCOORD1;

    float3 Normal : TEXCOORD2;

    float4 Diffuse : COLOR0;
};

// DXLib Direct3D11 MV1用 基本定数バッファ
struct DX_D3D11_VS_CONST_BUFFER_BASE
{
    float4 AntiViewportMatrix[4];

    float4 ProjectionMatrix[4];

    float4 ViewMatrix[3];

    float4 LocalWorldMatrix[3];

    float4 ToonOutLineSize;

    float DiffuseSource;
    float SpecularSource;
    float MulSpecularColor;
    float Padding;
};

cbuffer cbD3D11_CONST_BUFFER_VS_BASE : register(b1)
{
    DX_D3D11_VS_CONST_BUFFER_BASE g_Base;
};

float3 MulLocalWorld(float4 pos)
{
    float3 result;

    result.x = dot(pos, g_Base.LocalWorldMatrix[0]);
    result.y = dot(pos, g_Base.LocalWorldMatrix[1]);
    result.z = dot(pos, g_Base.LocalWorldMatrix[2]);

    return result;
}

float3 MulWorldView(float4 pos)
{
    float3 result;

    result.x = dot(pos, g_Base.ViewMatrix[0]);
    result.y = dot(pos, g_Base.ViewMatrix[1]);
    result.z = dot(pos, g_Base.ViewMatrix[2]);

    return result;
}

float4 MulViewProjection(float4 pos)
{
    float4 result;

    result.x = dot(pos, g_Base.ProjectionMatrix[0]);
    result.y = dot(pos, g_Base.ProjectionMatrix[1]);
    result.z = dot(pos, g_Base.ProjectionMatrix[2]);
    result.w = dot(pos, g_Base.ProjectionMatrix[3]);

    return result;
}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    float4 localPos =
        float4(input.Position.xyz, 1.0f);

    float3 worldPos =
        MulLocalWorld(localPos);

    float4 worldPos4 =
        float4(worldPos, 1.0f);

    float3 viewPos =
        MulWorldView(worldPos4);

    float4 viewPos4 =
        float4(viewPos, 1.0f);

    output.Position =
        MulViewProjection(viewPos4);

    output.WorldPos =
        worldPos;

    output.TexCoords0 =
        input.TexCoords0.xy;

    output.Normal =
        normalize(input.Normal);

    output.Diffuse =
        input.Diffuse;

    return output;
}