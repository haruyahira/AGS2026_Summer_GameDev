#include <DxLib.h>
#include <cmath>
#include "InputManager.h"

InputManager* InputManager::instance_ = nullptr;

void InputManager::CreateInstance(void)
{
    if (instance_ == nullptr)
    {
        instance_ = new InputManager();
    }

    instance_->Init();
}

InputManager& InputManager::GetInstance(void)
{
    if (instance_ == nullptr)
    {
        CreateInstance();
    }

    return *instance_;
}

void InputManager::Destroy(void)
{
    if (instance_ != nullptr)
    {
        instance_->keyInfos_.clear();
        instance_->mouseInfos_.clear();

        delete instance_;
        instance_ = nullptr;
    }
}

InputManager::InputManager(void)
{
    mouseInput_ = 0;

    mouseX_ = 0;
    mouseY_ = 0;
    mouseDiffX_ = 0;
    mouseDiffY_ = 0;

    isFixMouse_ = false;

    mousePos_.x = 0.0f;
    mousePos_.y = 0.0f;

    infoEmpty_ = Info();
    infoEmpty_.key = -1;
    infoEmpty_.keyOld = false;
    infoEmpty_.keyNew = false;
    infoEmpty_.keyTrgDown = false;
    infoEmpty_.keyTrgUp = false;

    mouseInfoEmpty_ = MouseInfo();
    mouseInfoEmpty_.key = -1;
    mouseInfoEmpty_.keyOld = false;
    mouseInfoEmpty_.keyNew = false;
    mouseInfoEmpty_.keyTrgDown = false;
    mouseInfoEmpty_.keyTrgUp = false;

    ZeroMemory(&joyDInState_, sizeof(joyDInState_));
    ZeroMemory(&joyXInState_, sizeof(joyXInState_));
    ZeroMemory(padInfos_, sizeof(padInfos_));
}

void InputManager::Init(void)
{
    keyInfos_.clear();
    mouseInfos_.clear();

    // 基本キー
    Add(KEY_INPUT_SPACE);
    Add(KEY_INPUT_N);
    Add(KEY_INPUT_Z);

    Add(KEY_INPUT_LEFT);
    Add(KEY_INPUT_RIGHT);
    Add(KEY_INPUT_UP);
    Add(KEY_INPUT_DOWN);

    Add(KEY_INPUT_W);
    Add(KEY_INPUT_A);
    Add(KEY_INPUT_S);
    Add(KEY_INPUT_D);

    Add(KEY_INPUT_R);
    Add(KEY_INPUT_T);
    Add(KEY_INPUT_C);
    Add(KEY_INPUT_F);
    Add(KEY_INPUT_E);

    Add(KEY_INPUT_RSHIFT);
    Add(KEY_INPUT_LSHIFT);

    Add(KEY_INPUT_BACKSLASH);
    Add(KEY_INPUT_LALT);
    Add(KEY_INPUT_RETURN);

    Add(KEY_INPUT_1);

    // マウス
    MouseInfo mouseInfo;

    mouseInfo = MouseInfo();
    mouseInfo.key = MOUSE_INPUT_LEFT;
    mouseInfo.keyOld = false;
    mouseInfo.keyNew = false;
    mouseInfo.keyTrgDown = false;
    mouseInfo.keyTrgUp = false;
    mouseInfos_.emplace(mouseInfo.key, mouseInfo);

    mouseInfo = MouseInfo();
    mouseInfo.key = MOUSE_INPUT_RIGHT;
    mouseInfo.keyOld = false;
    mouseInfo.keyNew = false;
    mouseInfo.keyTrgDown = false;
    mouseInfo.keyTrgUp = false;
    mouseInfos_.emplace(mouseInfo.key, mouseInfo);

    Reset();
}

void InputManager::Update(void)
{
    // キーボード更新
    for (auto& p : keyInfos_)
    {
        p.second.keyOld = p.second.keyNew;
        p.second.keyNew = CheckHitKey(p.second.key) != 0;

        p.second.keyTrgDown =
            p.second.keyNew && !p.second.keyOld;

        p.second.keyTrgUp =
            !p.second.keyNew && p.second.keyOld;
    }

    // パッド更新
    SetJPadInState(JOYPAD_NO::KEY_PAD1);
    SetJPadInState(JOYPAD_NO::PAD1);
    SetJPadInState(JOYPAD_NO::PAD2);
    SetJPadInState(JOYPAD_NO::PAD3);
    SetJPadInState(JOYPAD_NO::PAD4);

    // マウス更新
    UpdateMouse();
}

