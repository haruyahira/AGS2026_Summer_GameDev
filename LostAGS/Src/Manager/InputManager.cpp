#include <DxLib.h>
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
        InputManager::CreateInstance();
    }

    return *instance_;
}

InputManager::InputManager(void)
{
    mouseInput_ = -1;

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
    Add(KEY_INPUT_TAB);

    Add(KEY_INPUT_LCONTROL);
    Add(KEY_INPUT_RCONTROL);
    Add(KEY_INPUT_BACKSLASH);
    Add(KEY_INPUT_LALT);
    Add(KEY_INPUT_RETURN);

    Add(KEY_INPUT_1);
    Add(KEY_INPUT_2);
    Add(KEY_INPUT_3);
    Add(KEY_INPUT_4);

    Add(MOUSE_INPUT_LEFT);
    Add(MOUSE_INPUT_RIGHT);
    

    InputManager::MouseInfo info;

    info = InputManager::MouseInfo();
    info.key = MOUSE_INPUT_LEFT;
    info.keyOld = false;
    info.keyNew = false;
    info.keyTrgDown = false;
    info.keyTrgUp = false;
    mouseInfos_.emplace(info.key, info);

    info = InputManager::MouseInfo();
    info.key = MOUSE_INPUT_RIGHT;
    info.keyOld = false;
    info.keyNew = false;
    info.keyTrgDown = false;
    info.keyTrgUp = false;
    mouseInfos_.emplace(info.key, info);
}

void InputManager::Update(void)
{
    // キーボード検知
    for (auto& p : keyInfos_)
    {
        p.second.keyOld = p.second.keyNew;
        p.second.keyNew = CheckHitKey(p.second.key) != 0;
        p.second.keyTrgDown = p.second.keyNew && !p.second.keyOld;
        p.second.keyTrgUp = !p.second.keyNew && p.second.keyOld;
    }

    // パッド情報更新
    SetJPadInState(JOYPAD_NO::KEY_PAD1);
    SetJPadInState(JOYPAD_NO::PAD1);
    SetJPadInState(JOYPAD_NO::PAD2);
    SetJPadInState(JOYPAD_NO::PAD3);
    SetJPadInState(JOYPAD_NO::PAD4);

    // マウス更新
    UpdateMouse();
}

void InputManager::Destroy(void)
{
    keyInfos_.clear();
    mouseInfos_.clear();

    delete instance_;
    instance_ = nullptr;
}

void InputManager::Add(int key)
{
    InputManager::Info info = InputManager::Info();

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
    return mouseInput_ == MOUSE_INPUT_LEFT;
}

bool InputManager::IsClickMouseRight(void) const
{
    return mouseInput_ == MOUSE_INPUT_RIGHT;
}

bool InputManager::IsTrgMouseLeft(void) const
{
    return FindMouse(MOUSE_INPUT_LEFT).keyTrgDown;
}

