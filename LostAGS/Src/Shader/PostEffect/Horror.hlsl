#include "../Common/Pixel/PixelShader2DHeader.hlsli"

// 定数バッファ
cbuffer cbParam : register(b4)
{
    float4 g_color;
}

float4 main(PS_INPUT PSInput) : SV_TARGET
{
    float2 uv = PSInput.uv;

    // 色収差
    float2 offset = float2(0.003f, 0.0f);

    float r = tex.Sample(texSampler, uv + offset).r;
    float g = tex.Sample(texSampler, uv).g;
    float b = tex.Sample(texSampler, uv - offset).b;

    float4 srcCol = float4(r, g, b, 1.0f);

    // 透明なら描画しない
    if (srcCol.a < 0.01f)
    {
        discard;
    }

    float4 dstCol = srcCol;

    // モノクロ
    float gray =
        (srcCol.r * 0.299f) +
        (srcCol.g * 0.587f) +
        (srcCol.b * 0.114f);

    // 元画像70% + モノクロ30%
    dstCol.rgb = lerp(srcCol.rgb, gray, 0.2f);

    // ノイズ
    float noise =
        frac(
            sin(dot(uv + g_color.x,
            float2(12.9898f, 78.233f)))
            * 43758.5453f
        );

    dstCol.rgb += noise * 1.00f;

    // ビネット
    float2 center = uv - 0.5f;

    float vignette =
        1.0f - length(center) * 1.5f;

    vignette = saturate(vignette);

    dstCol.rgb *= vignette;
    
    
// 全体を暗くする
    dstCol.rgb *= 0.7f;


    return dstCol;
}
