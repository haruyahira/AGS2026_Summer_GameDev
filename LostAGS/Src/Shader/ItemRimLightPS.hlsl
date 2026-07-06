#include "Common/Model3DCommon.hlsli"

cbuffer RimLightCB : register(b5)
{
    float4 g_RimColorPower;
    float4 g_RimParam;
    float4 g_CameraPos;
};

float4 main(VertexToPixel input) : SV_TARGET
{
    float3 normal =
        normalize(input.Normal);

    float3 viewDir =
        normalize(g_CameraPos.xyz - input.WorldPos);

    float ndotv =
        saturate(dot(normal, viewDir));

    float rim =
        1.0f - ndotv;

    // Ç©Ç»ÇËó÷äsäÒÇËÇ…çiÇÈ
    rim =
        smoothstep(0.88f, 1.0f, rim);

    rim =
        pow(rim, g_RimColorPower.w);

    if (rim < 0.05f)
    {
        discard;
    }

    float3 color =
        g_RimColorPower.rgb * rim * g_RimParam.x;

    float alpha =
        rim * g_RimParam.y;

    return float4(color, alpha);
}