void InputManager::Add(int key)
{
    Info info = Info();

    info.key = key;
    info.keyOld = false;
    info.keyNew = false;
    info.keyTrgDown = false;
    info.keyTrgUp = false;

    keyInfos_.emplace(key, info);
}

void InputManager::Clear(void)
{
    keyInfos_.clear();
}

void InputManager::Reset(void)
{
    // キーボード
    for (auto& p : keyInfos_)
    {
        p.second.keyOld = false;
        p.second.keyNew = false;
        p.second.keyTrgDown = false;
        p.second.keyTrgUp = false;
    }

    // マウス
    for (auto& p : mouseInfos_)
    {
        p.second.keyOld = false;
        p.second.keyNew = false;
        p.second.keyTrgDown = false;
        p.second.keyTrgUp = false;
    }

    // パッド
    ZeroMemory(padInfos_, sizeof(padInfos_));

    // マウス座標
    mouseX_ = 0;
    mouseY_ = 0;
    mouseDiffX_ = 0;
    mouseDiffY_ = 0;
    mouseInput_ = 0;

    mousePos_.x = 0.0f;
    mousePos_.y = 0.0f;
}

bool InputManager::IsNew(int key) const
{
    return Find(key).keyNew;
}

bool InputManager::IsPress(int key) const
{
    return CheckHitKey(key) != 0;
}

bool InputManager::IsTrgDown(int key) const
{
    return Find(key).keyTrgDown;
}

bool InputManager::IsTrgUp(int key) const
{
    return Find(key).keyTrgUp;
}

Vector2 InputManager::GetMousePos(void) const
{
    return mousePos_;
}

int InputManager::GetMouse(void) const
{
    return mouseInput_;
}

bool InputManager::IsClickMouseLeft(void) const
{
    return (mouseInput_ & MOUSE_INPUT_LEFT) != 0;
}

bool InputManager::IsClickMouseRight(void) const
{
    return (mouseInput_ & MOUSE_INPUT_RIGHT) != 0;
}

bool InputManager::IsTrgMouseLeft(void) const
{
    return FindMouse(MOUSE_INPUT_LEFT).keyTrgDown;
}

bool InputManager::IsTrgMouseRight(void) const
{
    return FindMouse(MOUSE_INPUT_RIGHT).keyTrgDown;
}

int InputManager::GetMouseDiffX(void) const
{
    return mouseDiffX_;
}

int InputManager::GetMouseDiffY(void) const
{
    return mouseDiffY_;
}

void InputManager::SetFixMouse(bool isFix)
{
    isFixMouse_ = isFix;
}

bool InputManager::IsFixMouse(void) const
{
    return isFixMouse_;
}

const InputManager::Info& InputManager::Find(int key) const
{
    auto it = keyInfos_.find(key);

    if (it != keyInfos_.end())
    {
        return it->second;
    }

    return infoEmpty_;
}

const InputManager::MouseInfo& InputManager::FindMouse(int key) const
{
    auto it = mouseInfos_.find(key);

    if (it != mouseInfos_.end())
    {
        return it->second;
    }

    return mouseInfoEmpty_;
}

InputManager::JOYPAD_TYPE InputManager::GetJPadType(JOYPAD_NO no)
{
    return static_cast<JOYPAD_TYPE>(
        GetJoypadType(static_cast<int>(no))
        );
}

DINPUT_JOYSTATE InputManager::GetJPadDInputState(JOYPAD_NO no)
{
    DINPUT_JOYSTATE state;
    ZeroMemory(&state, sizeof(state));

    GetJoypadDirectInputState(static_cast<int>(no), &state);

    joyDInState_ = state;

    return state;
}

XINPUT_STATE InputManager::GetJPadXInputState(JOYPAD_NO no)
{
    XINPUT_STATE state;
    ZeroMemory(&state, sizeof(state));

    GetJoypadXInputState(static_cast<int>(no), &state);

    joyXInState_ = state;

    return state;
}

