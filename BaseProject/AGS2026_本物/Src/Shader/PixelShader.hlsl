struct PS_INPUT
{
    float4 Position : SV_POSITION;

    float2 TexCoords0 : TEXCOORD0;

    float3 WorldPos : TEXCOORD1;

    float3 Normal : TEXCOORD2;

    float4 Diffuse : COLOR0;
};

Texture2D DiffuseTexture : register(t0);

SamplerState DiffuseSampler : register(s0);

cbuffer LightBuffer : register(b4)
{
    float4 lightCount;
    float4 lightPosRange[100];
    float4 lightColor[100];
    float4 lightDirInner[100];
    float4 lightOuter[100];
};

float4 main(PS_INPUT input) : SV_Target
{
    float4 texColor =
        DiffuseTexture.Sample(
            DiffuseSampler,
            input.TexCoords0
        );

    // 夜の最低明るさ
    float3 light =
        float3(0.018f, 0.018f, 0.026f);

    int count =
        (int) lightCount.x;

    for (int i = 0; i < count; i++)
    {
        float3 lightPos =
            lightPosRange[i].xyz;

        float range =
            lightPosRange[i].w;

        float3 spotDir =
            normalize(lightDirInner[i].xyz);

        float innerCos =
            lightDirInner[i].w;

        float outerCos =
            lightOuter[i].x;

        float3 toPixel =
            input.WorldPos - lightPos;

        float dist =
            length(toPixel);

        float3 toPixelDir =
            normalize(toPixel);

        // スポットライト方向との角度
        float spotCos =
            dot(spotDir, toPixelDir);

        // 外側角度から内側角度へなめらかに明るくする
        float spotRate =
            saturate(
                (spotCos - outerCos) /
                max(innerCos - outerCos, 0.001f)
            );

        // 距離減衰
        float distanceRate =
            saturate(1.0f - dist / range);

        // 店のスポットライトっぽく中心を強くする
        spotRate =
            spotRate * spotRate;

        distanceRate =
            distanceRate * distanceRate;

        float rate =
            spotRate * distanceRate;

        light +=
            lightColor[i].xyz *
            rate *
            1.4f;
    }

    float3 finalColor =
        texColor.rgb * light;

    return float4(
        saturate(finalColor),
        texColor.a
    );
}