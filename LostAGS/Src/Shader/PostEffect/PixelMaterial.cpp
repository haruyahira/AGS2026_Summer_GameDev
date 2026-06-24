#include "PixelMaterial.h"
#include <assert.h>

PixelMaterial::PixelMaterial()
{
    shader_ = -1;
    constBuf_ = -1;

    constBufFloat4Size_ = 0;

    texAddress_ = DX_TEXADDRESS_WRAP;
}

PixelMaterial::~PixelMaterial()
{
    DeleteShader(shader_);

    DeleteShaderConstantBuffer(constBuf_);
}

void PixelMaterial::Load(
    std::string shaderFileName,
    int constBufFloat4Size)
{
    // ピクセルシェーダのロード
    shader_ = LoadPixelShader(shaderFileName.c_str());

	assert(shader_ != -1);

    // FLOAT4数
    constBufFloat4Size_ = constBufFloat4Size;

    // 定数バッファ作成
    constBuf_ =
        CreateShaderConstantBuffer(
            sizeof(FLOAT4) * constBufFloat4Size_);

    // CPU側配列確保
    constBufs_.resize(constBufFloat4Size_);
}

void PixelMaterial::Update()
{
    // 定数バッファ
    FLOAT4* constBufsPtr =
        (FLOAT4*)GetBufferShaderConstantBuffer(constBuf_);

    for (int i = 0; i < constBufFloat4Size_; i++)
    {
        constBufsPtr[i] = constBufs_[i];
    }

    // 定数バッファを更新して書き込んだ内容を反映する
    UpdateShaderConstantBuffer(constBuf_);
}

void PixelMaterial::SetParam(int index, FLOAT4 value)
{

    if (index < 0 || index >= constBufFloat4Size_)
    {
        return;
    }

    constBufs_[index] = value;

}
