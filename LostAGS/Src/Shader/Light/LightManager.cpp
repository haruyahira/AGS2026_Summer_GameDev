
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
    const VECTOR& pos,
    const VECTOR& color,
    const VECTOR& dir,
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
    light.isWallBlocked = isWallBlocked;

    lights_.push_back(light);
}

void LightManager::AddLight(
    const VECTOR& pos,
    const VECTOR& color,
    const VECTOR& dir,
    float range,
    float innerAngleRad,
    float outerAngleRad,
    const std::vector<LightBlocker*>& blockers)
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

    // 壁リストが空なら遮蔽なし
    light.isWallBlocked = !blockers.empty();

    for (auto blocker : blockers)
    {
        if (blocker != nullptr)
        {
            light.blockers.push_back(blocker);
        }
    }

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
    flashLight_.isWallBlocked = true;

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
    // 通常ライト
    // =========================
    for (int i = 0; i < (int)lights_.size(); i++)
    {
        if (count >= MAX_LIGHT)
        {
            break;
        }

        const ShaderLight& light = lights_[i];

        WriteLightToCB(cb, count, light);
        count++;
    }

    // =========================
    // シェーダーへ送る壁だけを集める
    // =========================
    std::vector<const LightBlocker*> sendWalls;

    // =========================
    // 懐中電灯用の壁
    // =========================
    if (useFlashLight_ && flashLight_.isWallBlocked)
    {
        for (const LightBlocker* wall : wallBlockers_)
        {
            if (wall == nullptr)
            {
                continue;
            }

            if (!IsWallNeededByLight(wall, flashLight_))
            {
                continue;
            }

            bool alreadyExists = false;

            for (const LightBlocker* addedWall : sendWalls)
            {
                if (addedWall == wall)
                {
                    alreadyExists = true;
                    break;
                }
            }

            if (!alreadyExists)
            {
                sendWalls.push_back(wall);
            }
        }
    }

    // =========================
    // 通常ライト用の壁
    // 各ライトが持っている blockers だけを見る
    // =========================
    for (const ShaderLight& light : lights_)
    {
        if (!light.isWallBlocked)
        {
            continue;
        }

        for (const LightBlocker* wall : light.blockers)
        {
            if (wall == nullptr)
            {
                continue;
            }

            if (!IsWallNeededByLight(wall, light))
            {
                continue;
            }

            bool alreadyExists = false;

            for (const LightBlocker* addedWall : sendWalls)
            {
                if (addedWall == wall)
                {
                    alreadyExists = true;
                    break;
                }
            }

            if (!alreadyExists)
            {
                sendWalls.push_back(wall);
            }
        }
    }

    // =========================
    // 壁情報をシェーダーへ送る
    // =========================
    int wallCount = 0;

    for (const LightBlocker* wall : sendWalls)
    {
        if (wall == nullptr)
        {
            continue;
        }

        if (wallCount >= MAX_WALL)
        {
            break;
        }

        VECTOR pos = wall->GetPos();

        VECTOR axisX = wall->GetAxisX();
        VECTOR axisZ = wall->GetAxisZ();

        // 今までの処理に合わせて入れ替え
        VECTOR temp = axisX;
        axisX = axisZ;
        axisZ = temp;

        VECTOR half = wall->GetHalfSize();

        cb->wallPosSize[wallCount].x = pos.x;
        cb->wallPosSize[wallCount].y = pos.y;
        cb->wallPosSize[wallCount].z = pos.z;
        cb->wallPosSize[wallCount].w = half.x;

        cb->wallAxisX[wallCount].x = axisX.x;
        cb->wallAxisX[wallCount].y = axisX.y;
        cb->wallAxisX[wallCount].z = axisX.z;
        cb->wallAxisX[wallCount].w = half.z;

        cb->wallAxisZ[wallCount].x = axisZ.x;
        cb->wallAxisZ[wallCount].y = axisZ.y;
        cb->wallAxisZ[wallCount].z = axisZ.z;
        cb->wallAxisZ[wallCount].w = half.y;

#ifdef _DEBUG
        wall->DrawDebug(GetColor(255, 0, 0));
#endif

        wallCount++;
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
    cb->lightOuter[index].y = light.isWallBlocked ? 1.0f : 0.0f;
    cb->lightOuter[index].z = 0.0f;
    cb->lightOuter[index].w = 0.0f;
}

bool LightManager::IsWallNeededByLight(
    const LightBlocker* wall,
    const ShaderLight& light
) const
{
    if (wall == nullptr)
    {
        return false;
    }

    if (!light.isWallBlocked)
    {
        return false;
    }

    VECTOR wallPos = wall->GetPos();

    float dx = wallPos.x - light.pos.x;
    float dz = wallPos.z - light.pos.z;

    VECTOR half = wall->GetHalfSize();

    // 壁の長さぶん少し余裕を持たせる
    float margin = 300.0f + half.x + half.z;

    float range = light.range + margin;
    float rangeSq = range * range;

    float distSq = dx * dx + dz * dz;

    return distSq <= rangeSq;
}

bool LightManager::IsWallNeededByAnyLight(
    const LightBlocker* wall
) const
{
    if (wall == nullptr)
    {
        return false;
    }

    // 懐中電灯
    if (useFlashLight_)
    {
        if (IsWallNeededByLight(wall, flashLight_))
        {
            return true;
        }
    }

    // 天井ライトなど
    for (const ShaderLight& light : lights_)
    {
        if (IsWallNeededByLight(wall, light))
        {
            return true;
        }
    }

    return false;
}