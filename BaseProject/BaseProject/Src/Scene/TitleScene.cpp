#include <string>
#include <DxLib.h>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Utility/ColorUtility.h"
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/Camera.h"
#include "TitleScene.h"

TitleScene::TitleScene(void)
{
	
	angleTime_ = 0.0f;
	currentAngle_ = 0.0f;
    fadeCount_ = 0.0f;
	imgPandaX_ = 0;
	imgPandaY_ = 0;

	imgTitleBack_ = -1;
	imgTitleLogo_ = -1;
	imgTitleRedpanda_ = -1;
	imgTitleSelect_ = -1;

}

TitleScene::~TitleScene(void)
{
}

void TitleScene::Init(void)
{
	// 画像読み込み
	imgTitleBack_ = resMng_.Load(ResourceManager::SRC::BACK_GROUND).handleId_;
	imgTitleLogo_ = resMng_.Load(ResourceManager::SRC::TITLE_LOGO).handleId_;
    imgTitleRedpanda_ = resMng_.Load(ResourceManager::SRC::TITLE_READ_PANDA).handleId_;
    imgTitleSelect_ = resMng_.Load(ResourceManager::SRC::TITLE_SELECT).handleId_;
	imgTitleSelectHitBox_ = resMng_.Load(ResourceManager::SRC::TITLE_SELECT2).handleId_;
	imgSelectHandles_[1] = resMng_.Load(ResourceManager::SRC::TITLE_SELECT_BRIGHT1).handleId_;
	imgSelectHandles_[2] = resMng_.Load(ResourceManager::SRC::TITLE_SELECT_BRIGHT2).handleId_;
	imgSelectHandles_[3] = resMng_.Load(ResourceManager::SRC::TITLE_SELECT_BRIGHT3).handleId_;

	// 選択肢の当たり判定用矩形を初期化
	InitSelect();

	// レッサーパンダの画像サイズを取得
	GetGraphSize(imgTitleRedpanda_, &imgPandaX_, &imgPandaY_);
	imgPandaX_/= CENTER_DIVIDER;

	fontHandle_ = CreateFontToHandle("ＭＳ Ｐゴシック", 64, 3, DX_FONTTYPE_ANTIALIASING_8X8);
	
	// テロップの内容一覧から、テロップのデータを自動生成して配列に追加する
	for (int i = 0; i < messages.size(); ++i) {
		telops_.push_back({
			(int)Application::SCREEN_SIZE_X + (i * 300), // 重ならないようにずらす
			(int)GetRand(Application::adjustedSizeY_),                  // 高さをずらす
			(float)AsoUtility::GetRandomFloat(2.0f,5.0f),                 // スピードをランダムにする
			messages[i],
		    Color::WHITE
			});
	}

	bgm_ = LoadSoundMem("Data/Bgm/TitleBgm.wav");

	ChangeVolumeSoundMem(0, bgm_);

	// 3. ループ再生を開始
	if (bgm_ != -1) {
		PlaySoundMem(bgm_, DX_PLAYTYPE_LOOP);
	}

	// 定点カメラ
	SceneManager::GetInstance().GetCamera()->ChangeMode(Camera::MODE::FIXED_POINT);
}

void TitleScene::Update(void)
{
	// マウスカーソルを表示する
	SetMouseDispFlag(TRUE);

	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);
	}
	// レッサーパンダ更新
	UpdateRedpanda();

	// 選択更新
	UpdateSelect();

	// テロップ更新
	UpdateTelop();
	
}

void TitleScene::Draw(void)
{
	
	// 背景
	DrawExtendGraph(0,0,
		Application::SCREEN_SIZE_X, Application::adjustedSizeY_, imgTitleBack_, true);
	
	// レッサーパンダ
	DrawRedpanda();

	// タイトルロゴ
	DrawExtendGraph(0, 0,Application::SCREEN_SIZE_X, Application::adjustedSizeY_,  imgTitleLogo_, true);

	// 選択肢
	DrawSelect();

	// テロップ
	DrawTelop();
	
}

void TitleScene::InitSelect(void)
{
	// 1. 画像サイズを取得
	int imgW, imgH;
	GetGraphSize(imgTitleSelectHitBox_, &imgW, &imgH);

	//// 2. 描画時と同じ拡大率と基準座標を設定
	float scale = 0.73f;
	int centerX = Application::SCREEN_SIZE_X / 2;
	int centerY = Application::adjustedSizeY_ - 320; // DrawRotaGraphで指定している座標

	//// 3. 1ボタンあたりのサイズ（元画像が縦に3つ並んでいる場合）
	int singleBtnSrcH = imgH / 3;
	int displayW = (int)(imgW * scale);
	int displayH = (int)(singleBtnSrcH * scale);

	// 4. 判定エリアの計算（中心座標から逆算）
	int startY = centerY - (int)(imgH * scale / 2.0f); // 画像全体の上の端

	for (int i = 0; i < 3; i++) {
		btnRects_[i].x = centerX - (displayW / 2);
		btnRects_[i].y = startY + (i * displayH);
		btnRects_[i].w = displayW;
		btnRects_[i].h = displayH;
	}
}


void TitleScene::UpdateRedpanda(void)
{
	// 時間を進める（この値を調整すると速さが変わる）
	angleTime_ += 0.015f;

	currentAngle_ = (25.0f * AsoUtility::DEG2RAD) * sinf(angleTime_);
}

