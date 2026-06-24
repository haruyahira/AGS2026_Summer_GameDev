Texture2D tex : register(t0);
SamplerState texSampler : register(s0);

cbuffer cbParam : register(b4)
{
    float4 g_color;
}

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 dif : COLOR0;
    float4 spc : COLOR1;
    float2 uv : TEXCOORD0;
    float2 suv : TEXCOORD1;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 srcCol = tex.Sample(texSampler, input.uv);

    float gray =
        srcCol.r * 0.299f +
        srcCol.g * 0.587f +
        srcCol.b * 0.114f;

    float4 dstCol = srcCol;

    dstCol.rgb = float3(gray, gray, gray);
    dstCol.a = 1.0f;

    return dstCol;
}