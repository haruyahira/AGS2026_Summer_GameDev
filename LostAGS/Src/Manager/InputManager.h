#pragma once
#include <map>
#include <DxLib.h>
#include "../Common/Vector2.h"

class InputManager
{
public:

    // ゲームコントローラーの認識番号
    // DxLib定数、DX_INPUT_PAD1等に対応
    enum class JOYPAD_NO
    {
        KEY_PAD1 = DX_INPUT_KEY_PAD1,
        PAD1 = DX_INPUT_PAD1,
        PAD2 = DX_INPUT_PAD2,
        PAD3 = DX_INPUT_PAD3,
        PAD4 = DX_INPUT_PAD4,
        INPUT_KEY = DX_INPUT_KEY
    };

    // ゲームコントローラータイプ
    enum class JOYPAD_TYPE
    {
        OTHER = 0,
        XBOX_360,
        XBOX_ONE,
        DUAL_SHOCK_4,
        DUAL_SENSE,
        SWITCH_JOY_CON_L,
        SWITCH_JOY_CON_R,
        SWITCH_PRO_CTRL,
        MAX
    };

    // ゲームコントローラーボタン
    enum class JOYPAD_BTN
    {

        TOP,
        LEFT,
        RIGHT,
        DOWN,

        L_BUTTON,
        R_BUTTON,

        L_TRIGGER,
        R_TRIGGER,

        L_STICK_PUSH,
        R_STICK_PUSH,

        MAX

    };

    // ゲームコントローラーの入力情報
    struct JOYPAD_IN_STATE
    {
        unsigned char ButtonsOld[static_cast<int>(JOYPAD_BTN::MAX)];
        unsigned char ButtonsNew[static_cast<int>(JOYPAD_BTN::MAX)];

        bool IsOld[static_cast<int>(JOYPAD_BTN::MAX)];
        bool IsNew[static_cast<int>(JOYPAD_BTN::MAX)];
        bool IsTrgDown[static_cast<int>(JOYPAD_BTN::MAX)];
        bool IsTrgUp[static_cast<int>(JOYPAD_BTN::MAX)];

        int AKeyLX;
        int AKeyLY;
        int AKeyRX;
        int AKeyRY;
    };

    static void CreateInstance(void);
    static InputManager& GetInstance(void);

    void Init(void);
    void Update(void);
    void Destroy(void);

    void Add(int key);
    void Clear(void);

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

    int GetMouseDiffX() const { return mouseDiffX_; }
    int GetMouseDiffY() const { return mouseDiffY_; }

    void SetFixMouse(bool fix) { isFixMouse_ = fix; }

    // コントローラの入力情報を取得
    JOYPAD_IN_STATE GetJPadInputState(JOYPAD_NO no);

    bool IsPadBtnNew(JOYPAD_NO no, JOYPAD_BTN btn) const;
    bool IsPadBtnTrgDown(JOYPAD_NO no, JOYPAD_BTN btn) const;
    bool IsPadBtnTrgUp(JOYPAD_NO no, JOYPAD_BTN btn) const;

    int GetPadAKeyLX(JOYPAD_NO no) const;
    int GetPadAKeyLY(JOYPAD_NO no) const;
    int GetPadAKeyRX(JOYPAD_NO no) const;
    int GetPadAKeyRY(JOYPAD_NO no) const;

    // デバッグ確認用：ボタンの生値を取得
    int GetPadButtonValue(JOYPAD_NO no, JOYPAD_BTN btn) const;
    DINPUT_JOYSTATE GetJPadDInputState(JOYPAD_NO no);
    JOYPAD_TYPE GetJPadType(JOYPAD_NO no);


private:

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

    int mouseX_;
    int mouseY_;
    int mouseDiffX_;
    int mouseDiffY_;
    bool isFixMouse_;

    DINPUT_JOYSTATE joyDInState_;
    XINPUT_STATE joyXInState_;

    static InputManager* instance_;

    std::map<int, InputManager::Info> keyInfos_;
    InputManager::Info infoEmpty_;

    std::map<int, InputManager::MouseInfo> mouseInfos_;
    InputManager::MouseInfo mouseInfoEmpty_;

    Vector2 mousePos_;
    int mouseInput_;

    JOYPAD_IN_STATE padInfos_[5];

    InputManager(void);
    InputManager(const InputManager& manager) = default;
    ~InputManager(void) = default;

    const InputManager::Info& Find(int key) const;
    const InputManager::MouseInfo& FindMouse(int key) const;

    
    XINPUT_STATE GetJPadXInputState(JOYPAD_NO no);

    void SetJPadInState(JOYPAD_NO jpNo);

    // padInfos_ 用の添字に変換する
    int GetPadIndex(JOYPAD_NO no) const;

    void UpdateMouse();
};