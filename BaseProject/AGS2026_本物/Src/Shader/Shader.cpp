#include "Shader.h"
#include <DxLib.h>

void Shader::Load(
    const char* vsPath,
    const char* psPath)
{
    vsHandle_ =
        LoadVertexShader(vsPath);

    psHandle_ =
        LoadPixelShader(psPath);
}

void Shader::Begin()
{
    SetUseVertexShader(vsHandle_);

    SetUsePixelShader(psHandle_);
}

void Shader::End()
{
    SetUseVertexShader(-1);

    SetUsePixelShader(-1);
}
