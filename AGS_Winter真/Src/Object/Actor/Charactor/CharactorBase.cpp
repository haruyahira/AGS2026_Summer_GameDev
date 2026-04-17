#include <DxLib.h>
#include "../../../Manager/SceneManager.h"
//#include "../../../Manager/ResourceManager.h"
#include "../../../Application.h"
#include "../../../Utility/AsoUtility.h"
#include "../../Common/AnimationController.h"
#include "../../Collider/ColliderLine.h"
#include "../../Collider/ColliderModel.h"
#include "../../Collider/ColliderCapsule.h"
#include "CharactorBase.h"

CharactorBase::CharactorBase(void) 
	:
	ActorBase()
{

}
CharactorBase::~CharactorBase(void)
{
}

//void CharactorBase::Draw(void)
//{
//	// 基底クラスの描画処理
//	//ActorBase::Draw();
//	// 丸影の描画
//	//DrawShadow();
//}

void CharactorBase::Update(void)
{
	// 移動前座標を更新
	prevPos_ = transform_.pos;

	// 各キャラクターごとの更新処理
	UpdateProcess();

	// 移動方向に応じた遅延回転
	DelayRotate();

	// 重力による移動量
	CalcGravityPow();

	// 衝突判定
	Collision();

	// モデル制御更新
	transform_.Update();

	// アニメーション再生
	animCtr_->Update();

	// 各キャラクターごとの更新後処理
	UpdateProcessPost();
}

void CharactorBase::Release(void)
{
}

void CharactorBase::DelayRotate(void)
{
	// 移動方向から回転に変換する
	Quaternion goalRot = Quaternion::LookRotation(moveDir_);
	// 回転の補間
	transform_.quaRot =
		Quaternion::Slerp(transform_.quaRot, goalRot, 0.2f);
}

void CharactorBase::CalcGravityPow(void)
{
	// 重力方向
	VECTOR dirGravity = AsoUtility::DIR_D;
	// 重力の強さ
	float gravityPow = Application::GetInstance().GetGravityPow() * scnMng_.GetDeltaTime();
	// 重力
	VECTOR gravity = VScale(dirGravity, gravityPow);
	jumpPow_ = VAdd(jumpPow_, gravity);
	// ジャンプ量を加算
	//transform_.pos = VAdd(transform_.pos, jumpPow_);
}

void CharactorBase::Collision(void)
{
	// 移動処理
	transform_.pos = VAdd(transform_.pos, movePow_);

	// 衝突(カプセル)
	CollisionCapsule();

	// ジャンプ量を加算
	transform_.pos = VAdd(transform_.pos, jumpPow_);
	// 衝突(重力)
	CollisionGravity();
}

void CharactorBase::CollisionGravity(void)
{
	// 線分コライダ
	int lineType = static_cast<int>(COLLIDER_TYPE::LINE);
	// 線分コライダが無ければ処理を抜ける
	if (ownColliders_.count(lineType) == 0) return;
	// 線分コライダ情報
	ColliderLine* colliderLine_ = dynamic_cast<ColliderLine*>(ownColliders_.at(lineType));
	if (colliderLine_ == nullptr) return;
	dynamic_cast<ColliderLine*>(ownColliders_.at(lineType));
	// 線分の始点と終点を取得
	VECTOR s = colliderLine_->GetPosStart();
	VECTOR e = colliderLine_->GetPosEnd();
	// 登録されている衝突物を全てチェック
	for (const auto& hitCol : hitColliders_)
	{
		// ステージ以外は処理を飛ばす
		if (hitCol->GetTag() != ColliderBase::TAG::STAGE) continue;
		// 派生クラスへキャスト
		const ColliderModel* colliderModel =
			dynamic_cast<const ColliderModel*>(hitCol);
		if (colliderModel == nullptr) continue;
		// ステージモデル(地面)との衝突
		auto hit = MV1CollCheck_Line(
			colliderModel->GetFollow()->modelId, -1, s, e);
		if (hit.HitFlag > 0)
		{
			// 衝突地点から、少し上に移動
			transform_.pos =
				VAdd(hit.HitPosition, VScale(AsoUtility::DIR_U, 2.0f));
			// ジャンプリセット
			jumpPow_ = AsoUtility::VECTOR_ZERO;
			// ジャンプ判定
			isJump_ = false;
		}
	}

	// 重力速度の制限
	if (jumpPow_.y < MAX_FALL_SPEED)
	{
		jumpPow_.y = MAX_FALL_SPEED;
	}
}

