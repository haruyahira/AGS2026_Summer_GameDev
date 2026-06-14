Texture2D<float4> texColor : register(t0);
Texture2D<float4> texNormal : register(t1);

SamplerState smp : register(s0);

struct PSInput
{
    float4 pos : SV_Position;
    float4 col : COLOR0;
    float4 spc : COLOR1;
    float2 uv : TEXCOORD0;
    float2 suv : TEXCOORD1;
};

float4 main(PSInput input) : SV_TARGET
{
    uint w, h;
    texColor.GetDimensions(w, h);

    // 2.0fだと太いので、自然にするなら1.0f
    float2 pixel = float2(
        1.0f / (float) w,
        1.0f / (float) h
    );

    float4 color = texColor.Sample(smp, input.uv);

    float3 centerNormal = texNormal.Sample(smp, input.uv).rgb;

    float3 normalR = texNormal.Sample(smp, input.uv + float2(pixel.x, 0.0f)).rgb;
    float3 normalL = texNormal.Sample(smp, input.uv + float2(-pixel.x, 0.0f)).rgb;
    float3 normalU = texNormal.Sample(smp, input.uv + float2(0.0f, -pixel.y)).rgb;
    float3 normalD = texNormal.Sample(smp, input.uv + float2(0.0f, pixel.y)).rgb;

    float normalDiff = 0.0f;
    normalDiff += distance(centerNormal, normalR);
    normalDiff += distance(centerNormal, normalL);
    normalDiff += distance(centerNormal, normalU);
    normalDiff += distance(centerNormal, normalD);

    // stepではなくsmoothstepにすると自然
    // 0.10～0.25の間でなめらかに線を出す
    float edge = smoothstep(0.10f, 0.25f, normalDiff);

    // 輪郭を強くしすぎない
    edge *= 0.25f;

    // 真っ黒ではなく、元の色を暗くした色にする
    float3 outlineColor = color.rgb * 0.18f;

    float3 finalColor = lerp(
        color.rgb,
        outlineColor,
        edge
    );

    return float4(finalColor, color.a);
}