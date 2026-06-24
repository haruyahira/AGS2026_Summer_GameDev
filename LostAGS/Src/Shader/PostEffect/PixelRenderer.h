#pragma once
#include <DxLib.h>
#include "../../Utility/AsoUtility.h"
#include "PixelMaterial.h"

class PixelRenderer
{
public:
    // コンストラクタ
    PixelRenderer(
        PixelMaterial& material,
        int width,
        int height);

    ~PixelRenderer();

    void MakeSquareVertex(
        Vector2 pos,
        Vector2 size);

    void Draw(int textureHandle);

    int GetScreenHandle() const
    {
        return screenHandle_;
    }

private:
    // ピクセルシェーダ用オリジナル定数バッファの使用開始スロット
    static constexpr int CONSTANT_BUF_SLOT_BEGIN_PS = 4;

    // 頂点数
    static constexpr int NUM_VERTEX = 4;

    // 頂点インデックス数
    static constexpr int NUM_VERTEX_IDX = 6;

    // ポリゴン数
    static constexpr int NUM_POLYGON = 2;

    // 座標
    Vector2 pos_;

    // 描画サイズ
    Vector2 size_;

    // 頂点
    VERTEX2DSHADER vertexs_[NUM_VERTEX];

    // 頂点インデックス
    WORD indexes_[NUM_VERTEX_IDX];

    // ピクセルマテリアル
    PixelMaterial& material_;

    // ポストエフェクト用スクリーン
    int screenHandle_;
};