int InputManager::GetPadIndex(JOYPAD_NO no) const
{
    switch (no)
    {
    case JOYPAD_NO::KEY_PAD1:
        return 0;

    case JOYPAD_NO::PAD1:
        return 1;

    case JOYPAD_NO::PAD2:
        return 2;

    case JOYPAD_NO::PAD3:
        return 3;

    case JOYPAD_NO::PAD4:
        return 4;

    default:
        return 0;
    }
}

void InputManager::SetJPadInState(JOYPAD_NO jpNo)
{
    int no = GetPadIndex(jpNo);

    JOYPAD_IN_STATE stateNew = GetJPadInputState(jpNo);
    JOYPAD_IN_STATE& stateNow = padInfos_[no];

    int max = static_cast<int>(JOYPAD_BTN::MAX);

    for (int i = 0; i < max; i++)
    {
        stateNow.ButtonsOld[i] = stateNow.ButtonsNew[i];
        stateNow.ButtonsNew[i] = stateNew.ButtonsNew[i];

        stateNow.IsOld[i] = stateNow.IsNew[i];

        // L2/R2はアナログ値やボタン値の両方を想定して少ししきい値を持たせる
        if (i == static_cast<int>(JOYPAD_BTN::L_TRIGGER) ||
            i == static_cast<int>(JOYPAD_BTN::R_TRIGGER))
        {
            stateNow.IsNew[i] = stateNow.ButtonsNew[i] > 30;
        }
        else
        {
            stateNow.IsNew[i] = stateNow.ButtonsNew[i] > 0;
        }

        stateNow.IsTrgDown[i] =
            stateNow.IsNew[i] && !stateNow.IsOld[i];

        stateNow.IsTrgUp[i] =
            !stateNow.IsNew[i] && stateNow.IsOld[i];
    }

    stateNow.AKeyLX = stateNew.AKeyLX;
    stateNow.AKeyLY = stateNew.AKeyLY;
    stateNow.AKeyRX = stateNew.AKeyRX;
    stateNow.AKeyRY = stateNew.AKeyRY;
}

