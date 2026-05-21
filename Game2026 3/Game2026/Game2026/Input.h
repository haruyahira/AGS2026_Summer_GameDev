#pragma once
#include<array>
#include<vector>
#include<string>
#include<unordered_map>

enum class PeriphaeraType {
	keybd, // キーボード
	pad, //ジョイパッド
	mouse // マウス
};

struct InputState {
	PeriphaeraType type;// 周辺機器種別
	unsigned int id; // 実入力の値
};

class KeyconfigScene;
/// <summary>
/// 入力をコントロールするクラス
/// </summary>

class Input
{
	friend KeyconfigScene;
private:
	using InputTable_t = std::unordered_map<std::string, std::vector<InputState>>;
	InputTable_t inputTable_;

	// 押されたかどうかの記録用(実入力の値ではなくて、イベント単位)

	std::unordered_map<std::string, bool> currentInputInfo_;
	std::unordered_map<std::string, bool> lastInputInfo_;



public:
	Input();
	void Update();

	bool IsPressed(const std::string& name)const;
	bool IsTriggered(const std::string& name)const;

};

