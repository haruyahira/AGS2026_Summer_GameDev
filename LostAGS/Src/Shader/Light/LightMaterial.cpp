#include "LightMaterial.h"

LightMaterial::LightMaterial()
{
    vertexShader_ = -1;
    pixelShader_ = -1;
}

LightMaterial::~LightMaterial()
{
    Release();
}

bool LightMaterial::Load(
    const std::string& vertexShaderFileName,
    const std::string& pixelShaderFileName)
{
    Release();

    vertexShader_ =
        LoadVertexShader(vertexShaderFileName.c_str());

    pixelShader_ =
        LoadPixelShader(pixelShaderFileName.c_str());

    if (vertexShader_ == -1 ||
        pixelShader_ == -1)
    {
        return false;
    }

    return true;
}

void LightMaterial::Release()
{
    if (vertexShader_ != -1)
    {
        DeleteShader(vertexShader_);
        vertexShader_ = -1;
    }

    if (pixelShader_ != -1)
    {
        DeleteShader(pixelShader_);
        pixelShader_ = -1;
    }
}

void LightMaterial::Begin()
{
    SetUseVertexShader(vertexShader_);
    SetUsePixelShader(pixelShader_);
}

void LightMaterial::End()
{
    SetUseVertexShader(-1);
    SetUsePixelShader(-1);
}

bool LightMaterial::IsValid() const
{
    return vertexShader_ != -1 &&
        pixelShader_ != -1;
}