Texture2D tex0 : register(t0);
SamplerState smp : register(s0);

// DxLib は cbuffer 使えない！
// float4 c0 = LightDir.xyz, ShadowBorder1
// float4 c1 = CameraPos.xyz, ShadowBorder2

float4 LightParam  : register(c0);   // xyz = LightDir, w = ShadowBorder1
float4 CameraParam : register(c1);   // xyz = CameraPos, w = ShadowBorder2

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

float4 PSMain(VS_OUT pin) : SV_TARGET
{
    float3 LightDir = LightParam.xyz;
    float ShadowBorder1 = LightParam.w;

    float3 CameraPos = CameraParam.xyz;
    float ShadowBorder2 = CameraParam.w;

    float3 N = normalize(pin.normal);
    float3 L = normalize(-LightDir);
    float3 V = normalize(CameraPos - pin.worldPos);

    float NdotL = dot(N, L);

    float shade = 1.0;
    if (NdotL < ShadowBorder2)
        shade = 0.25;
    else if (NdotL < ShadowBorder1)
        shade = 0.6;

    // 髪ハイライト
    float3 hairColor = float3(0.4, 0.4, 0.6);
    float hairBand = smoothstep(0.1, 0.3, dot(N, float3(0, 1, 0)));
    float3 hairHL = hairColor + hairBand * 0.5;

    float4 albedo = tex0.Sample(smp, pin.uv);

    if (pin.uv.y < 0.4)
        albedo.rgb = lerp(albedo.rgb, hairHL, 0.5);

    float3 col = albedo.rgb * shade;

    return float4(col, albedo.a);
}