void CharactorBase::CollisionCapsule(void)
{
	// カプセルコライダ
	int capsuleType = static_cast<int>(COLLIDER_TYPE::CAPSULE);
	// カプセルコライダが無ければ処理を抜ける
	if (ownColliders_.count(capsuleType) == 0) return;
	// カプセルコライダ情報
	ColliderCapsule* colliderCapsule =
		dynamic_cast<ColliderCapsule*>(ownColliders_.at(capsuleType));
	if (colliderCapsule == nullptr) return;
	// 登録されている衝突物を全てチェック
	for (const auto& hitCol : hitColliders_)
	{
		// モデル以外は処理を飛ばす
		if (hitCol->GetShape() != ColliderBase::SHAPE::MODEL) continue;
		// 派生クラスへキャスト
		const ColliderModel* colliderModel =
			dynamic_cast<const ColliderModel*>(hitCol);
		if (colliderModel == nullptr) continue;
		auto hits = MV1CollCheck_Capsule(
			colliderModel->GetFollow()->modelId, -1,
			colliderCapsule->GetPosTop(), colliderCapsule->GetPosDown(),
			colliderCapsule->GetRadius());
		// 衝突した複数のポリゴンと衝突回避するまで、
	   // プレイヤーの位置を移動させる
		for (int i = 0; i < hits.HitNum; i++)
		{
			auto hit = hits.Dim[i];
			// 地面と異なり、衝突回避位置が不明なため、何度か移動させる
		   // この時、移動させる方向は、移動前座標に向いた方向であったり、
		   // 衝突したポリゴンの法線方向だったりする
			for (int tryCnt = 0; tryCnt < CNT_TRY_COLLISION; tryCnt++)
			{
				// 再度、モデル全体と衝突検出するには、効率が悪過ぎるので、
			   // 最初の衝突判定で検出した衝突ポリゴン1枚と衝突判定を取る
				int pHit = HitCheck_Capsule_Triangle(
					colliderCapsule->GetPosTop(), colliderCapsule->GetPosDown(),
					colliderCapsule->GetRadius(),
					hit.Position[0], hit.Position[1], hit.Position[2]);
				if (pHit)
				{
					// 法線の方向にちょっとだけ移動させる
					transform_.pos =
						VAdd(transform_.pos,
							VScale(hit.Normal, COLLISION_BACK_DIS));
					continue;
				}
				break;
			}
		}
		// 検出した地面ポリゴン情報の後始末
		MV1CollResultPolyDimTerminate(hits);
	}

}

//void CharactorBase::InitLoad(void)
//{
//	// 丸影画像
//	imgShadow_ = resMng_.Load(ResourceManager::SRC::PLAYER_SHADOW).handleId_;
//}

//void CharactorBase::DrawShadow(void)
//{
//	float PLAYER_SHADOW_HEIGHT = 800.0f;
//	float PLAYER_SHADOW_SIZE = 30.0f;
//	
//	SetUseLighting(FALSE);
//
//	// Ｚバッファを有効にする
//	SetUseZBuffer3D(TRUE);
//
//	// テクスチャアドレスモードを CLAMP にする( テクスチャの端より先は端のドットが延々続く )
//	SetTextureAddressMode(DX_TEXADDRESS_CLAMP);
//	// ステージとの衝突判定（ColliderModel 経由）
//	for (const auto& hitCol : hitColliders_)
//	{
//		if (hitCol->GetTag() != ColliderBase::TAG::STAGE) continue;
//		const ColliderModel* colliderModel =
//			dynamic_cast<const ColliderModel*>(hitCol);
//		if (!colliderModel) continue;
//
//		int stageModel = colliderModel->GetFollow()->modelId;
//		VECTOR playerPos = transform_.pos;
//
//		// プレイヤーの真下に地面があるか調べる
//		auto hit = MV1CollCheck_Capsule(
//			stageModel,
//			-1,
//			playerPos,
//			VAdd(playerPos, VGet(0.0f, -PLAYER_SHADOW_HEIGHT, 0.0f)),
//			PLAYER_SHADOW_SIZE,
//			TRUE
//		);
//
//		if (hit.HitNum > 0)
//		{
//			// 最初の衝突ポリゴンの座標を使用
//			VECTOR groundPos = hit.Dim[0].Position[0];
//			// 地面の少し上に影を浮かせて描画
//			groundPos = VAdd(groundPos, VScale(hit.Dim[0].Normal, 1.0f));
//
//			// 丸影描画
//			DrawBillboard3D(
//				groundPos,
//				1.0f,
//				PLAYER_SHADOW_SIZE,
//				0.0f,
//				imgShadow_,
//				TRUE,
//				1.0f,                // Alpha
//				DX_BLENDMODE_ALPHA,   // BlendMode
//				255                  // BlendParam
//			);
//		}
//
//		MV1CollResultPolyDimTerminate(hit);
//	}
//
//	// ブレンドとZ設定を戻す
//	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
//	SetWriteZBuffer3D(TRUE);
//	SetUseLighting(TRUE);
//
//}
