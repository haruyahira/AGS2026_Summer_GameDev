#pragma once

#include <string>
#include "LightManager.h"
#include "LightRenderer.h"

class LightEffect
{
public:
    LightEffect();
    ~LightEffect();

    bool Init(
        const std::string& vertexShaderFileName,
        const std::string& pixelShaderFileName);

    void Release();

    void Begin();
    void End();

    LightManager& GetLightManager();

private:
    LightManager lightManager_;
    LightRenderer renderer_;
};