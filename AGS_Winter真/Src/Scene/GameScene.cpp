#include <DxLib.h>
#include <algorithm>
#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/SoundManager.h"
#include "../Manager/EffectManager.h"
#include "../Object/Actor/Charactor/Player.h"
#include "../Common/CameraShake.h"
#include "GameScene.h"

GameScene::GameScene(void)
	:
	SceneBase()
{
}

GameScene::~GameScene(void)
{
}

void GameScene::Init(void)
{
	// 影の初期化
	shadowHandle_ = MakeShadowMap(2048, 2048);
	// 光の向き（かなり重要）
	VECTOR lightDir = VNorm(VGet(-0.3f, -1.0f, 0.1f));
	SetLightDirection(lightDir);
	SetShadowMapLightDirection(shadowHandle_, lightDir);

	// 高精細影設定
	SetShadowMapDrawArea(
		shadowHandle_,
		VGet(-300.0f, -5.0f, -300.0f),
		VGet(300.0f, 200.0f, 300.0f)
	);


	// 黒潰れ防止（正しい引数）
	SetShadowMapAdjustDepth(shadowHandle_, 0.002f);

	// 環境光（暗くなりすぎ防止）
	SetLightAmbColor(GetColorF(0.4f, 0.4f, 0.4f, 1.0f));
	SetLightDifColor(GetColorF(1.0f, 1.0f, 1.0f, 1.0f));

	// サウンド
	SoundManager::GetInstance()->Load();
	SoundManager::GetInstance()->SetVolumeBGM(150);
	SoundManager::GetInstance()->PlayBGM("bgm1");
	
	// グリッド
	grid_.Init();

	// ステージ
	stage_ = new Stage();
	stage_->Init();
	
	// プレイヤー
	player_ = SceneManager::GetInstance().GetPlayer();
	player_->ResetState();
	player_->Init();

	// 敵
	enemy_= SceneManager::GetInstance().GetEnemy();
	enemy_->Init();
    
	// カメラ
	SceneManager::GetInstance().GetCamera()->Init();
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FOLLOW);
	SceneManager::GetInstance().GetCamera()->SetFollow(
		&SceneManager::GetInstance().GetPlayer()->GetTransform());
	
	// ステージモデルのコライダーをプレイヤーに登録
	const ColliderBase* stageCollider =
	stage_->GetOwnCollider(static_cast<int>(Stage::COLLIDER_TYPE::MODEL));
	player_->AddHitCollider(stageCollider);

    // 修正後:
    //SceneManager::GetInstance().GetPlayer()->Init();

	showHowTo_ = true;   // ← 最初は表示
	howToImage_ = LoadGraph("Data/Image/1.png"); // 画像名はあなたのに合わせて
	
}

void GameScene::Update(void)
{
	if (SceneManager::GetInstance().IsPause())
	{
		return;
	}

	// 操作説明が表示中ならゲームの更新を止める
	if (showHowTo_)
	{
		auto& ins = InputManager::GetInstance();

		// スペース or 左クリック で閉じる
		if (ins.IsTrgDown(KEY_INPUT_SPACE) ||
			(GetMouseInput() & MOUSE_INPUT_LEFT))
		{
			showHowTo_ = false;
		}
		return;   // ゲーム処理はまだ動かさない
	}

	// ステージの更新
	stage_->Update();

	// プレイヤーの更新
	SceneManager::GetInstance().GetPlayer()->Update();

	// 敵の更新
	SceneManager::GetInstance().GetEnemy()->Update();

	VECTOR shake = GetCameraShakeOffset();
	SceneManager::GetInstance().GetCamera()->SetShake(shake);
	

	// シーン遷移
	/*auto const& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		sceMng_.ChangeScene(SceneManager::SCENE_ID::TITLE);
	}*/

}

void GameScene::Draw(void)
{
	// 操作説明中なら背景だけ塗って操作説明を描く
	if (showHowTo_)
	{
		int screenW, screenH;
		GetDrawScreenSize(&screenW, &screenH);

		int w, h;
		GetGraphSize(howToImage_, &w, &h);

		float scaleX = (float)screenW / w;
		float scaleY = (float)screenH / h;
		float scale = (scaleX < scaleY ? scaleX : scaleY);

		int drawW = (int)(w * scale);
		int drawH = (int)(h * scale);

		int drawX = (screenW - drawW) / 2;
		int drawY = (screenH - drawH) / 2;

		DrawExtendGraph(drawX, drawY, drawX + drawW, drawY + drawH, howToImage_, TRUE);
		return;   // ゲーム画面は描かない
	}

	SceneManager::GetInstance().GetCamera()->SetBeforeDraw();

	// ① まずシャドウマップ用：影を落としたいものだけ
		ShadowMap_DrawSetup(shadowHandle_);
		// ステージの描画
		stage_->Draw();
		// プレイヤーの描画
		SceneManager::GetInstance().GetPlayer()->DrawModelOnly();
		SceneManager::GetInstance().GetEnemy()->DrawModelOnly();
		// 終了
		ShadowMap_DrawEnd();


	

	grid_.Draw();  // グリッドの描画
	stage_->Draw();    // ステージを描画（影つき）
	

	SetUseShadowMap(0, shadowHandle_);    // シャドウマップを使う設定
	SceneManager::GetInstance().GetPlayer()->DrawModelOnly();       // キャラも描画（影つき）
	SceneManager::GetInstance().GetEnemy()->DrawModelOnly();
	SetUseShadowMap(0, -1);              // 使用解除

	player_->Draw();
	enemy_->Draw();

	

	/*VECTOR  playerPos = player_->GetTransform().GetPos();
	DrawFormatString(0, 100, 0x111111,
		"PlayerPos = (%.1f, %.1f, %.1f)",
		playerPos.x, playerPos.y, playerPos.z);*/
}

void GameScene::Release(void)
{
	// グリッドの解放
	grid_.Release();
	// ステージの解放
	stage_->Release();
	delete stage_;
	
	// 影の解放
	DeleteShadowMap(shadowHandle_);
}

