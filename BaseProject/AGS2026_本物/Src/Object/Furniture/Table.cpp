#include "Table.h"
#include "../../Manager/InputManager.h"
Table::Table(const Transform* trans) : Furniture(NAME::TABLE, trans) {
}
// ---------------------------------------------------------
  // バランス調整版（全幅100, 高さ60くらいの机を想定）
  // ---------------------------------------------------------
// 【天板 (Plate) 用】
float plateH = 77.0f;  // 天板の上面の高さ
float plateW = 240.0f;  // 天板の幅
float plateD = 150.0f;  // 天板の奥行
float plateT = 5.0f;   // 天板の厚み
float plateSlideX = 8.0f;  // 天板の上面の高さ
float plateSlideZ = 10.2f;  // 天板の幅

// 【脚 (Leg) 用】
float legH = 150.0f;  // 脚そのものの長さ（高さ）
float legW = 4.0f;   // 脚の太さ
float legOX = 58.0f;  // 脚の左右位置 (Offset X)
float legOZ = 35.0f;  // 脚の前後位置 (Offset Z)
static float legSlideX = 7.0f;  // ★脚全体の横移動（スライド）
static float legSlideZ = 11.0f;  // ★脚全体の前後移動（スライド）
static float legSlideY = -40.0f; // ★脚の上下移動用に追加
// ---------------------------------------------------------

void Table::Init() {
    colliders_.clear();
    VECTOR pos = trans_.GetPos();

    // --- 1. 天板 (Plate) ---
    // plateH は「天板上面の高さ」なので、中心はその半分厚み分下になります
    float plateCenterY = plateH - (plateT / 2.0f);
    VECTOR pCenter = VAdd(pos, VGet(plateSlideX, plateCenterY, plateSlideZ));

    // BoxColliderの引数が (中心座標, 各辺の半径(HalfSize)) の場合
    colliders_.push_back(BoxCollider(pCenter, VGet(plateW / 2.0f, plateT / 2.0f, plateD / 2.0f)));

    // --- 2. 脚 (4本) ---
    float lY = legH / 2.0f;
    float xs[2] = { legOX, -legOX };
    float zs[2] = { legOZ, -legOZ };


    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            // 中心位置：親のpos + スライド量 + 四隅のオフセット
            VECTOR lPos = VAdd(pos, VGet(xs[i] + legSlideX, lY + legSlideY, zs[j] + legSlideZ));

            // サイズ指定：太さ(legW)を半分、高さ(legH)も半分にして渡す
            colliders_.push_back(BoxCollider(lPos, VGet(legW / 2.0f, lY, legW / 2.0f)));
        }
    }
}

void Table::Update(void) {
    // デバッグ中だけ有効にするフラグなどがあると安心
#ifdef _DEBUG 
    auto& ins = InputManager::GetInstance();
    bool changed = false;

  

    //// --- Shiftあり：天板の位置を移動（スライド） ---
    //if (CheckHitKey(KEY_INPUT_P)) {
    //    // LSHIFTも一緒に押していたら「移動（スライド）」
    //    if (CheckHitKey(KEY_INPUT_LSHIFT)) {
    //        if (ins.IsTrgDown(KEY_INPUT_RIGHT)) { plateSlideX += 1.0f; changed = true; }
    //        if (ins.IsTrgDown(KEY_INPUT_LEFT)) { plateSlideX -= 1.0f; changed = true; }
    //        if (ins.IsTrgDown(KEY_INPUT_UP)) { plateSlideZ += 1.0f; changed = true; }
    //        if (ins.IsTrgDown(KEY_INPUT_DOWN)) { plateSlideZ -= 1.0f; changed = true; }
    //    }
    //    // Pキーのみなら「サイズ・高さ調整」
    //    else {
    //        if (ins.IsTrgDown(KEY_INPUT_UP)) { plateH += 1.0f; changed = true; } // 高く
    //        if (ins.IsTrgDown(KEY_INPUT_DOWN)) { plateH -= 1.0f; changed = true; } // 低く
    //        if (ins.IsTrgDown(KEY_INPUT_RIGHT)) { plateW += 1.0f; changed = true; } // 幅広く(X)
    //        if (ins.IsTrgDown(KEY_INPUT_LEFT)) { plateW -= 1.0f; changed = true; } // 幅狭く(X)
    //        // 奥行(D)も変えたいなら適当なキーを割り当ててください
    //    }
    //}
    //

    //// --- 脚(Leg)の操作：Lキーを押しながら ---
    //if (CheckHitKey(KEY_INPUT_L)) {
    //    if (CheckHitKey(KEY_INPUT_LSHIFT)) {
    //        if (ins.IsTrgDown(KEY_INPUT_RIGHT)) { legSlideX += 1.0f; changed = true; }
    //        if (ins.IsTrgDown(KEY_INPUT_LEFT)) { legSlideX -= 1.0f; changed = true; }
    //        if (ins.IsTrgDown(KEY_INPUT_UP)) { legSlideZ += 1.0f; changed = true; }
    //        if (ins.IsTrgDown(KEY_INPUT_DOWN)) { legSlideZ -= 1.0f; changed = true; }
    //    }
    //    else {
    //        // IsPress に変えると押しっぱなしで連続変化します
    //        if (ins.IsTrgDown(KEY_INPUT_UP)) { legH += 1.0f;  changed = true; }
    //        if (ins.IsTrgDown(KEY_INPUT_DOWN)) { legH -= 1.0f;  changed = true; }
    //        if (ins.IsTrgDown(KEY_INPUT_RIGHT)) { legOX += 1.0f; changed = true; }
    //        if (ins.IsTrgDown(KEY_INPUT_LEFT)) { legOX -= 1.0f; changed = true; }
    //    }
    //}

    if (changed) Init();
#endif

    //trans_.Update();
}

void Table::Draw(void) {
    trans_.Update();

    // モデル描画
    MV1DrawModel(trans_.modelId);

    for (const auto& box : colliders_) {
        box.DrawDebug(GetColor(0, 255, 0));
    }

    // デバッグ情報の表示
 
  /*  printfDx("Plate: H%.1f W%.1f D%.1f | Slide X%.1f Z%.1f\n",
        plateH, plateW, plateD, plateSlideX, plateSlideZ);
    printfDx("Leg:   H%.1f OX%.1f OZ%.1f | Slide X%.1f Z%.1f\n",
        legH, legOX, legOZ, legSlideX, legSlideZ);*/
}