bool InputManager::IsTrgMouseRight(void) const
{
    return FindMouse(MOUSE_INPUT_RIGHT).keyTrgDown;
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
    return static_cast<InputManager::JOYPAD_TYPE>(
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

    auto stateNew = GetJPadInputState(jpNo);
    auto& stateNow = padInfos_[no];

    int max = static_cast<int>(JOYPAD_BTN::MAX);

    for (int i = 0; i < max; i++)
    {
        stateNow.ButtonsOld[i] = stateNow.ButtonsNew[i];
        stateNow.ButtonsNew[i] = stateNew.ButtonsNew[i];

        stateNow.IsOld[i] = stateNow.IsNew[i];

        if (i == static_cast<int>(JOYPAD_BTN::R_TRIGGER) ||
            i == static_cast<int>(JOYPAD_BTN::L_TRIGGER))
        {
            stateNow.IsNew[i] = stateNow.ButtonsNew[i] > 30;
        }
        else
        {
            stateNow.IsNew[i] = stateNow.ButtonsNew[i] > 0;
        }

        stateNow.IsTrgDown[i] = stateNow.IsNew[i] && !stateNow.IsOld[i];
        stateNow.IsTrgUp[i] = !stateNow.IsNew[i] && stateNow.IsOld[i];
    }

    stateNow.AKeyLX = stateNew.AKeyLX;
    stateNow.AKeyLY = stateNew.AKeyLY;
    stateNow.AKeyRX = stateNew.AKeyRX;
    stateNow.AKeyRY = stateNew.AKeyRY;
}

//InputManager::JOYPAD_IN_STATE InputManager::GetJPadInputState(JOYPAD_NO no)
//{
//    JOYPAD_IN_STATE ret;
//    ZeroMemory(&ret, sizeof(ret));
//
//    int inputType = static_cast<int>(no);
//
//    auto type = GetJPadType(no);
//
//    // =====================================================
//    // XInput
//    // DualSense 以外は XInput を優先
// // =====================================================
//// XInput
//// Xboxコントローラーも PS 配置として扱う
//// TOP   = △ = Y
//// LEFT  = □ = X
//// RIGHT = ○ = B
//// DOWN  = × = A
//// =====================================================
//    XINPUT_STATE x;
//    ZeroMemory(&x, sizeof(x));
//
//    if (type != JOYPAD_TYPE::DUAL_SENSE &&
//        GetJoypadXInputState(inputType, &x) == 0)
//    {
//        int idx;
//
//        // △ / Y
//        idx = static_cast<int>(JOYPAD_BTN::TOP);
//        ret.ButtonsNew[idx] = x.Buttons[XINPUT_BUTTON_Y];
//
//        // □ / X
//        idx = static_cast<int>(JOYPAD_BTN::LEFT);
//        ret.ButtonsNew[idx] = x.Buttons[XINPUT_BUTTON_X];
//
//        // ○ / B
//        idx = static_cast<int>(JOYPAD_BTN::RIGHT);
//        ret.ButtonsNew[idx] = x.Buttons[XINPUT_BUTTON_B];
//
//        // × / A
//        idx = static_cast<int>(JOYPAD_BTN::DOWN);
//        ret.ButtonsNew[idx] = x.Buttons[XINPUT_BUTTON_A];
//
//        // L1 / LB
//        idx = static_cast<int>(JOYPAD_BTN::L_BUTTON);
//        ret.ButtonsNew[idx] = x.Buttons[XINPUT_BUTTON_LEFT_SHOULDER];
//
//        // R1 / RB
//        idx = static_cast<int>(JOYPAD_BTN::R_BUTTON);
//        ret.ButtonsNew[idx] = x.Buttons[XINPUT_BUTTON_RIGHT_SHOULDER];
//
//        // L2 / LT
//        idx = static_cast<int>(JOYPAD_BTN::L_TRIGGER);
//        ret.ButtonsNew[idx] = x.LeftTrigger;
//
//        // R2 / RT
//        idx = static_cast<int>(JOYPAD_BTN::R_TRIGGER);
//        ret.ButtonsNew[idx] = x.RightTrigger;
//
//        // L3
//        idx = static_cast<int>(JOYPAD_BTN::L_STICK_PUSH);
//        ret.ButtonsNew[idx] = x.Buttons[XINPUT_BUTTON_LEFT_THUMB];
//
//        // R3
//        idx = static_cast<int>(JOYPAD_BTN::R_STICK_PUSH);
//        ret.ButtonsNew[idx] = x.Buttons[XINPUT_BUTTON_RIGHT_THUMB];
//
//        // 左スティック
//        ret.AKeyLX = x.ThumbLX;
//        ret.AKeyLY = x.ThumbLY;
//
//        // 右スティック
//        ret.AKeyRX = x.ThumbRX;
//        ret.AKeyRY = x.ThumbRY;
//
//        return ret;
//    }
//
//    // =====================================================
//    // DirectInput
//    // =====================================================
//    switch (type)
//    {
//        // =====================================================
//        // PS5 DualSense
//        // =====================================================
//    case InputManager::JOYPAD_TYPE::DUAL_SENSE:
//    {
//        auto d = GetJPadDInputState(no);
//
//        int idx;
//
//        // △
//        idx = static_cast<int>(JOYPAD_BTN::TOP);
//        ret.ButtonsNew[idx] =
//            d.Buttons[3];
//
//        // □
//        idx = static_cast<int>(JOYPAD_BTN::LEFT);
//        ret.ButtonsNew[idx] =
//            d.Buttons[0];
//
//        // ○
//        idx = static_cast<int>(JOYPAD_BTN::RIGHT);
//        ret.ButtonsNew[idx] =
//            d.Buttons[2];
//
//        // ×
//        idx = static_cast<int>(JOYPAD_BTN::DOWN);
//        ret.ButtonsNew[idx] =
//            d.Buttons[1];
//
//        // L1
//        idx = static_cast<int>(JOYPAD_BTN::L_BUTTON);
//        ret.ButtonsNew[idx] =
//            d.Buttons[4];
//
//        // R1
//        idx = static_cast<int>(JOYPAD_BTN::R_BUTTON);
//        ret.ButtonsNew[idx] =
//            d.Buttons[5];
//
//        // LT
//        idx = static_cast<int>(JOYPAD_BTN::L_TRIGGER);
//        ret.ButtonsNew[idx] = d.Buttons[6];
//
//        // RT
//        idx = static_cast<int>(JOYPAD_BTN::R_TRIGGER);
//        ret.ButtonsNew[idx] = d.Buttons[7];
//
//
//
//        // L3
//        idx = static_cast<int>(JOYPAD_BTN::L_STICK_PUSH);
//        ret.ButtonsNew[idx] =
//            d.Buttons[10];
//
//        // R3
//        idx = static_cast<int>(JOYPAD_BTN::R_STICK_PUSH);
//        ret.ButtonsNew[idx] =
//            d.Buttons[11];
//
//
//        // 左スティック
//        ret.AKeyLX = d.X;
//        ret.AKeyLY = d.Y;
//
//        // 右スティック
//        ret.AKeyRX = d.Z;
//        ret.AKeyRY = d.Rz;
//
//    }
//    break;
//
//    // =====================================================
//    // その他 DirectInput
//    // =====================================================
//    default:
//    {
//        auto d = GetJPadDInputState(no);
//
//        int idx;
//
//        // Y
//        idx = static_cast<int>(JOYPAD_BTN::TOP);
//        ret.ButtonsNew[idx] =
//            d.Buttons[3];
//
//        // X
//        idx = static_cast<int>(JOYPAD_BTN::LEFT);
//        ret.ButtonsNew[idx] =
//            d.Buttons[0];
//
//        // B
//        idx = static_cast<int>(JOYPAD_BTN::RIGHT);
//        ret.ButtonsNew[idx] =
//            d.Buttons[2];
//
//        // A
//        idx = static_cast<int>(JOYPAD_BTN::DOWN);
//        ret.ButtonsNew[idx] =
//            d.Buttons[1];
//
//
//        idx = static_cast<int>(JOYPAD_BTN::L_TRIGGER);
//        ret.ButtonsNew[idx] = d.Buttons[6];
//
//        idx = static_cast<int>(JOYPAD_BTN::R_TRIGGER);
//        ret.ButtonsNew[idx] = d.Buttons[7];
//
//
//        // L3
//        idx = static_cast<int>(JOYPAD_BTN::L_STICK_PUSH);
//        ret.ButtonsNew[idx] =
//            d.Buttons[10];
//
//        // R3
//        idx = static_cast<int>(JOYPAD_BTN::R_STICK_PUSH);
//        ret.ButtonsNew[idx] =
//            d.Buttons[11];
//
//        // 左スティック
//        ret.AKeyLX = d.X;
//        ret.AKeyLY = d.Y;
//
//        // 右スティック
//        ret.AKeyRX = d.Z;
//        ret.AKeyRY = d.Rz;
//    }
//    break;
//    }
//
//    return ret;
//}
InputManager::JOYPAD_IN_STATE
InputManager::GetJPadInputState(
    JOYPAD_NO no
)
{
    JOYPAD_IN_STATE ret;
    ZeroMemory(
        &ret,
        sizeof(ret)
    );

    const int inputType =
        static_cast<int>(no);

    const JOYPAD_TYPE padType =
        GetJPadType(no);

#ifdef _DEBUG

    // どのパッド種類として認識されているか確認
    if (no == JOYPAD_NO::PAD1)
    {
        static int typeDebugCounter = 0;

        typeDebugCounter++;

        if (typeDebugCounter >= 60)
        {
            typeDebugCounter = 0;

            char text[128];

            sprintf_s(
                text,
                sizeof(text),
                "PAD1 Type=%d DualSenseEnum=%d\n",
                static_cast<int>(padType),
                static_cast<int>(
                    JOYPAD_TYPE::DUAL_SENSE
                    )
            );

            OutputDebugStringA(text);
        }
    }

#endif

    // ========================================
    // Xbox専用 XInput処理
    // ========================================

    XINPUT_STATE x;
    ZeroMemory(
        &x,
        sizeof(x)
    );

    const int xInputResult =
        GetJoypadXInputState(
            inputType,
            &x
        );

    // DualSenseの場合は、
    // XInputが成功扱いでも使用しない
    if (padType != JOYPAD_TYPE::DUAL_SENSE &&
        xInputResult == 0)
    {
        int idx = 0;

        // Y / △
        idx = static_cast<int>(
            JOYPAD_BTN::TOP
            );

        ret.ButtonsNew[idx] =
            x.Buttons[XINPUT_BUTTON_Y];

        // X / □
        idx = static_cast<int>(
            JOYPAD_BTN::LEFT
            );

        ret.ButtonsNew[idx] =
            x.Buttons[XINPUT_BUTTON_X];

        // B / ○
        idx = static_cast<int>(
            JOYPAD_BTN::RIGHT
            );

        ret.ButtonsNew[idx] =
            x.Buttons[XINPUT_BUTTON_B];

        // A / ×
        idx = static_cast<int>(
            JOYPAD_BTN::DOWN
            );

        ret.ButtonsNew[idx] =
            x.Buttons[XINPUT_BUTTON_A];

        // LB / L1
        idx = static_cast<int>(
            JOYPAD_BTN::L_BUTTON
            );

        ret.ButtonsNew[idx] =
            x.Buttons[
                XINPUT_BUTTON_LEFT_SHOULDER
            ];

        // RB / R1
        idx = static_cast<int>(
            JOYPAD_BTN::R_BUTTON
            );

        ret.ButtonsNew[idx] =
            x.Buttons[
                XINPUT_BUTTON_RIGHT_SHOULDER
            ];

        // LT / L2
        idx = static_cast<int>(
            JOYPAD_BTN::L_TRIGGER
            );

        ret.ButtonsNew[idx] =
            x.LeftTrigger;

        // RT / R2
        idx = static_cast<int>(
            JOYPAD_BTN::R_TRIGGER
            );

        ret.ButtonsNew[idx] =
            x.RightTrigger;

        // L3
        idx = static_cast<int>(
            JOYPAD_BTN::L_STICK_PUSH
            );

        ret.ButtonsNew[idx] =
            x.Buttons[
                XINPUT_BUTTON_LEFT_THUMB
            ];

        // R3
        idx = static_cast<int>(
            JOYPAD_BTN::R_STICK_PUSH
            );

        ret.ButtonsNew[idx] =
            x.Buttons[
                XINPUT_BUTTON_RIGHT_THUMB
            ];

        // Xbox左スティック
        ret.AKeyLX =
            x.ThumbLX;

        ret.AKeyLY =
            x.ThumbLY;

        // Xbox右スティック
        ret.AKeyRX =
            x.ThumbRX;

        ret.AKeyRY =
            x.ThumbRY;

        return ret;
    }

    // ========================================
    // DualSense DirectInput処理
    // ========================================

    DINPUT_JOYSTATE d;
    ZeroMemory(
        &d,
        sizeof(d)
    );

    const int directInputResult =
        GetJoypadDirectInputState(
            inputType,
            &d
        );

    if (directInputResult != 0)
    {
#ifdef _DEBUG

        if (no == JOYPAD_NO::PAD1)
        {
            OutputDebugStringA(
                "PAD1 DirectInput failed.\n"
            );
        }

#endif

        return ret;
    }

    int idx = 0;

    // △
    idx = static_cast<int>(
        JOYPAD_BTN::TOP
        );

    ret.ButtonsNew[idx] =
        d.Buttons[3];

    // □
    idx = static_cast<int>(
        JOYPAD_BTN::LEFT
        );

    ret.ButtonsNew[idx] =
        d.Buttons[0];

    // ○
    idx = static_cast<int>(
        JOYPAD_BTN::RIGHT
        );

    ret.ButtonsNew[idx] =
        d.Buttons[2];

    // ×
    idx = static_cast<int>(
        JOYPAD_BTN::DOWN
        );

    ret.ButtonsNew[idx] =
        d.Buttons[1];

    // L1
    idx = static_cast<int>(
        JOYPAD_BTN::L_BUTTON
        );

    ret.ButtonsNew[idx] =
        d.Buttons[4];

    // R1
    idx = static_cast<int>(
        JOYPAD_BTN::R_BUTTON
        );

    ret.ButtonsNew[idx] =
        d.Buttons[5];

    // L2
    idx = static_cast<int>(
        JOYPAD_BTN::L_TRIGGER
        );

    ret.ButtonsNew[idx] =
        d.Buttons[6];

    // R2
    idx = static_cast<int>(
        JOYPAD_BTN::R_TRIGGER
        );

    ret.ButtonsNew[idx] =
        d.Buttons[7];

    // L3
    idx = static_cast<int>(
        JOYPAD_BTN::L_STICK_PUSH
        );

    ret.ButtonsNew[idx] =
        d.Buttons[10];

    // R3
    idx = static_cast<int>(
        JOYPAD_BTN::R_STICK_PUSH
        );

    ret.ButtonsNew[idx] =
        d.Buttons[11];

    // ========================================
    // DualSenseスティック
    // ========================================

    int leftX = 0;
    int leftY = 0;

    int rightX = 0;
    int rightY = 0;

    const int leftResult =
        GetJoypadAnalogInput(
            &leftX,
            &leftY,
            inputType
        );

    const int rightResult =
        GetJoypadAnalogInputRight(
            &rightX,
            &rightY,
            inputType
        );

    // 左スティック
    if (leftResult == 0)
    {
        ret.AKeyLX = leftX;
        ret.AKeyLY = leftY;
    }
    else
    {
        ret.AKeyLX = d.X;
        ret.AKeyLY = d.Y;
    }

    // 右スティック
    if (rightResult == 0)
    {
        ret.AKeyRX = rightX;
        ret.AKeyRY = rightY;
    }
    else
    {
        // DualSenseでは通常Z、Rz側
        ret.AKeyRX = d.Z;
        ret.AKeyRY = d.Rz;
    }

#ifdef _DEBUG

    if (no == JOYPAD_NO::PAD1)
    {
        static int stickDebugCounter = 0;

        stickDebugCounter++;

        if (stickDebugCounter >= 30)
        {
            stickDebugCounter = 0;

            char text[512];

            sprintf_s(
                text,
                sizeof(text),
                "PS: XInput=%d Direct=%d "
                "LeftResult=%d L=(%d,%d) "
                "RightResult=%d R=(%d,%d) "
                "Raw X=%d Y=%d Z=%d "
                "Rx=%d Ry=%d Rz=%d\n",
                xInputResult,
                directInputResult,
                leftResult,
                ret.AKeyLX,
                ret.AKeyLY,
                rightResult,
                ret.AKeyRX,
                ret.AKeyRY,
                d.X,
                d.Y,
                d.Z,
                d.Rx,
                d.Ry,
                d.Rz
            );

            OutputDebugStringA(text);
        }
    }

#endif

    return ret;
}

void InputManager::UpdateMouse()
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

        if (abs(mouseDiffX_) <= 1)
        {
            mouseDiffX_ = 0;
        }

        if (abs(mouseDiffY_) <= 1)
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
        pair.second.keyNew = (currentInput & pair.second.key) != 0;
        pair.second.keyTrgDown = pair.second.keyNew && !pair.second.keyOld;
        pair.second.keyTrgUp = !pair.second.keyNew && pair.second.keyOld;
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

bool InputManager::IsXInputPad(
    JOYPAD_NO no
) const
{
    XINPUT_STATE state;
    ZeroMemory(
        &state,
        sizeof(state)
    );

    const int result =
        GetJoypadXInputState(
            static_cast<int>(no),
            &state
        );

    // DualSenseと判定されている場合は、
    // XInput成功扱いでもDirectInputとする
    const JOYPAD_TYPE padType =
        static_cast<JOYPAD_TYPE>(
            GetJoypadType(
                static_cast<int>(no)
            )
            );

    if (padType ==
        JOYPAD_TYPE::DUAL_SENSE)
    {
        return false;
    }

    return result == 0;
}