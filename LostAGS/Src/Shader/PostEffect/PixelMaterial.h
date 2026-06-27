#pragma once
#include <DxLib.h>
#include <string>
#include <vector>

class PixelMaterial
{
public:

    PixelMaterial();
    ~PixelMaterial();

    void Load(std::string shaderFileName, int constBufFloat4Size);

    void Update();

    void SetParam(int index, FLOAT4 value);

public:

    // シェーダハンドル
    int shader_;

    // 定数バッファハンドル
    int constBuf_;

    // 定数バッファサイズ
    int constBufFloat4Size_;

    // 定数バッファ配列
    std::vector<FLOAT4> constBufs_;

    // テクスチャ
    std::vector<int> textures_;

    // テクスチャアドレス
    int texAddress_;
};