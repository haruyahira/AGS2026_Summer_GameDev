#include "../Model3DCommon.hlsli"
#include "../Light3DCommon.hlsli"

Texture2D DiffuseTexture : register(t0);
SamplerState DiffuseSampler : register(s0);

struct PSOutput
{
    float4 color : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 depth : SV_TARGET2;
};

bool IsWallBlocked(
    float3 lightPos,
    float3 worldPos
)
{
    float distToPixel =
        distance(lightPos, worldPos);

    if (distToPixel > 900.0f)
    {
        return false;
    }

    int wallNum =
        (int) wallCount.x;

    float3 rayDir =
        worldPos - lightPos;

    float rayLen =
        length(rayDir);

    if (rayLen < 0.0001f)
    {
        return false;
    }

    rayDir /= rayLen;

    for (int i = 0; i < wallNum; i++)
    {
        // =========================
        // 壁情報
        // =========================

        float3 center =
            wallPosSize[i].xyz;

        float halfX =
   wallAxisX[i].w;

   
        float3 axisX =
    normalize(
        wallAxisX[i].xyz
    );


        float halfZ =
     wallPosSize[i].w;

       
        float3 axisZ =
    normalize(
        wallAxisZ[i].xyz
    );


        float halfY = wallAxisZ[i].w;

        float3 axisY =
            float3(0.0f, 1.0f, 0.0f);

        // =========================
        // レイ開始点をOBBローカルへ
        // =========================

        float3 localStart =
            lightPos - center;

        float localX =
            dot(localStart, axisX);

        float localY =
            dot(localStart, axisY);

        float localZ =
            dot(localStart, axisZ);

      // =========================
      // レイ方向をローカルへ
      // =========================

        float dirX =
    dot(rayDir, axisX);

        float dirY =
    dot(rayDir, axisY);

        float dirZ =
    dot(rayDir, axisZ);

      // ゼロ除算防止
        if (abs(dirX) < 0.0001f)
        {
            dirX = (dirX < 0.0f) ? -0.0001f : 0.0001f;
        }

        if (abs(dirY) < 0.0001f)
        {
            dirY = (dirY < 0.0f) ? -0.0001f : 0.0001f;
        }

        if (abs(dirZ) < 0.0001f)
        {
            dirZ = (dirZ < 0.0f) ? -0.0001f : 0.0001f;
        }
        // =========================
        // slab法
        // =========================

        float tx1 =
            (-halfX - localX) / dirX;

        float tx2 =
            (halfX - localX) / dirX;

        float ty1 =
            (-halfY - localY) / dirY;

        float ty2 =
            (halfY - localY) / dirY;

        float tz1 =
            (-halfZ - localZ) / dirZ;

        float tz2 =
            (halfZ - localZ) / dirZ;

        float tmin =
            max(
                max(
                    min(tx1, tx2),
                    min(ty1, ty2)
                ),
                min(tz1, tz2)
            );

        float tmax =
            min(
                min(
                    max(tx1, tx2),
                    max(ty1, ty2)
                ),
                max(tz1, tz2)
            );

        // =========================
        // ヒット判定
        // =========================

        if (tmax >= tmin - 5.0f)
        {
            if (tmin > 0.0f &&
                tmin < rayLen)
            {
                return true;
            }
        }
    }

    return false;
}

PSOutput main(VertexToPixel input)
{
    PSOutput output;

    // =========================
    // テクスチャ
    // =========================

    float4 texColor =
        DiffuseTexture.Sample(
            DiffuseSampler,
            input.TexCoords0
        );

    // =========================
    // 環境光
    // =========================

    //float3 light =
    //    float3(
    //        0.018f,
    //        0.018f,
    //        0.026f
    //    );
    
    
    float3 environmentLight =
    float3(
        0.06f,
        0.06f,
        0.08f
    );
    
    
    float3 light =
    environmentLight;



    int count =
        min(
            (int) lightCount.x,
            MAX_SHADER_LIGHT
        );

    [unroll(MAX_SHADER_LIGHT)]
    for (int i = 0; i < MAX_SHADER_LIGHT; i++)
    {
        if (i >= count)
        {
            break;
        }

        float3 lightPos =
            lightPosRange[i].xyz;

        float dist =
            distance(
                lightPos,
                input.WorldPos
            );

        float shadow =
            1.0f;

        // =========================
        // 壁遮蔽
        // =========================

        if (IsWallBlocked(
            lightPos,
            input.WorldPos
        ))
        {
            shadow = 0.08f;
        }

        // =========================
        // スポットライト
        // =========================

        float rate =
            CalcSpotLightRate(
                input.WorldPos,
                lightPos,
                lightPosRange[i].w,
                normalize(
                    lightDirInner[i].xyz
                ),
                lightDirInner[i].w,
                lightOuter[i].x
            );

        light +=
            lightColor[i].xyz *
            rate *
            shadow *
            1.4f;
    }

    // =========================
    // 最終色
    // =========================

    float3 baseColor =
        texColor.rgb *
        input.Diffuse.rgb;

  
    float3 finalColor =
    baseColor * light;

    output.color =
        float4(
            saturate(finalColor),
            texColor.a *
            input.Diffuse.a
        );

    // =========================
    // 法線
    // =========================

    float3 n =
        normalize(input.Normal);

    output.normal =
        float4(
            n * 0.5f + 0.5f,
            1.0f
        );

    // =========================
    // 深度
    // =========================

    float depth =
        saturate(input.Position.z);

    output.depth =
        float4(
            depth,
            depth,
            depth,
            1.0f
        );

    return output;
}