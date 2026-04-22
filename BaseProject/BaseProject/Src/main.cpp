#include <DxLib.h>
#include "Application.h"

// WinMain関数
//---------------------------------
int WINAPI WinMain(
	_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, 
	_In_ LPSTR lpCmdLine, _In_ int nCmdShow)
	/*
	WindowsのGUIアプリケーションにおけるプログラムの「エン
	トリポイント（開始地点）」となる関数
	ウィンドウの生成やメッセージループの処理を行い、アプリ
	ケーションの寿命を管理する重要な関数
	*/
{

	// インスタンスの生成
	Application::CreateInstance();

	// インスタンスの取得
	Application& instance = Application::GetInstance();

	if (instance.IsInitFail())
	{
		// 初期化失敗
		return -1;
	}

	// 実行
	instance.Run();

	// 解放
	instance.Destroy();

	return 0;

}
