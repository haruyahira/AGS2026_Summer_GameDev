#include "Input.h"

#include<DxLib.h>

Input::Input()
{
	inputTable_["up"] = { { PeriphaeraType::keybd, KEY_INPUT_UP },
		{PeriphaeraType::keybd, KEY_INPUT_W},
		{PeriphaeraType::pad, PAD_INPUT_UP} };
	inputTable_["down"] = { { PeriphaeraType::keybd, KEY_INPUT_DOWN },
		{PeriphaeraType::keybd, KEY_INPUT_S},
		{PeriphaeraType::pad, PAD_INPUT_DOWN} };
	inputTable_["left"] = { { PeriphaeraType::keybd, KEY_INPUT_LEFT },
		{PeriphaeraType::keybd, KEY_INPUT_A},
		{PeriphaeraType::pad, PAD_INPUT_LEFT} };
	inputTable_["right"] = { { PeriphaeraType::keybd, KEY_INPUT_RIGHT },
		{PeriphaeraType::keybd, KEY_INPUT_D},
		{PeriphaeraType::pad, PAD_INPUT_RIGHT} };


	// DXライブラリはPADボタン番号が６ボタンベースなのでずれています
	inputTable_["ok"] = { { PeriphaeraType::keybd, KEY_INPUT_RETURN },
	                     {PeriphaeraType::mouse, MOUSE_INPUT_LEFT},
		                 {PeriphaeraType::pad, PAD_INPUT_L} }; // セレクトボタン
	
	inputTable_["cancel"] = {
		{PeriphaeraType::keybd, KEY_INPUT_ESCAPE} };


	inputTable_["pause"] = { {PeriphaeraType::keybd, KEY_INPUT_P},
							{PeriphaeraType::pad,PAD_INPUT_R} };
	inputTable_["jump"] = { {PeriphaeraType::keybd, KEY_INPUT_Z},
						   {PeriphaeraType::pad,PAD_INPUT_C} };
	inputTable_["attack"] = { {PeriphaeraType::keybd, KEY_INPUT_X},
							 {PeriphaeraType::pad , PAD_INPUT_A} };
 
	for (auto& inputInfo : inputTable_) {
		currentInputInfo_[inputInfo.first] = false;
		lastInputInfo_[inputInfo.first] = false;
	}

}

void Input::Update()
{
	lastInputInfo_ = currentInputInfo_;
	// 生データの取得
	std::array<char, 256> keystate;
	GetHitKeyStateAll(keystate.data());
	int mouseState = GetMouseInput();
	int padState = GetJoypadInputState(DX_INPUT_PAD1);// 取りあえず１コンだけ
	// 生データをcurrentInputInfo_に反映させる
	for (const auto& inputInfo : inputTable_) {
		const auto& eventName = inputInfo.first;
		for (const auto& inputState : inputInfo.second) {
			switch (inputState.type)
			{
			case PeriphaeraType::keybd:
				currentInputInfo_[eventName] = keystate[inputState.id];
				break;
			case PeriphaeraType::mouse:
				currentInputInfo_[eventName] = mouseState&inputState.id;
				break;
			case PeriphaeraType::pad:
				currentInputInfo_[eventName] = (padState&inputState.id);
				break;
			}
			if (currentInputInfo_[eventName]) {
				break;
			}
		}
	}
}

bool Input::IsPressed(const std::string& name) const
{
	if (!currentInputInfo_.contains(name)) {
		return false;
	}

		return currentInputInfo_.at(name);
		
		

}

bool Input::IsTriggered(const std::string& name) const
{

	if (!currentInputInfo_.contains(name)) {
		return false;
	}

		return currentInputInfo_.at(name) && !(lastInputInfo_.at(name));



	return false;
}