void TitleScene::UpdateSelect(void)
{
	// マウス座標の取得
	Vector2 mousePos = InputManager::GetInstance().GetMousePos();
	// 表示インデックスをリセット（基本は「誰も選択されていない(0)」）
	drawIndex_ = 0;

	// 3つのボタンをループで判定
	for (int i = 0; i < 3; i++) {
		if (mousePos.x >= btnRects_[i].x && mousePos.x <= btnRects_[i].x + btnRects_[i].w &&
			mousePos.y >= btnRects_[i].y && mousePos.y <= btnRects_[i].y + btnRects_[i].h)
		{
			drawIndex_ = i + 1; // マウスが乗っていたら 1, 2, 3 のいずれかになる
			break;
		}
		
	}

	if (drawIndex_ > 0) {
		// マウスが乗っている間、サイン波などでふわふわさせる
		fadeCount_ += RAISE＿FADE;
	}
	else {
		fadeCount_ = RESET_FADE; // 離れたらリセット
	}

	// クリック判定
	InputManager& ins = InputManager::GetInstance();

	// マウスの左ボタンが押された瞬間をチェック
	if (ins.IsTrgMouseLeft())
	{
		// 1番目（スタート）
		if (drawIndex_ == 1) {
			SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);
		}
		// 2番目（ランキング）
		else if (drawIndex_ == 2) {
			// 
		}
		// 3番目（終了）
		else if (drawIndex_ == 3) {
			// DXライブラリの終了メッセージを送る
			PostQuitMessage(0);
		}
	}
}

void TitleScene::UpdateTelop(void)
{
	// 全てのテロップに対して処理を行う
	for (Telop& telop : telops_) {
		telop.x -= static_cast<int>(telop.speed); // 左へ移動

		// 文字の横幅を取得
		textX_ = GetDrawStringWidth(telop.text.c_str(), (int)telop.text.length());

		// 画面左端に消えたら右端に戻す
		if (telop.x < -textX_) {
			telop.speed = (float)AsoUtility::GetRandomFloat(2.0f, 5.0f); // スピードをランダムにする
			telop.x = (int)Application::SCREEN_SIZE_X;
			// 右に戻るついでに、高さをランダムに変える
			telop.y = (int)(GetRand(Application::adjustedSizeY_));
		}
	}
}


void TitleScene::DrawRedpanda(void)
{
	
	// レッサーパンダ
	DrawRotaGraph3(
		Application::SCREEN_SIZE_X,
		Application::adjustedSizeY_ + REDPANDA_ADJUST_Y,
		(int)imgPandaX_,
		(int)imgPandaY_,
		0.7, 0.7,
		currentAngle_,
		imgTitleRedpanda_,
		true
	);
}

void TitleScene::DrawSelect(void)
{
	//DrawExtendGraph(0, 0, Application::SCREEN_SIZE_X, Application::adjustedSizeY_,  imgTitleSelect_, true);
	
	//// マウスが乗っている選択肢のみ、光っている画像(imgSelectHandles_)を重ねる
	//if (drawIndex_ > 0) {
	//	int i = drawIndex_ - 1; // 0, 1, 2 に変換

	//	// 当たり判定の四角形(btnRects_)をそのまま使って描画
	//	DrawExtendGraph(
	//		0,
	//		0,
	//		Application::SCREEN_SIZE_X ,
	//		Application::adjustedSizeY_,
	//		imgSelectHandles_[drawIndex_], TRUE
	//	);
	//}
	// 光っていない選択肢画像の描画（ベースとして常に表示）
	DrawExtendGraph(0, 0, Application::SCREEN_SIZE_X, Application::adjustedSizeY_, imgTitleSelect_, TRUE);

	// マウスが乗っているボタンだけ、光っている画像をフェードインで重ねる
	if (drawIndex_ > 0)
	{
		// sin関数（明るい＜－＞暗い）
		int alpha = (int)((sin(fadeCount_) * ALPHA_HALF_RANGE + ALPHA_OFFSET) * MAX_ALPHA);

		// 描画モードを「アルファブレンド」に設定して透明度を適用
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

		// 光っている画像の描画
		DrawExtendGraph(
			0, 0,
			Application::SCREEN_SIZE_X, Application::adjustedSizeY_,
			imgSelectHandles_[drawIndex_],
			TRUE
		);

		// 重要：描画が終わったらブレンドモードを通常（不透明）に戻す
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}




	// --- デバッグ用：当たり判定の可視化 ---
	// 3つの判定範囲をループで描画する
	//for (int i = 0; i < 3; i++) {
	//	// マウスが乗っている判定範囲は「赤」、それ以外は「緑」で表示
	//	unsigned int color = (drawIndex_ == i + 1) ? GetColor(255, 0, 0) : GetColor(0, 255, 0);

	//	// DrawBoxの引数: (左上x, 左上y, 右下x, 右下y, 色, 塗りつぶしフラグ)
	//	DrawBox(
	//		btnRects_[i].x,
	//		btnRects_[i].y,
	//		btnRects_[i].x + btnRects_[i].w,
	//		btnRects_[i].y + btnRects_[i].h,
	//		color,
	//		FALSE // FALSEにすると枠線だけになる
	//	);
	//}
	
}

void TitleScene::DrawTelop()
{

	for (const auto& t : telops_) {
		// 縁取り文字で描画
		DrawStringFToHandle((int)t.x, (int)t.y,
			t.text.c_str(), t.color, fontHandle_);
		//DrawString((int)t.x, (int)t.y, t.text.c_str(), t.color, GetColor(0, 0, 0));
	}
}