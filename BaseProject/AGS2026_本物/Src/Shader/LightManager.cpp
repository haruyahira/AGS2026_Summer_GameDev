#include "LightManager.h"
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
    float outerAngleRad)
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

    // 天井ライト
    for (int i = 0; i < (int)lights_.size(); i++)
    {
        if (count >= MAX_LIGHT)
        {
            break;
        }

        const ShaderLight& light = lights_[i];

        cb->lightPosRange[count].x = light.pos.x;
        cb->lightPosRange[count].y = light.pos.y;
        cb->lightPosRange[count].z = light.pos.z;
        cb->lightPosRange[count].w = light.range;

        cb->lightColor[count].x = light.color.x;
        cb->lightColor[count].y = light.color.y;
        cb->lightColor[count].z = light.color.z;
        cb->lightColor[count].w = 1.0f;

        cb->lightDirInner[count].x = light.dir.x;
        cb->lightDirInner[count].y = light.dir.y;
        cb->lightDirInner[count].z = light.dir.z;
        cb->lightDirInner[count].w = light.innerCos;

        cb->lightOuter[count].x = light.outerCos;
        cb->lightOuter[count].y = 0.0f;
        cb->lightOuter[count].z = 0.0f;
        cb->lightOuter[count].w = 0.0f;

        count++;
    }

    // Player懐中電灯
    if (useFlashLight_ && count < MAX_LIGHT)
    {
        const ShaderLight& light = flashLight_;

        cb->lightPosRange[count].x = light.pos.x;
        cb->lightPosRange[count].y = light.pos.y;
        cb->lightPosRange[count].z = light.pos.z;
        cb->lightPosRange[count].w = light.range;

        cb->lightColor[count].x = light.color.x;
        cb->lightColor[count].y = light.color.y;
        cb->lightColor[count].z = light.color.z;
        cb->lightColor[count].w = 1.0f;

        cb->lightDirInner[count].x = light.dir.x;
        cb->lightDirInner[count].y = light.dir.y;
        cb->lightDirInner[count].z = light.dir.z;
        cb->lightDirInner[count].w = light.innerCos;

        cb->lightOuter[count].x = light.outerCos;
        cb->lightOuter[count].y = 0.0f;
        cb->lightOuter[count].z = 0.0f;
        cb->lightOuter[count].w = 0.0f;

        count++;
    }

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