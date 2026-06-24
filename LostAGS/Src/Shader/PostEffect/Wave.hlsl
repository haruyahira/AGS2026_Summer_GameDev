#include "../Common/Pixel/PixelShader2DHeader.hlsli"

cbuffer cbParam : register(b4)
{
    float4 g_color;
}

float4 main(PS_INPUT PSInput) : SV_TARGET
{
    float2 uv = PSInput.uv;

    float4 srcCol = tex.Sample(texSampler, uv);

    if (srcCol.a < 0.01f)
    {
        discard;
    }

    float time = g_color.x;
    float amplitude = g_color.y;
    float frequency = g_color.z;
    float speed = g_color.w;

    float wave =
        sin(uv.x * frequency + time * speed)
        * amplitude;

    uv.y += wave;

    float4 dstCol = tex.Sample(texSampler, uv);

    return dstCol;
}