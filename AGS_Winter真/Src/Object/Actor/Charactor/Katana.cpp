#include <DxLib.h>
#include "../../../Manager/ResourceManager.h"
#include "../../../Manager/SceneManager.h"
#include "../../../Manager/InputManager.h"
#include "../../../Manager/Camera.h"
#include "../../../Manager/SoundManager.h"
#include "../../../Utility/AsoUtility.h"
#include "../../Common/AnimationController.h"
#include "../ActorBase.h"
#include "Katana.h"
#include "../../Collider/ColliderLine.h"
Katana::Katana(void)
{

}


Katana::~Katana(void)
{
}




void Katana::Draw(void)
{
	ActorBase::Draw();

	MV1DrawModel(transform_.modelId);
}

void Katana::Release(void)
{
	ActorBase::Release();
}

void Katana::InitLoad(void)
{
	// 基底クラスのリソースロード
	//CharactorBase::InitLoad();

	transform_.SetModel(resMng_.Load(
		ResourceManager::SRC::KATANA).handleId_);
}

void Katana::InitTransform(void)
{
	transform_.scl = KATANA_SCL;
	transform_.pos = PLAYER_POS;
	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::AngleAxis(
		AsoUtility::Deg2RadF(180.0f), AsoUtility::AXIS_Y);
	//transform_.quaRotLocal = Quaternion::Identity();

	transform_.Update();
}

void Katana::InitCollider(void)
{
	// 主に地面との衝突で仕様する線分コライダ
	ColliderLine* colLine = new ColliderLine(
		ColliderBase::TAG::PLAYER, &transform_,
		COL_LINE_START_LOCAL_POS, COL_LINE_END_LOCAL_POS);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::LINE), colLine);
}

void Katana::InitAnimation(void)
{

}

void Katana::InitPost(void)
{
}

void Katana::UpdateProcess(void)
{
	
}

void Katana::UpdateProcessPost(void)
{
}

// 移動操作
void Katana::ProcessMove(void)
{

}

void Katana::ProcessJump(void)
{
	
}

void Katana::UpdateHandMatrix(const MATRIX& handMatrix)
{
	// --- ① スケールを除去した正規化行列を作る（最重要） ---
	MATRIX noScale = handMatrix;

	// X軸ベクトル
	VECTOR ax = VGet(noScale.m[0][0], noScale.m[0][1], noScale.m[0][2]);
	float lenX = sqrtf(ax.x * ax.x + ax.y * ax.y + ax.z * ax.z);
	noScale.m[0][0] /= lenX;
	noScale.m[0][1] /= lenX;
	noScale.m[0][2] /= lenX;

	// Y軸ベクトル
	VECTOR ay = VGet(noScale.m[1][0], noScale.m[1][1], noScale.m[1][2]);
	float lenY = sqrtf(ay.x * ay.x + ay.y * ay.y + ay.z * ay.z);
	noScale.m[1][0] /= lenY;
	noScale.m[1][1] /= lenY;
	noScale.m[1][2] /= lenY;

	// Z軸ベクトル
	VECTOR az = VGet(noScale.m[2][0], noScale.m[2][1], noScale.m[2][2]);
	float lenZ = sqrtf(az.x * az.x + az.y * az.y + az.z * az.z);
	noScale.m[2][0] /= lenZ;
	noScale.m[2][1] /= lenZ;
	noScale.m[2][2] /= lenZ;
	// --- ② ローカルオフセット（握り位置調整） ---
	MATRIX offset = MGetIdent();
	offset = MMult(offset, MGetRotX(AsoUtility::Deg2RadF(/*-90*/0)));
	offset = MMult(offset, MGetRotY(AsoUtility::Deg2RadF(0)));
	offset = MMult(offset, MGetRotZ(AsoUtility::Deg2RadF(90)));

	offset.m[3][0] = 10.5f;
	offset.m[3][1] = 28.0f;
	offset.m[3][2] = 1.0f;

	// --- ③ 最終行列 = オフセット × スケール除去した手ボーン ---
	MATRIX result = MMult(offset, noScale);

	// --- ④ プレイヤーのスケールを打ち消し、刀は自分の scale のみを適用 ---
	result = MMult(MGetScale(KATANA_SCL), result);

	MV1SetMatrix(transform_.modelId, result);
}
