#pragma once

#include <string>
#include "LightMaterial.h"

class LightManager;

class LightRenderer
{
public:
    LightRenderer();
    ~LightRenderer();

    bool Init(
        const std::string& vertexShaderFileName,
        const std::string& pixelShaderFileName);

    void Release();

    void Begin(LightManager& lightManager);
    void End();

    void DrawModel(int modelHandle);

private:
    LightMaterial material_;
};