#pragma once

#include <DxLib.h>

class RimLightEffect
{
public:

    RimLightEffect();
    ~RimLightEffect();

    bool Init(const char* psPath);
    void Release();

    void Begin(
        const VECTOR& color,
        float power,
        float intensity,
        float alpha
    );

    void End();

private:

    struct RimLightCB
    {
        DxLib::FLOAT4 rimColorPower;
        DxLib::FLOAT4 rimParam;
        DxLib::FLOAT4 cameraPos;
    };

private:

    int pixelShader_;
    int constBuffer_;
};
