#include "LightRenderer.h"
#include "LightManager.h"
#include <DxLib.h>

LightRenderer::LightRenderer()
{
}

LightRenderer::~LightRenderer()
{
    Release();
}

bool LightRenderer::Init(
    const std::string& vertexShaderFileName,
    const std::string& pixelShaderFileName)
{
    return material_.Load(
        vertexShaderFileName,
        pixelShaderFileName);
}

void LightRenderer::Release()
{
    material_.Release();
}

void LightRenderer::Begin(LightManager& lightManager)
{
    MV1SetUseOrigShader(TRUE);

    material_.Begin();

    lightManager.SendToShader();
}

void LightRenderer::End()
{
    material_.End();

    MV1SetUseOrigShader(FALSE);
}

void LightRenderer::DrawModel(int modelHandle)
{
    MV1DrawModel(modelHandle);
}