InputManager::JOYPAD_IN_STATE InputManager::GetJPadInputState(JOYPAD_NO no)
{
    JOYPAD_IN_STATE ret;
    ZeroMemory(&ret, sizeof(ret));

    int inputType = static_cast<int>(no);

    // =====================================================
    // XInput対応
    // Xboxコントローラーなど
    // =====================================================
    XINPUT_STATE x;
    ZeroMemory(&x, sizeof(x));

    if (GetJoypadXInputState(inputType, &x) == 0)
    {
        int idx;

        // Y
        idx = static_cast<int>(JOYPAD_BTN::TOP);
        ret.ButtonsNew[idx] = x.Buttons[XINPUT_BUTTON_Y];

        // X
        idx = static_cast<int>(JOYPAD_BTN::LEFT);
        ret.ButtonsNew[idx] = x.Buttons[XINPUT_BUTTON_X];

        // B
        idx = static_cast<int>(JOYPAD_BTN::RIGHT);
        ret.ButtonsNew[idx] = x.Buttons[XINPUT_BUTTON_B];

        // A
        idx = static_cast<int>(JOYPAD_BTN::DOWN);
        ret.ButtonsNew[idx] = x.Buttons[XINPUT_BUTTON_A];

        // LT
        idx = static_cast<int>(JOYPAD_BTN::L_TRIGGER);
        ret.ButtonsNew[idx] = x.LeftTrigger;

        // RT
        idx = static_cast<int>(JOYPAD_BTN::R_TRIGGER);
        ret.ButtonsNew[idx] = x.RightTrigger;

        // L3
        idx = static_cast<int>(JOYPAD_BTN::L_STICK_PUSH);
        ret.ButtonsNew[idx] = x.Buttons[XINPUT_BUTTON_LEFT_THUMB];

        // R3
        idx = static_cast<int>(JOYPAD_BTN::R_STICK_PUSH);
        ret.ButtonsNew[idx] = x.Buttons[XINPUT_BUTTON_RIGHT_THUMB];

        // 左スティック
        ret.AKeyLX = x.ThumbLX;
        ret.AKeyLY = x.ThumbLY;

        // 右スティック
        ret.AKeyRX = x.ThumbRX;
        ret.AKeyRY = x.ThumbRY;

        return ret;
    }

    // =====================================================
    // DirectInput対応
    // PS5コントローラー / PS4コントローラーなど
    // =====================================================
    JOYPAD_TYPE type = GetJPadType(no);

    switch (type)
    {
    case JOYPAD_TYPE::DUAL_SENSE:
    {
        DINPUT_JOYSTATE d = GetJPadDInputState(no);
        int idx;

        // PS5 DualSense
        // ▲
        idx = static_cast<int>(JOYPAD_BTN::TOP);
        ret.ButtonsNew[idx] = d.Buttons[3];

        // □
        idx = static_cast<int>(JOYPAD_BTN::LEFT);
        ret.ButtonsNew[idx] = d.Buttons[0];

        // ○
        idx = static_cast<int>(JOYPAD_BTN::RIGHT);
        ret.ButtonsNew[idx] = d.Buttons[2];

        // ×
        idx = static_cast<int>(JOYPAD_BTN::DOWN);
        ret.ButtonsNew[idx] = d.Buttons[1];

        // L2
        // 環境によっては Buttons[6] に入る
        idx = static_cast<int>(JOYPAD_BTN::L_TRIGGER);
        ret.ButtonsNew[idx] = d.Buttons[6];

        // R2
        // 環境によっては Buttons[7] に入る
        idx = static_cast<int>(JOYPAD_BTN::R_TRIGGER);
        ret.ButtonsNew[idx] = d.Buttons[7];

        // L3
        idx = static_cast<int>(JOYPAD_BTN::L_STICK_PUSH);
        ret.ButtonsNew[idx] = d.Buttons[10];

        // R3
        idx = static_cast<int>(JOYPAD_BTN::R_STICK_PUSH);
        ret.ButtonsNew[idx] = d.Buttons[11];

        // 左スティック
        ret.AKeyLX = d.X;
        ret.AKeyLY = d.Y;

        // 右スティック
        ret.AKeyRX = d.Z;
        ret.AKeyRY = d.Rz;
    }
    break;

    case JOYPAD_TYPE::DUAL_SHOCK_4:
    {
        DINPUT_JOYSTATE d = GetJPadDInputState(no);
        int idx;

        // PS4 DualShock
        // ▲
        idx = static_cast<int>(JOYPAD_BTN::TOP);
        ret.ButtonsNew[idx] = d.Buttons[3];

        // □
        idx = static_cast<int>(JOYPAD_BTN::LEFT);
        ret.ButtonsNew[idx] = d.Buttons[0];

        // ○
        idx = static_cast<int>(JOYPAD_BTN::RIGHT);
        ret.ButtonsNew[idx] = d.Buttons[2];

        // ×
        idx = static_cast<int>(JOYPAD_BTN::DOWN);
        ret.ButtonsNew[idx] = d.Buttons[1];

        // L2
        idx = static_cast<int>(JOYPAD_BTN::L_TRIGGER);
        ret.ButtonsNew[idx] = d.Buttons[6];

        // R2
        idx = static_cast<int>(JOYPAD_BTN::R_TRIGGER);
        ret.ButtonsNew[idx] = d.Buttons[7];

        // L3
        idx = static_cast<int>(JOYPAD_BTN::L_STICK_PUSH);
        ret.ButtonsNew[idx] = d.Buttons[10];

        // R3
        idx = static_cast<int>(JOYPAD_BTN::R_STICK_PUSH);
        ret.ButtonsNew[idx] = d.Buttons[11];

        // 左スティック
        ret.AKeyLX = d.X;
        ret.AKeyLY = d.Y;

        // 右スティック
        ret.AKeyRX = d.Z;
        ret.AKeyRY = d.Rz;
    }
    break;

    default:
    {
        DINPUT_JOYSTATE d = GetJPadDInputState(no);
        int idx;

        // 汎用DirectInput
        idx = static_cast<int>(JOYPAD_BTN::TOP);
        ret.ButtonsNew[idx] = d.Buttons[3];

        idx = static_cast<int>(JOYPAD_BTN::LEFT);
        ret.ButtonsNew[idx] = d.Buttons[2];

        idx = static_cast<int>(JOYPAD_BTN::RIGHT);
        ret.ButtonsNew[idx] = d.Buttons[1];

        idx = static_cast<int>(JOYPAD_BTN::DOWN);
        ret.ButtonsNew[idx] = d.Buttons[0];

        // 汎用ではL2/R2は誤反応防止で基本0
        idx = static_cast<int>(JOYPAD_BTN::L_TRIGGER);
        ret.ButtonsNew[idx] = 0;

        idx = static_cast<int>(JOYPAD_BTN::R_TRIGGER);
        ret.ButtonsNew[idx] = 0;

        idx = static_cast<int>(JOYPAD_BTN::L_STICK_PUSH);
        ret.ButtonsNew[idx] = d.Buttons[8];

        idx = static_cast<int>(JOYPAD_BTN::R_STICK_PUSH);
        ret.ButtonsNew[idx] = d.Buttons[9];

        ret.AKeyLX = d.X;
        ret.AKeyLY = d.Y;

        ret.AKeyRX = d.Rx;
        ret.AKeyRY = d.Ry;
    }
    break;
    }

    return ret;
}

