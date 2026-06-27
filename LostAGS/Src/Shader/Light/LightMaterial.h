#pragma once

#include <string>
#include <DxLib.h>

class LightMaterial
{
public:
    LightMaterial();
    ~LightMaterial();

    bool Load(
        const std::string& vertexShaderFileName,
        const std::string& pixelShaderFileName);

    void Release();

    void Begin();
    void End();

    bool IsValid() const;

private:
    int vertexShader_;
    int pixelShader_;
};