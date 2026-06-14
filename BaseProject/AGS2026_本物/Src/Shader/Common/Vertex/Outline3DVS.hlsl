#include "MV1Input.hlsli"
#include "../Model3DCommon.hlsli"

struct DX_D3D11_VS_CONST_BUFFER_BASE
{
    float4 AntiViewportMatrix[4];
    float4 ProjectionMatrix[4];
    float4 ViewMatrix[3];
    float4 LocalWorldMatrix[3];
    float4 ToonOutLineSize;

    float DiffuseSource;
    float SpecularSource;
    float MulSpecularColor;
    float Padding;
};

cbuffer cbD3D11_CONST_BUFFER_VS_BASE : register(b1)
{
    DX_D3D11_VS_CONST_BUFFER_BASE g_Base;
};

float3 MulLocalWorld(float4 pos)
{
    float3 result;

    result.x = dot(pos, g_Base.LocalWorldMatrix[0]);
    result.y = dot(pos, g_Base.LocalWorldMatrix[1]);
    result.z = dot(pos, g_Base.LocalWorldMatrix[2]);

    return result;
}

float3 MulWorldView(float4 pos)
{
    float3 result;

    result.x = dot(pos, g_Base.ViewMatrix[0]);
    result.y = dot(pos, g_Base.ViewMatrix[1]);
    result.z = dot(pos, g_Base.ViewMatrix[2]);

    return result;
}

float4 MulViewProjection(float4 pos)
{
    float4 result;

    result.x = dot(pos, g_Base.ProjectionMatrix[0]);
    result.y = dot(pos, g_Base.ProjectionMatrix[1]);
    result.z = dot(pos, g_Base.ProjectionMatrix[2]);
    result.w = dot(pos, g_Base.ProjectionMatrix[3]);

    return result;
}

float3 MulLocalVectorToWorld(float3 vec)
{
    float4 v = float4(vec, 0.0f);

    float3 result;

    result.x = dot(v, g_Base.LocalWorldMatrix[0]);
    result.y = dot(v, g_Base.LocalWorldMatrix[1]);
    result.z = dot(v, g_Base.LocalWorldMatrix[2]);

    return normalize(result);
}

VertexToPixel main(VS_INPUT input)
{
    VertexToPixel output;

    // =====================================================
    // 調整値
    // =====================================================

    // 前後方向への押し出し
    const float OUTLINE_NORMAL_PUSH = 0.10f;

    // 横方向への拡大量
    // まずは大きめにして確認する
    const float OUTLINE_SIDE_EXPAND = 0.25f;

    // 上下方向への拡大量
    const float OUTLINE_Y_EXPAND = 0.0f;

    // 前後方向の位置補正
    const float OUTLINE_NORMAL_OFFSET = -0.05f;

    // 全体Y補正
    const float OUTLINE_Y_OFFSET = 0.0f;

    // =====================================================
    // 壁モデルの横方向設定
    // =====================================================

    // 0 = ローカルXを横方向として使う
    // 1 = ローカルZを横方向として使う
    const int WIDTH_AXIS = 1;

    // モデル中心
    // まずは 0.0f を試す
    const float LOCAL_CENTER_X = 0.0f;
    const float LOCAL_CENTER_Z = 0.0f;
    const float LOCAL_CENTER_Y = 0.0f;

    // =====================================================
    // ローカル座標
    // =====================================================

    float3 localPos =
        input.Position.xyz;

    // =====================================================
    // ワールド座標へ変換
    // =====================================================

    float4 localPos4 =
        float4(localPos, 1.0f);

    float3 worldPos =
        MulLocalWorld(localPos4);

    // =====================================================
    // モデル中心のワールド座標
    // =====================================================

    float3 localCenter;

    if (WIDTH_AXIS == 0)
    {
        localCenter =
            float3(LOCAL_CENTER_X, LOCAL_CENTER_Y, 0.0f);
    }
    else
    {
        localCenter =
            float3(0.0f, LOCAL_CENTER_Y, LOCAL_CENTER_Z);
    }

    float3 worldCenter =
        MulLocalWorld(float4(localCenter, 1.0f));

    // =====================================================
    // 法線方向
    // =====================================================

    float3 worldNormal =
        MulLocalVectorToWorld(normalize(input.Normal));

    worldNormal.y = 0.0f;

    float normalLen =
        length(worldNormal);

    if (normalLen > 0.0001f)
    {
        worldNormal /= normalLen;
    }
    else
    {
        worldNormal = float3(0.0f, 0.0f, 0.0f);
    }

    // =====================================================
    // 横方向ベクトル
    // =====================================================

    float3 localSideDir;

    if (WIDTH_AXIS == 0)
    {
        localSideDir =
            float3(1.0f, 0.0f, 0.0f);
    }
    else
    {
        localSideDir =
            float3(0.0f, 0.0f, 1.0f);
    }

    float3 worldSide =
        MulLocalVectorToWorld(localSideDir);

    worldSide.y = 0.0f;

    float sideLen =
        length(worldSide);

    if (sideLen > 0.0001f)
    {
        worldSide /= sideLen;
    }
    else
    {
        worldSide = float3(0.0f, 0.0f, 0.0f);
    }

    // =====================================================
    // ワールド座標で左右判定
    // =====================================================

    float sideDot =
        dot(worldPos - worldCenter, worldSide);

    float sideSign = 1.0f;

    if (sideDot < 0.0f)
    {
        sideSign = -1.0f;
    }

    // =====================================================
    // 座標補正
    // =====================================================

    // 前後方向
    worldPos +=
        worldNormal * OUTLINE_NORMAL_PUSH;

    worldPos +=
        worldNormal * OUTLINE_NORMAL_OFFSET;

    // 横方向拡大
    worldPos +=
        worldSide * sideSign * OUTLINE_SIDE_EXPAND;

    // 上下拡大
    if (input.Position.y > LOCAL_CENTER_Y)
    {
        worldPos.y += OUTLINE_Y_EXPAND;
    }
    else
    {
        worldPos.y -= OUTLINE_Y_EXPAND;
    }

    worldPos.y += OUTLINE_Y_OFFSET;

    // =====================================================
    // 変換
    // =====================================================

    float4 worldPos4 =
        float4(worldPos, 1.0f);

    float3 viewPos =
        MulWorldView(worldPos4);

    float4 viewPos4 =
        float4(viewPos, 1.0f);

    output.Position =
        MulViewProjection(viewPos4);

    output.WorldPos =
        worldPos;

    output.TexCoords0 =
        input.TexCoords0.xy;

    // PixelShaderで側面判定に使うため、ローカル法線を渡す
    output.Normal =
    normalize(input.Normal);

    output.Diffuse =
        input.Diffuse;

    return output;
}
