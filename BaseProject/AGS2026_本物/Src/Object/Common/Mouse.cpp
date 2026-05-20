#include "Mouse.h"

Mouse::Mouse() : x_(0), y_(0), diffX_(0), diffY_(0), mouseInput_(0), prevMouseInput_(0) {}

void Mouse::Update() {
    prevMouseInput_ = mouseInput_;
    mouseInput_ = GetMouseInput();

    int nowX, nowY;
    GetMousePoint(&nowX, &nowY);

    // 移動量の計算
    diffX_ = nowX - x_;
    diffY_ = nowY - y_;

    // 現在の座標を更新
    x_ = nowX;
    y_ = nowY;

    // FPS視点などの場合、マウスを画面中央に戻す処理が必要になることがあります
    // SetMousePoint(640 / 2, 480 / 2); // 必要に応じて
}

bool Mouse::IsPress(int button) const {
    return (mouseInput_ & button) != 0;
}

bool Mouse::IsTrg(int button) const {
    return ((mouseInput_ & button) != 0) && ((prevMouseInput_ & button) == 0);
}