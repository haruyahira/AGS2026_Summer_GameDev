#include "../Common/Pixel/PixelShader2DHeader.hlsli"

cbuffer cbParam : register(b4)
{
    float4 g_color;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float2 uv = input.uv;

    float2 offset = float2(0.003f, 0.0f);

    float r = tex.Sample(texSampler, uv + offset).r;
    float g = tex.Sample(texSampler, uv).g;
    float b = tex.Sample(texSampler, uv - offset).b;

    float4 srcCol = float4(r, g, b, 1.0f);
    float4 dstCol = srcCol;

    float gray =
        srcCol.r * 0.299f +
        srcCol.g * 0.587f +
        srcCol.b * 0.114f;

    dstCol.rgb = lerp(srcCol.rgb, float3(gray, gray, gray), 0.2f);

    float noise =
        frac(
            sin(dot(uv + g_color.x, float2(12.9898f, 78.233f)))
            * 43758.5453f
        );

    dstCol.rgb += noise * 0.12f;

 // ビネット
    float2 center = uv - 0.5f;

    float vignette =
    1.0f - length(center) * 0.8f;

    vignette = saturate(vignette);

    float vignettePower = 0.4f;

    dstCol.rgb *= lerp(1.0f, vignette, vignettePower);

 // 全体を少し暗くする
    dstCol.rgb *= 0.9f;

    dstCol.rgb = saturate(dstCol.rgb);
    dstCol.a = 1.0f;

    return dstCol;
}