
#include "LightManager.h"
#include "../../Object/Furniture/Wall.h"
#include "../../Object/Furniture/LightBlocker.h"
#include <math.h>


void LightManager::Init()
{
    constBufferHandle_ =
        CreateShaderConstantBuffer(
            sizeof(LightCB)
        );
}

void LightManager::Release()
{
    if (constBufferHandle_ != -1)
    {
        DeleteShaderConstantBuffer(
            constBufferHandle_
        );

        constBufferHandle_ = -1;
    }
}

void LightManager::AddLight(
    VECTOR pos,
    VECTOR color,
    VECTOR dir,
    float range,
    float innerAngleRad,
    float outerAngleRad,
    bool isWallBlocked)
{
    if ((int)lights_.size() >= MAX_LIGHT)
    {
        return;
    }

    ShaderLight light;

    light.pos = pos;
    light.color = color;
    light.dir = VNorm(dir);
    light.range = range;
    light.innerCos = cosf(innerAngleRad);
    light.outerCos = cosf(outerAngleRad);

    // 天井ライトは壁で遮る
    light.isWallBlocked = isWallBlocked;

    lights_.push_back(light);
}
void LightManager::Clear()
{
    lights_.clear();
}

void LightManager::SetFlashLight(
    VECTOR pos,
    VECTOR color,
    VECTOR dir,
    float range,
    float innerAngleRad,
    float outerAngleRad)
{

    useFlashLight_ = true;

    flashLight_.pos = pos;
    flashLight_.color = color;
    flashLight_.dir = VNorm(dir);
    flashLight_.range = range;
    flashLight_.innerCos = cosf(innerAngleRad);
    flashLight_.outerCos = cosf(outerAngleRad);

    // 懐中電灯は今回は壁遮蔽しない
    flashLight_.isWallBlocked = false;

}

void LightManager::DisableFlashLight()
{
    useFlashLight_ = false;
}

void LightManager::SendToShader()
{
    if (constBufferHandle_ == -1)
    {
        return;
    }

    LightCB* cb =
        (LightCB*)GetBufferShaderConstantBuffer(
            constBufferHandle_
        );

    if (cb == nullptr)
    {
        return;
    }

    *cb = {};

    int count = 0;

    // =========================
    // Player懐中電灯を先に登録
    // =========================
    if (useFlashLight_ && count < MAX_LIGHT)
    {
        WriteLightToCB(cb, count, flashLight_);
        count++;
    }

    // =========================
    // 天井ライト
    // =========================
    for (int i = 0; i < (int)lights_.size(); i++)
    {
        if (count >= MAX_LIGHT)
        {
            break;
        }

        const ShaderLight& light = lights_[i];

        // 壁で遮られているライトはShaderへ送らない
      /*  if (IsBlockedByWall(light))
        {
            continue;
        }*/

        WriteLightToCB(cb, count, light);
        count++;
    }

    int wallCount = 0;

    for (const LightBlocker* wall : wallBlockers_)
    {
        if (wall == nullptr)
        {
            continue;
        }

        float dist =
            VSize(
                VSub(
                    wall->GetPos(),
                    viewPoint_
                )
            );

        if (dist > 2000.0f)
        {
            continue;
        }


        if (wallCount >= MAX_WALL)
        {
            break;
        }

      
        VECTOR pos =
            wall->GetPos();


        VECTOR axisX =
            wall->GetAxisX();

        VECTOR axisZ =
            wall->GetAxisZ();

        VECTOR temp = axisX;
        axisX = axisZ;
        axisZ = temp;

        VECTOR half =
            wall->GetHalfSize();

        cb->wallPosSize[wallCount].x =
            pos.x;

        cb->wallPosSize[wallCount].y =
            pos.y;

        cb->wallPosSize[wallCount].z =
            pos.z;

        // X half
        cb->wallPosSize[wallCount].w =
            half.x;

        // axisX
        cb->wallAxisX[wallCount].x =
            axisX.x;

        cb->wallAxisX[wallCount].y =
            axisX.y;

        cb->wallAxisX[wallCount].z =
            axisX.z;

        // Z half
        cb->wallAxisX[wallCount].w =
            half.z;

        // axisZ
        cb->wallAxisZ[wallCount].x =
            axisZ.x;

        cb->wallAxisZ[wallCount].y =
            axisZ.y;

        cb->wallAxisZ[wallCount].z =
            axisZ.z;


        // 高さ
        cb->wallAxisZ[wallCount].w =
            half.y;

        wallCount++;

#ifdef _DEBUG
#else

        wall->DrawDebug(
            GetColor(255, 0, 0)
        );

#endif
    }

    cb->wallCount.x = (float)wallCount;

    cb->lightCount.x = (float)count;
    cb->lightCount.y = 0.0f;
    cb->lightCount.z = 0.0f;
    cb->lightCount.w = 0.0f;

    UpdateShaderConstantBuffer(
        constBufferHandle_
    );

    SetShaderConstantBuffer(
        constBufferHandle_,
        DX_SHADERTYPE_PIXEL,
        4
    );
}

void LightManager::AddWallBlocker(
    const LightBlocker* wall
)
{
    if (wall == nullptr)
    {
        return;
    }

    wallBlockers_.push_back(wall);
}
void LightManager::ClearWallBlockers(void)
{
    wallBlockers_.clear();
}

void LightManager::SetViewPoint(VECTOR viewPoint)
{
    viewPoint_ = viewPoint;
}


bool LightManager::IsBlockedByWall(
    const ShaderLight& light
) const
{
    return false;
}

void LightManager::WriteLightToCB(
    LightCB* cb,
    int index,
    const ShaderLight& light)
{
    cb->lightPosRange[index].x = light.pos.x;
    cb->lightPosRange[index].y = light.pos.y;
    cb->lightPosRange[index].z = light.pos.z;
    cb->lightPosRange[index].w = light.range;

    cb->lightColor[index].x = light.color.x;
    cb->lightColor[index].y = light.color.y;
    cb->lightColor[index].z = light.color.z;
    cb->lightColor[index].w = 1.0f;

    cb->lightDirInner[index].x = light.dir.x;
    cb->lightDirInner[index].y = light.dir.y;
    cb->lightDirInner[index].z = light.dir.z;
    cb->lightDirInner[index].w = light.innerCos;

    cb->lightOuter[index].x = light.outerCos;
    cb->lightOuter[index].y = 0.0f;
    cb->lightOuter[index].z = 0.0f;
    cb->lightOuter[index].w = 0.0f;
}