void InputManager::UpdateMouse(void)
{
    int oldX = mouseX_;
    int oldY = mouseY_;

    GetMousePoint(&mouseX_, &mouseY_);

    if (isFixMouse_)
    {
        int centerX = 640 / 2;
        int centerY = 480 / 2;

        mouseDiffX_ = mouseX_ - centerX;
        mouseDiffY_ = mouseY_ - centerY;

        if (std::abs(mouseDiffX_) <= 1)
        {
            mouseDiffX_ = 0;
        }

        if (std::abs(mouseDiffY_) <= 1)
        {
            mouseDiffY_ = 0;
        }

        SetMousePoint(centerX, centerY);

        mouseX_ = centerX;
        mouseY_ = centerY;
    }
    else
    {
        mouseDiffX_ = mouseX_ - oldX;
        mouseDiffY_ = mouseY_ - oldY;
    }

    mousePos_.x = static_cast<float>(mouseX_);
    mousePos_.y = static_cast<float>(mouseY_);

    int currentInput = GetMouseInput();
    mouseInput_ = currentInput;

    for (auto& pair : mouseInfos_)
    {
        pair.second.keyOld = pair.second.keyNew;

        pair.second.keyNew =
            (currentInput & pair.second.key) != 0;

        pair.second.keyTrgDown =
            pair.second.keyNew && !pair.second.keyOld;

        pair.second.keyTrgUp =
            !pair.second.keyNew && pair.second.keyOld;
    }
}

bool InputManager::IsPadBtnNew(JOYPAD_NO no, JOYPAD_BTN btn) const
{
    return padInfos_[GetPadIndex(no)].IsNew[static_cast<int>(btn)];
}

bool InputManager::IsPadBtnTrgDown(JOYPAD_NO no, JOYPAD_BTN btn) const
{
    return padInfos_[GetPadIndex(no)].IsTrgDown[static_cast<int>(btn)];
}

bool InputManager::IsPadBtnTrgUp(JOYPAD_NO no, JOYPAD_BTN btn) const
{
    return padInfos_[GetPadIndex(no)].IsTrgUp[static_cast<int>(btn)];
}

int InputManager::GetPadAKeyLX(JOYPAD_NO no) const
{
    return padInfos_[GetPadIndex(no)].AKeyLX;
}

int InputManager::GetPadAKeyLY(JOYPAD_NO no) const
{
    return padInfos_[GetPadIndex(no)].AKeyLY;
}

int InputManager::GetPadAKeyRX(JOYPAD_NO no) const
{
    return padInfos_[GetPadIndex(no)].AKeyRX;
}

int InputManager::GetPadAKeyRY(JOYPAD_NO no) const
{
    return padInfos_[GetPadIndex(no)].AKeyRY;
}

int InputManager::GetPadButtonValue(JOYPAD_NO no, JOYPAD_BTN btn) const
{
    return padInfos_[GetPadIndex(no)].ButtonsNew[static_cast<int>(btn)];
}