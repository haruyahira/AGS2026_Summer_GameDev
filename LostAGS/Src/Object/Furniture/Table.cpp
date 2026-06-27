#include "Table.h"
#include "../../Manager/InputManager.h"

// ---------------------------------------------------------
// ローカル座標のオフセットを、
// 親Transformの回転・スケールを反映したワールド方向のオフセットに変換する
// ---------------------------------------------------------
static VECTOR GetRotatedOffset(
    const VECTOR& local,
    const VECTOR& scaleRate,
    const Quaternion& rot)
{
    VECTOR right = rot.GetRight();
    VECTOR up = rot.GetUp();
    VECTOR forward = rot.GetForward();

    VECTOR result = VGet(0.0f, 0.0f, 0.0f);

    result = VAdd(result, VScale(right, local.x * scaleRate.x));
    result = VAdd(result, VScale(up, local.y * scaleRate.y));
    result = VAdd(result, VScale(forward, local.z * scaleRate.z));

    return result;
}

Table::Table(const Transform* trans)
    : Furniture(NAME::TABLE, trans)
{
}

// ---------------------------------------------------------
// 天板用
// ---------------------------------------------------------
static float plateH = 77.0f;        // 天板上面の高さ
static float plateW = 240.0f;       // 天板の幅
static float plateD = 150.0f;       // 天板の奥行
static float plateT = 5.0f;         // 天板の厚み
static float plateSlideX = 8.0f;    // 天板のX位置補正
static float plateSlideZ = 10.2f;   // 天板のZ位置補正

// ---------------------------------------------------------
// 脚用
// ---------------------------------------------------------
static float legH = 150.0f;         // 脚の高さ
static float legW = 4.0f;           // 脚の太さ
static float legOX = 58.0f;         // 脚の左右位置
static float legOZ = 35.0f;         // 脚の前後位置
static float legSlideX = 7.0f;      // 脚全体のX補正
static float legSlideZ = 11.0f;     // 脚全体のZ補正
static float legSlideY = -40.0f;    // 脚全体のY補正

void Table::Init(void)
{
    isHideSpot_ = true;

    // Table は BoxCollider を使わない
    colliders_.clear();

    // OBB を作り直す
    obbColliders_.clear();

    VECTOR pos = trans_.GetPos();
    VECTOR scl = trans_.GetScale();
    Quaternion rot = trans_.GetRot();

    // -----------------------------------------------------
    // 今の調整値は Stage 側の scale 0.5 を基準にしている想定
    // なので、現在スケール / 基準スケール で補正する
    // -----------------------------------------------------
    const VECTOR BASE_SCALE = VGet(0.5f, 0.5f, 0.5f);

    VECTOR scaleRate =
        VGet(
            scl.x / BASE_SCALE.x,
            scl.y / BASE_SCALE.y,
            scl.z / BASE_SCALE.z
        );

    VECTOR axisX = rot.GetRight();
    VECTOR axisY = rot.GetUp();
    VECTOR axisZ = rot.GetForward();

    // -----------------------------------------------------
    // 重要：
    // 以前の BoxCollider と同じ大きさに近づけるための倍率
    // BoxCollider 側でさらに /2 されていたので 0.25f にする
    // -----------------------------------------------------
    const float OBB_SIZE_RATE = 0.25f;

    // -----------------------------------------------------
    // 天板 OBB
    // -----------------------------------------------------
    float plateCenterY = plateH - plateT * 0.5f;

    VECTOR plateLocalCenter =
        VGet(
            plateSlideX,
            plateCenterY,
            plateSlideZ
        );

    VECTOR plateWorldCenter =
        VAdd(
            pos,
            GetRotatedOffset(
                plateLocalCenter,
                scaleRate,
                rot
            )
        );

    VECTOR plateHalfSize =
        VGet(
            plateW * OBB_SIZE_RATE * scaleRate.x,
            plateT * OBB_SIZE_RATE * scaleRate.y,
            plateD * OBB_SIZE_RATE * scaleRate.z
        );

    obbColliders_.push_back(
        OBBCollider(
            plateWorldCenter,
            plateHalfSize,
            axisX,
            axisY,
            axisZ
        )
    );

    // -----------------------------------------------------
    // 脚 OBB 4本
    // -----------------------------------------------------
    float legCenterY = legH * 0.5f;

    float xs[2] = { legOX, -legOX };
    float zs[2] = { legOZ, -legOZ };

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            VECTOR legLocalCenter =
                VGet(
                    xs[i] + legSlideX,
                    legCenterY + legSlideY,
                    zs[j] + legSlideZ
                );

            VECTOR legWorldCenter =
                VAdd(
                    pos,
                    GetRotatedOffset(
                        legLocalCenter,
                        scaleRate,
                        rot
                    )
                );

            VECTOR legHalfSize =
                VGet(
                    legW * OBB_SIZE_RATE * scaleRate.x,
                    legH * OBB_SIZE_RATE * scaleRate.y,
                    legW * OBB_SIZE_RATE * scaleRate.z
                );

            obbColliders_.push_back(
                OBBCollider(
                    legWorldCenter,
                    legHalfSize,
                    axisX,
                    axisY,
                    axisZ
                )
            );
        }
    }
}

