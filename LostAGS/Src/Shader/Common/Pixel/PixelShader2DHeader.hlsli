#pragma once

Texture2D tex : register(t0);
SamplerState texSampler : register(s0);

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 dif : COLOR0;
    float4 spc : COLOR1;
    float2 uv : TEXCOORD0;
    float2 suv : TEXCOORD1;
};