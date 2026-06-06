#pragma once

#include "Shader.h"
#include "Material.h"
#include "LightManager.h"

class Renderer
{
public:

    Shader shader_;

    LightManager lightMng_;

public:

    void Init();

    void Begin();

    void End();

    void DrawModel(
        int modelHandle,
        const Material& mat
    );
};
