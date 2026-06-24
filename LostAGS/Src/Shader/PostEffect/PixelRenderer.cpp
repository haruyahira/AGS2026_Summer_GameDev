#include "PixelRenderer.h"

PixelRenderer::PixelRenderer(
    PixelMaterial& material,
    int width,
    int height)
    :
    material_(material)
{
    // スクリーン生成
    screenHandle_ =
        MakeScreen(width, height, true);
}


PixelRenderer::~PixelRenderer()
{
    DeleteGraph(screenHandle_);
}


void PixelRenderer::MakeSquareVertex(
    Vector2 pos,
    Vector2 size)
{
    pos_ = pos;
    size_ = size;

    int cnt = 0;
    float sX = static_cast<float>(pos.x);
    float sY = static_cast<float>(pos.y);
    float eX = static_cast<float>(pos.x + size.x);
    float eY = static_cast<float>(pos.y + size.y);

    //４頂点の初期化
    for (int i = 0; i < NUM_VERTEX; i++)
    {
        vertexs_[i].rhw = 1.0f;
        vertexs_[i].dif = GetColorU8(255, 255, 255, 255);
        vertexs_[i].spc = GetColorU8(255, 255, 255, 255);
        vertexs_[i].su = 0.0f;
        vertexs_[i].sv = 0.0f;
    }

    // 左上
    vertexs_[cnt].pos = VGet(sX, sY, 0.0f);
    vertexs_[cnt].u = 0.0f;
    vertexs_[cnt].v = 0.0f;
    cnt++;

    // 右上
    vertexs_[cnt].pos = VGet(eX, sY, 0.0f);
    vertexs_[cnt].u = 1.0f;
    vertexs_[cnt].v = 0.0f;
    cnt++;

    // 右下
    vertexs_[cnt].pos = VGet(eX, eY, 0.0f);
    vertexs_[cnt].u = 1.0f;
    vertexs_[cnt].v = 1.0f;
    cnt++;

    // 左下
    vertexs_[cnt].pos = VGet(sX, eY, 0.0f);
    vertexs_[cnt].u = 0.0f;
    vertexs_[cnt].v = 1.0f;

    // 頂点インデックス
    cnt = 0;

    indexes_[cnt++] = 0;
    indexes_[cnt++] = 1;
    indexes_[cnt++] = 3;

    indexes_[cnt++] = 1;
    indexes_[cnt++] = 2;
    indexes_[cnt++] = 3;
}

void PixelRenderer::Draw(int textureHandle)
{

    // 描画先変更
    SetDrawScreen(screenHandle_);

    // クリア
    ClearDrawScreen();

    // オリジナルシェーダON
    MV1SetUseOrigShader(true);

    // シェーダ設定
    SetUsePixelShader(material_.shader_);

    // テクスチャ設定
    SetUseTextureToShader(0, textureHandle);


    // 定数バッファをピクセルシェーダー用定数バッファレジスタにセット
    SetShaderConstantBuffer(
        material_.constBuf_, DX_SHADERTYPE_PIXEL, CONSTANT_BUF_SLOT_BEGIN_PS);

    // 描画
    DrawPolygonIndexed2DToShader(
        vertexs_,
        NUM_VERTEX,
        indexes_,
        NUM_POLYGON);


    // 後始末
    // テクスチャ解除
    SetUseTextureToShader(0, -1);

    // ピクセルシェーダ解除
    SetUsePixelShader(-1);

    // オリジナルシェーダ設定(OFF)
    MV1SetUseOrigShader(false);

    // メインに戻す
    SetDrawScreen(textureHandle);
    DrawGraph(0, 0, screenHandle_, false);
}
