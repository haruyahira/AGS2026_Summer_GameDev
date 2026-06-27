#include "../Common/Pixel/PixelShader2DHeader.hlsli"

cbuffer cbParam : register(b4)
{
    float4 g_param;
}

float4 main(PS_INPUT PSInput) : SV_TARGET
{
    float2 uv = PSInput.uv;

    float4 color =
        tex.Sample(texSampler, uv);

    // ’†‰›‚©‚ç‹——£
    float2 center = uv - 0.5f;

    center.x *= 1.2f;

    float dist = length(center);

    // ü•Ó‚¾‚¯ŒŒ
    float blood =
        smoothstep(0.35f, 0.75f, dist);

    // ƒmƒCƒY
    float noise =
        frac(
            sin(dot(
                uv * g_param.x,
                float2(12.9898f, 78.233f)))
            * 43758.5453f
        );

    blood += noise * 0.12f;

    // –¬“®
    float pulse =
        sin(g_param.x * 1.5f)
        * 0.5f + 0.5f;

    blood *=
        g_param.y + pulse * 0.25f;

    // Ô•‚¢ŒŒF
    float3 bloodColor =
        float3(0.35f, 0.02f, 0.02f);

    // ŒŒ‚ğ¬‚º‚é
    color.rgb =
        lerp(
            color.rgb,
            bloodColor,
            blood * 0.8f);

    // ˆÃ‚­
    color.rgb *=
        (1.0f - blood * 0.5f);

    return color;
}