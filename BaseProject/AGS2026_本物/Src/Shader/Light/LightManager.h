#pragma once

#include <vector>
#include <DxLib.h>
class LightBlocker;

struct ShaderLight
{
    VECTOR pos;
    VECTOR color;
    VECTOR dir;

    float range;

    float innerCos;
    float outerCos;

    bool isWallBlocked;

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
        float outerAngleRad,
        bool isWallBlocked
    );

    void Clear();

    void SendToShader();

    // Playerâ˘íÜìdìîóp
    void SetFlashLight(
        VECTOR pos,
        VECTOR color,
        VECTOR dir,
        float range,
        float innerAngleRad,
        float outerAngleRad
    );

    void DisableFlashLight();


    void AddWallBlocker(
        const LightBlocker* wall
    );

    void ClearWallBlockers(void);

    void SetViewPoint(VECTOR viewPoint);


private:


    static const int MAX_LIGHT = 100;
    static const int MAX_WALL = 64;


    struct LightCB
    {
        DxLib::FLOAT4 lightCount;

        DxLib::FLOAT4 lightPosRange[MAX_LIGHT];

        DxLib::FLOAT4 lightColor[MAX_LIGHT];

        DxLib::FLOAT4 lightDirInner[MAX_LIGHT];

        DxLib::FLOAT4 lightOuter[MAX_LIGHT];


        // =====================
        // ï«èÓïÒí«â¡
        // =====================

        DxLib::FLOAT4 wallPosSize[MAX_WALL];

        // xyz = axisX
        DxLib::FLOAT4 wallAxisX[MAX_WALL];

        // xyz = axisZ
        DxLib::FLOAT4 wallAxisZ[MAX_WALL];

        DxLib::FLOAT4 wallCount;

    };

private:

    std::vector<ShaderLight> lights_;

    bool useFlashLight_ = false;

    ShaderLight flashLight_;

    int constBufferHandle_ = -1;

    std::vector<const LightBlocker*> wallBlockers_;   
    VECTOR viewPoint_;

    bool IsBlockedByWall(const ShaderLight& light) const;

    void WriteLightToCB(
        LightCB* cb,
        int index,
        const ShaderLight& light);

};