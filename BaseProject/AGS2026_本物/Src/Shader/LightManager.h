#pragma once

#include <vector>
#include <DxLib.h>

struct ShaderLight
{
    VECTOR pos;
    VECTOR color;
    VECTOR dir;

    float range;

    float innerCos;
    float outerCos;
};

class LightManager
{
public:

    void Init();

    void Release();

    void AddLight(
        VECTOR pos,
        VECTOR color,
        VECTOR dir,
        float range,
        float innerAngleRad,
        float outerAngleRad
    );

    void Clear();

    void SendToShader();

    // Player‰ù’†“d“”—p
    void SetFlashLight(
        VECTOR pos,
        VECTOR color,
        VECTOR dir,
        float range,
        float innerAngleRad,
        float outerAngleRad
    );

    void DisableFlashLight();

private:

    static const int MAX_LIGHT = 100;

    struct LightCB
    {
        DxLib::FLOAT4 lightCount;

        DxLib::FLOAT4 lightPosRange[MAX_LIGHT];

        DxLib::FLOAT4 lightColor[MAX_LIGHT];

        DxLib::FLOAT4 lightDirInner[MAX_LIGHT];

        DxLib::FLOAT4 lightOuter[MAX_LIGHT];
    };

private:

    std::vector<ShaderLight> lights_;

    bool useFlashLight_ = false;

    ShaderLight flashLight_;

    int constBufferHandle_ = -1;
};