//#include <float.h>
//#include "Wall.h"
//#include "../../Manager/InputManager.h"
//Wall::Wall(const Transform* trans) : Furniture(NAME::TABLE, trans) {
//}
//
//
//
//void Wall::Init() {
//    // 過去の古い当たり判定を一旦すべて消去
//    colliders_.clear();
//
//    // 1. モデルに含まれるメッシュ（パーツ）の総数を取得
//    int meshNum = MV1GetMeshNum(trans_.modelId);
//    if (meshNum <= 0) return;
//
//    // 2. 最小・最大座標の初期化（極端な値を入れておく）
//    VECTOR minPos = VGet(FLT_MAX, FLT_MAX, FLT_MAX);
//    VECTOR maxPos = VGet(-FLT_MAX, -FLT_MAX, -FLT_MAX);
//
//    // 3. すべてのメッシュをループして、それぞれの最小・最大座標を公式関数で取得
//    for (int i = 0; i < meshNum; i++) {
//
//        // ★これがDxLibに確実に実在する2つの関数です！戻り値で直接VECTORを受け取ります
//        VECTOR meshMin = MV1GetMeshMinPosition(trans_.modelId, i);
//        VECTOR meshMax = MV1GetMeshMaxPosition(trans_.modelId, i);
//
//        // 全体の最小値を更新
//        if (meshMin.x < minPos.x) minPos.x = meshMin.x;
//        if (meshMin.y < minPos.y) minPos.y = meshMin.y;
//        if (meshMin.z < minPos.z) minPos.z = meshMin.z;
//
//        // 全体の最大値を更新
//        if (meshMax.x > maxPos.x) maxPos.x = meshMax.x;
//        if (meshMax.y > maxPos.y) maxPos.y = meshMax.y;
//        if (meshMax.z > maxPos.z) maxPos.z = meshMax.z;
//    }
//
//    // 4. 「中心の座標」を計算する
//    VECTOR localCenter = VGet(
//        (minPos.x + maxPos.x) / 2.0f,
//        (minPos.y + maxPos.y) / 2.0f,
//        (minPos.z + maxPos.z) / 2.0f
//    );
//
//    // 実際のステージ上の位置（ワールド座標）に変換
//    VECTOR worldCenter = VAdd(trans_.GetPos(), localCenter);
//
//    // 5. BoxColliderに渡すための「各辺の半径（HalfSize）」を計算
//    VECTOR halfSize = VGet(
//        (maxPos.x - minPos.x) / 2.0f,
//        (maxPos.y - minPos.y) / 2.0f,
//        (maxPos.z - minPos.z) / 2.0f
//    );
//
//    // 6. 公式関数だけで自動計算した「中心」と「サイズ」をBoxColliderに登録！
//    colliders_.push_back(BoxCollider(worldCenter, halfSize));
//}
//
//void Wall::Update()
//{
//
//}
//
//void Wall::Draw(void) {
//    trans_.Update();
//
//    // モデル描画
//    MV1DrawModel(trans_.modelId);
//
//    for (const auto& box : colliders_) {
//        box.DrawDebug(GetColor(0, 255, 0));
//    }
//
//    // デバッグ情報の表示
//
//  /*  printfDx("Plate: H%.1f W%.1f D%.1f | Slide X%.1f Z%.1f\n",
//        plateH, plateW, plateD, plateSlideX, plateSlideZ);
//    printfDx("Leg:   H%.1f OX%.1f OZ%.1f | Slide X%.1f Z%.1f\n",
//        legH, legOX, legOZ, legSlideX, legSlideZ);*/
//}