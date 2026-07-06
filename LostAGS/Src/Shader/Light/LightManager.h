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

    bool isWallBlocked = false;

    // このライトが判定する壁だけを持つ
    std::vector<const LightBlocker*> blockers;
};

class LightManager
{
public:

    void Init();

    void Release();

    // 従来用：壁遮蔽ON/OFFだけ指定する版
    void AddLight(
        const VECTOR& pos,
        const VECTOR& color,
        const VECTOR& dir,
        float range,
        float innerAngleRad,
        float outerAngleRad,
        bool isWallBlocked
    );

    // 新方式：このライト専用の壁リストを渡す版
    void AddLight(
        const VECTOR& pos,
        const VECTOR& color,
        const VECTOR& dir,
        float range,
        float innerAngleRad,
        float outerAngleRad,
        const std::vector<LightBlocker*>& blockers
    );

    void Clear();

    void SendToShader();

    // Player懐中電灯用
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

        DxLib::FLOAT4 wallPosSize[MAX_WALL];
        DxLib::FLOAT4 wallAxisX[MAX_WALL];
        DxLib::FLOAT4 wallAxisZ[MAX_WALL];

        DxLib::FLOAT4 wallCount;
    };

private:

    std::vector<ShaderLight> lights_;

    bool useFlashLight_ = false;
    ShaderLight flashLight_;

    int constBufferHandle_ = -1;

    // 懐中電灯用の全壁候補
    std::vector<const LightBlocker*> wallBlockers_;

    VECTOR viewPoint_;

    bool IsBlockedByWall(const ShaderLight& light) const;

    void WriteLightToCB(
        LightCB* cb,
        int index,
        const ShaderLight& light
    );

    bool IsWallNeededByLight(
        const LightBlocker* wall,
        const ShaderLight& light
    ) const;

    bool IsWallNeededByAnyLight(
        const LightBlocker* wall
    ) const;
};