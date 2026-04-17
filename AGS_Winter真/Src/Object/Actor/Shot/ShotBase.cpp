#include "ShotBase.h"
#include <cmath>

ShotBase::ShotBase(TYPE type, int baseModelId)
    : type_(type)
    , modelId_(MV1DuplicateModel(baseModelId))
    , dir_(VGet(0.0f, 0.0f, 1.0f))
    , scales_(VGet(1.0f, 1.0f, 1.0f))
    , pos_(VGet(0.0f, 0.0f, 0.0f))
    , collider_(nullptr)
    , speed_(0.0f)
    , isAlive_(false)
    , cntAlive_(0)
    , collisionRadius_(radius_)
{
}

ShotBase::~ShotBase(void)
{
    if (collider_)
    {
        delete collider_;
        collider_ = nullptr;
    }
}

void ShotBase::CreateShot(VECTOR pos, VECTOR dir)
{
    // 弾の発射位置を設定
    pos_ = pos;
    // 弾の発射方向
    dir_ = dir;

    shotPos.clear();
    shotPos.push_back(pos_);

    // パラメータ設定（速度, スケール, 生存時間, 当たり半径など）
    SetParam();

    // 大きさ
    MV1SetScale(modelId_, scales_);

    // 位置
    MV1SetPosition(modelId_, pos_);

    // 自己発光
    MV1SetMaterialEmiColor(modelId_, 0, COLOR_EMI_DEFAULT);

    // 既存コライダ削除
    if (collider_)
    {
        delete collider_;
        collider_ = nullptr;
    }

    // ★ 弾用 球コライダ（Transform なし）
    collider_ = new ColliderSphere(
        ColliderBase::TAG::SHOT,   // プレイヤーの弾扱い
        nullptr,                   // Transform 追従なし
        VGet(0, 0, 0),             // ローカルは使わないので 0
        collisionRadius_                    // 当たり判定半径（必要なら SetParam で上書き）
    );

    // 初期位置を反映
    collider_->SetPosition(pos_);

    // 生存フラグON
    isAlive_ = true;
}

void ShotBase::Update(void)
{
    if (!IsAlive())
    {
        return;
    }

    // 移動
    Move();

    // 生存カウンタの減少
    ReduceCntAlive();

    if (collider_ && !shotPos.empty())
    {
        // 弾の座標をそのままコライダに渡す
        collider_->SetPosition(shotPos[0]);
    }
}

void ShotBase::Draw(void)
{
    if (!IsAlive())
    {
        // 生存していなければ処理中断
        return;
    }

    DrawModel();
}

void ShotBase::Release(void)
{
    MV1DeleteModel(modelId_);
    shotPos.clear();

    if (collider_)
    {
        delete collider_;
        collider_ = nullptr;
    }
}

void ShotBase::ReduceCntAlive(void)
{
    cntAlive_--;
    if (cntAlive_ < 0)
    {
        shotPos.clear();
        isAlive_ = false;
    }
}

void ShotBase::Move(void)
{
    if (shotPos.empty())
        shotPos.push_back(pos_);

    // 移動量の計算(方向×スピード)
    VECTOR movePow = VScale(dir_, speed_);

    // 移動処理
    pos_ = VAdd(pos_, movePow);
    shotPos[0] = pos_;

    // 位置の設定
    MV1SetPosition(modelId_, pos_);
}

void ShotBase::DrawModel(void)
{
    if (shotPos.empty()) return;

    // 角度計算（Y軸回転）
    float angleY = atan2f(dir_.x, dir_.z);

    // 回転を適用
    MV1SetRotationXYZ(modelId_, VGet(0, angleY, 0));
    // 位置を適用
    MV1SetPosition(modelId_, shotPos[0]);

    // 描画
    MV1DrawModel(modelId_);
}
