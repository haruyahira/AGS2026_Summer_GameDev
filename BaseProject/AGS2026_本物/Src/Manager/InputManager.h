#pragma once

#include <DxLib.h>
#include <unordered_map>
#include "../Utility/AsoUtility.h"

class InputManager
{
public:

    // ジョイパッド番号
    enum class JOYPAD_NO
    {
        KEY_PAD1 = DX_INPUT_KEY_PAD1,
        PAD1 = DX_INPUT_PAD1,
        PAD2 = DX_INPUT_PAD2,
        PAD3 = DX_INPUT_PAD3,
        PAD4 = DX_INPUT_PAD4,
    };

    // ジョイパッド種類
    enum class JOYPAD_TYPE
    {
        OTHER = 0,
        XBOX_360 = DX_PADTYPE_XBOX_360,
        XBOX_ONE = DX_PADTYPE_XBOX_ONE,
        DUAL_SHOCK_4 = DX_PADTYPE_DUAL_SHOCK_4,
        DUAL_SENSE = DX_PADTYPE_DUAL_SENSE,
    };

    // パッドボタン
    enum class JOYPAD_BTN
    {
        TOP,            // ▲ / Y
        LEFT,           // □ / X
        RIGHT,          // ○ / B
        DOWN,           // × / A

        L_TRIGGER,      // L2 / LT
        R_TRIGGER,      // R2 / RT

        L_STICK_PUSH,   // L3
        R_STICK_PUSH,   // R3

        MAX
    };

    struct Info
    {
        int key;
        bool keyOld;
        bool keyNew;
        bool keyTrgDown;
        bool keyTrgUp;
    };

    struct MouseInfo
    {
        int key;
        bool keyOld;
        bool keyNew;
        bool keyTrgDown;
        bool keyTrgUp;
    };

    struct JOYPAD_IN_STATE
    {
        int ButtonsOld[static_cast<int>(JOYPAD_BTN::MAX)];
        int ButtonsNew[static_cast<int>(JOYPAD_BTN::MAX)];

        bool IsOld[static_cast<int>(JOYPAD_BTN::MAX)];
        bool IsNew[static_cast<int>(JOYPAD_BTN::MAX)];
        bool IsTrgDown[static_cast<int>(JOYPAD_BTN::MAX)];
        bool IsTrgUp[static_cast<int>(JOYPAD_BTN::MAX)];

        int AKeyLX;
        int AKeyLY;
        int AKeyRX;
        int AKeyRY;
    };

public:

    static void CreateInstance(void);
    static InputManager& GetInstance(void);
    static void Destroy(void);

    void Init(void);
    void Update(void);
    void Reset(void);
    void Clear(void);

    void Add(int key);

    bool IsNew(int key) const;
    bool IsPress(int key) const;
    bool IsTrgDown(int key) const;
    bool IsTrgUp(int key) const;

    Vector2 GetMousePos(void) const;
    int GetMouse(void) const;

    bool IsClickMouseLeft(void) const;
    bool IsClickMouseRight(void) const;
    bool IsTrgMouseLeft(void) const;
    bool IsTrgMouseRight(void) const;

    int GetMouseDiffX(void) const;
    int GetMouseDiffY(void) const;

    void SetFixMouse(bool isFix);
    bool IsFixMouse(void) const;

    JOYPAD_TYPE GetJPadType(JOYPAD_NO no);
    DINPUT_JOYSTATE GetJPadDInputState(JOYPAD_NO no);
    XINPUT_STATE GetJPadXInputState(JOYPAD_NO no);

    bool IsPadBtnNew(JOYPAD_NO no, JOYPAD_BTN btn) const;
    bool IsPadBtnTrgDown(JOYPAD_NO no, JOYPAD_BTN btn) const;
    bool IsPadBtnTrgUp(JOYPAD_NO no, JOYPAD_BTN btn) const;

    int GetPadAKeyLX(JOYPAD_NO no) const;
    int GetPadAKeyLY(JOYPAD_NO no) const;
    int GetPadAKeyRX(JOYPAD_NO no) const;
    int GetPadAKeyRY(JOYPAD_NO no) const;

    int GetPadButtonValue(JOYPAD_NO no, JOYPAD_BTN btn) const;

private:

    InputManager(void);
    ~InputManager(void) = default;

    const Info& Find(int key) const;
    const MouseInfo& FindMouse(int key) const;

    void UpdateMouse(void);

    int GetPadIndex(JOYPAD_NO no) const;
    void SetJPadInState(JOYPAD_NO jpNo);
    JOYPAD_IN_STATE GetJPadInputState(JOYPAD_NO no);

private:

    static InputManager* instance_;

    std::unordered_map<int, Info> keyInfos_;
    std::unordered_map<int, MouseInfo> mouseInfos_;

    Info infoEmpty_;
    MouseInfo mouseInfoEmpty_;

    int mouseInput_;

    int mouseX_;
    int mouseY_;
    int mouseDiffX_;
    int mouseDiffY_;

    bool isFixMouse_;

    Vector2 mousePos_;

    DINPUT_JOYSTATE joyDInState_;
    XINPUT_STATE joyXInState_;

    JOYPAD_IN_STATE padInfos_[5];
   
};