void Table::Update(void)
{
#ifdef _DEBUG
    auto& ins = InputManager::GetInstance();

    bool changed = false;

    // -----------------------------------------------------
    // Pキー：天板調整
    // -----------------------------------------------------
    if (CheckHitKey(KEY_INPUT_P))
    {
        // Shift + P：天板の位置調整
        if (CheckHitKey(KEY_INPUT_LSHIFT))
        {
            if (ins.IsTrgDown(KEY_INPUT_RIGHT))
            {
                plateSlideX += 1.0f;
                changed = true;
            }

            if (ins.IsTrgDown(KEY_INPUT_LEFT))
            {
                plateSlideX -= 1.0f;
                changed = true;
            }

            if (ins.IsTrgDown(KEY_INPUT_UP))
            {
                plateSlideZ += 1.0f;
                changed = true;
            }

            if (ins.IsTrgDown(KEY_INPUT_DOWN))
            {
                plateSlideZ -= 1.0f;
                changed = true;
            }
        }
        // Pのみ：天板の高さ・幅調整
        else
        {
            if (ins.IsTrgDown(KEY_INPUT_UP))
            {
                plateH += 1.0f;
                changed = true;
            }

            if (ins.IsTrgDown(KEY_INPUT_DOWN))
            {
                plateH -= 1.0f;
                changed = true;
            }

            if (ins.IsTrgDown(KEY_INPUT_RIGHT))
            {
                plateW += 1.0f;
                changed = true;
            }

            if (ins.IsTrgDown(KEY_INPUT_LEFT))
            {
                plateW -= 1.0f;
                changed = true;
            }
        }
    }

    // -----------------------------------------------------
    // Lキー：脚調整
    // -----------------------------------------------------
    if (CheckHitKey(KEY_INPUT_L))
    {
        // Shift + L：脚の位置調整
        if (CheckHitKey(KEY_INPUT_LSHIFT))
        {
            if (ins.IsTrgDown(KEY_INPUT_RIGHT))
            {
                legSlideX += 1.0f;
                changed = true;
            }

            if (ins.IsTrgDown(KEY_INPUT_LEFT))
            {
                legSlideX -= 1.0f;
                changed = true;
            }

            if (ins.IsTrgDown(KEY_INPUT_UP))
            {
                legSlideZ += 1.0f;
                changed = true;
            }

            if (ins.IsTrgDown(KEY_INPUT_DOWN))
            {
                legSlideZ -= 1.0f;
                changed = true;
            }
        }
        // Lのみ：脚の高さ・左右位置調整
        else
        {
            if (ins.IsTrgDown(KEY_INPUT_UP))
            {
                legH += 1.0f;
                changed = true;
            }

            if (ins.IsTrgDown(KEY_INPUT_DOWN))
            {
                legH -= 1.0f;
                changed = true;
            }

            if (ins.IsTrgDown(KEY_INPUT_RIGHT))
            {
                legOX += 1.0f;
                changed = true;
            }

            if (ins.IsTrgDown(KEY_INPUT_LEFT))
            {
                legOX -= 1.0f;
                changed = true;
            }
        }
    }

    // 調整値が変わったら OBB を作り直す
    if (changed)
    {
        Init();
    }
#endif
}

void Table::Draw(void)
{
    trans_.Update();

    // モデル描画
    MV1DrawModel(trans_.modelId);

#ifdef _DEBUG
    // OBBデバッグ描画
    for (const auto& obb : obbColliders_)
    {
        obb.DrawDebug(GetColor(0, 255, 0));
    }
#endif
}
