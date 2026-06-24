#include "../Common/Vertex/MV1Input.hlsli"
#include "../Common/Model3DCommon.hlsli"

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

    result.x =
        dot(pos, g_Base.LocalWorldMatrix[0]);

    result.y =
        dot(pos, g_Base.LocalWorldMatrix[1]);

    result.z =
        dot(pos, g_Base.LocalWorldMatrix[2]);

    return result;
}

float3 MulWorldView(float4 pos)
{
    float3 result;

    result.x =
        dot(pos, g_Base.ViewMatrix[0]);

    result.y =
        dot(pos, g_Base.ViewMatrix[1]);

    result.z =
        dot(pos, g_Base.ViewMatrix[2]);

    return result;
}

float4 MulViewProjection(float4 pos)
{
    float4 result;

    result.x =
        dot(pos, g_Base.ProjectionMatrix[0]);

    result.y =
        dot(pos, g_Base.ProjectionMatrix[1]);

    result.z =
        dot(pos, g_Base.ProjectionMatrix[2]);

    result.w =
        dot(pos, g_Base.ProjectionMatrix[3]);

    return result;
}

VertexToPixel main(VS_INPUT input)
{
    VertexToPixel output;

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

float3 MulLocalWorldNormal(float3 normal)
{
    float3 result;

    result.x =
        dot(float4(normal, 0.0f),
            g_Base.LocalWorldMatrix[0]);

    result.y =
        dot(float4(normal, 0.0f),
            g_Base.LocalWorldMatrix[1]);

    result.z =
        dot(float4(normal, 0.0f),
            g_Base.LocalWorldMatrix[2]);

    return normalize(result);
}