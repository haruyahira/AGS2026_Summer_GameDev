#include "LightEffect.h"

LightEffect::LightEffect()
{
}

LightEffect::~LightEffect()
{
    Release();
}

bool LightEffect::Init(
    const std::string& vertexShaderFileName,
    const std::string& pixelShaderFileName)
{
    lightManager_.Init();

    return renderer_.Init(
        vertexShaderFileName,
        pixelShaderFileName);
}

void LightEffect::Release()
{
    renderer_.Release();
    lightManager_.Release();
}

void LightEffect::Begin()
{
    renderer_.Begin(lightManager_);
}

void LightEffect::End()
{
    renderer_.End();
}

LightManager& LightEffect::GetLightManager()
{
    return lightManager_;
}