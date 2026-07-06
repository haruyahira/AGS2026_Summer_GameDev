#include "RimLightEffect.h"

RimLightEffect::RimLightEffect()
{
    pixelShader_ = -1;
    constBuffer_ = -1;
}

RimLightEffect::~RimLightEffect()
{
    Release();
}

bool RimLightEffect::Init(const char* psPath)
{
    pixelShader_ = LoadPixelShader(psPath);

    if (pixelShader_ < 0)
    {
        return false;
    }

    constBuffer_ =
        CreateShaderConstantBuffer(
            sizeof(RimLightCB)
        );

    if (constBuffer_ < 0)
    {
        return false;
    }

    return true;
}

void RimLightEffect::Release()
{
    if (pixelShader_ != -1)
    {
        DeleteShader(pixelShader_);
        pixelShader_ = -1;
    }

    if (constBuffer_ != -1)
    {
        DeleteShaderConstantBuffer(constBuffer_);
        constBuffer_ = -1;
    }
}

void RimLightEffect::Begin(
    const VECTOR& color,
    float power,
    float intensity,
    float alpha
)
{
    if (pixelShader_ < 0 || constBuffer_ < 0)
    {
        return;
    }

    RimLightCB* cb =
        (RimLightCB*)GetBufferShaderConstantBuffer(
            constBuffer_
        );

    if (cb == nullptr)
    {
        return;
    }

    cb->rimColorPower.x = color.x;
    cb->rimColorPower.y = color.y;
    cb->rimColorPower.z = color.z;
    cb->rimColorPower.w = power;

    cb->rimParam.x = intensity;
    cb->rimParam.y = alpha;
    cb->rimParam.z = 0.0f;
    cb->rimParam.w = 0.0f;

    VECTOR cameraPos = GetCameraPosition();

    cb->cameraPos.x = cameraPos.x;
    cb->cameraPos.y = cameraPos.y;
    cb->cameraPos.z = cameraPos.z;
    cb->cameraPos.w = 0.0f;

    UpdateShaderConstantBuffer(constBuffer_);

    SetShaderConstantBuffer(
        constBuffer_,
        DX_SHADERTYPE_PIXEL,
        5
    );

    SetUsePixelShader(pixelShader_);

    SetUseLighting(FALSE);

    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(FALSE);

    SetUseBackCulling(TRUE);

    SetDrawBlendMode(DX_BLENDMODE_ADD, 160);
}

void RimLightEffect::End()
{
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    SetUseBackCulling(TRUE);
    SetWriteZBuffer3D(TRUE);
    SetUseZBuffer3D(TRUE);
    SetUseLighting(TRUE);

    SetUsePixelShader(-1);
}