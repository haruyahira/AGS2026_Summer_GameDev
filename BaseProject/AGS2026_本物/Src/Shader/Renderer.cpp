#include "Renderer.h"
#include <DxLib.h>

void Renderer::Init()
{
    shader_.Load(
        "VertexShader.vso",
        "PixelShader.pso"
    );
}

void Renderer::Begin()
{
    shader_.Begin();

    lightMng_.SendToShader();
}

void Renderer::End()
{
    shader_.End();
}

void Renderer::DrawModel(
    int modelHandle,
    const Material& mat)
{
    MV1DrawModel(modelHandle);
}