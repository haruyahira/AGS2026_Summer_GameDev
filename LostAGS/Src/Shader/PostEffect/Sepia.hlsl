#include "../Common/Pixel/PixelShader2DHeader.hlsli"

// セピア用パラメータ
cbuffer cbParam : register(b4)
{
    // g_color.x : セピア強度 (0.0 - 1.0)
    float4 g_color;
}

float4 main(PS_INPUT PSInput) : SV_TARGET
{
    float2 uv = PSInput.uv;

    // 原色取得
    float4 srcCol = tex.Sample(texSampler, uv);

    // 透明ピクセルは破棄
    if (srcCol.a < 0.01f)
    {
        discard;
    }

    // セピア色変換行列による変換
    float3 sepia;
    sepia.r = dot(srcCol.rgb, float3(0.393f, 0.769f, 0.189f));
    sepia.g = dot(srcCol.rgb, float3(0.349f, 0.686f, 0.168f));
    sepia.b = dot(srcCol.rgb, float3(0.272f, 0.534f, 0.131f));
    sepia = saturate(sepia);

    // 強度でオリジナルと混合
    float intensity = saturate(g_color.x);
    float3 outColor = lerp(srcCol.rgb, sepia, intensity);

    return float4(outColor, srcCol.a);
}