#include "PixelRenderer.h"
#include <assert.h>

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

    assert(screenHandle_ != -1);
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
    int oldScreen = GetDrawScreen();

    SetDrawScreen(screenHandle_);
    ClearDrawScreen();

    SetUseVertexShader(-1);
    SetUsePixelShader(-1);

    SetUseTextureToShader(0, -1);
    SetUseTextureToShader(1, -1);
    SetUseTextureToShader(2, -1);

    SetUseZBuffer3D(FALSE);
    SetWriteZBuffer3D(FALSE);
    SetUseBackCulling(FALSE);
    SetUseLighting(FALSE);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    MV1SetUseOrigShader(FALSE);

    SetUseTextureToShader(0, textureHandle);
    SetUsePixelShader(material_.shader_);

    SetShaderConstantBuffer(
        material_.constBuf_,
        DX_SHADERTYPE_PIXEL,
        CONSTANT_BUF_SLOT_BEGIN_PS
    );

    float x0 = static_cast<float>(pos_.x);
    float y0 = static_cast<float>(pos_.y);
    float x1 = static_cast<float>(pos_.x + size_.x);
    float y1 = static_cast<float>(pos_.y + size_.y);

    VERTEX2DSHADER v[6];

    for (int i = 0; i < 6; i++)
    {
        v[i].rhw = 1.0f;
        v[i].dif = GetColorU8(255, 255, 255, 255);
        v[i].spc = GetColorU8(0, 0, 0, 0);
    }

    v[0].pos = VGet(x0, y0, 0.0f);
    v[0].u = 0.0f;
    v[0].v = 0.0f;
    v[0].su = 0.0f;
    v[0].sv = 0.0f;

    v[1].pos = VGet(x0, y1, 0.0f);
    v[1].u = 0.0f;
    v[1].v = 1.0f;
    v[1].su = 0.0f;
    v[1].sv = 1.0f;

    v[2].pos = VGet(x1, y1, 0.0f);
    v[2].u = 1.0f;
    v[2].v = 1.0f;
    v[2].su = 1.0f;
    v[2].sv = 1.0f;

    v[3].pos = VGet(x0, y0, 0.0f);
    v[3].u = 0.0f;
    v[3].v = 0.0f;
    v[3].su = 0.0f;
    v[3].sv = 0.0f;

    v[4].pos = VGet(x1, y1, 0.0f);
    v[4].u = 1.0f;
    v[4].v = 1.0f;
    v[4].su = 1.0f;
    v[4].sv = 1.0f;

    v[5].pos = VGet(x1, y0, 0.0f);
    v[5].u = 1.0f;
    v[5].v = 0.0f;
    v[5].su = 1.0f;
    v[5].sv = 0.0f;

    DrawPrimitive2DToShader(
        v,
        6,
        DX_PRIMTYPE_TRIANGLELIST
    );

    SetUseTextureToShader(0, -1);
    SetUsePixelShader(-1);
    SetUseVertexShader(-1);

    SetDrawScreen(oldScreen);

    DrawGraph(0, 0, screenHandle_, FALSE);

    SetUseLighting(TRUE);
    SetUseBackCulling(TRUE);
    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}