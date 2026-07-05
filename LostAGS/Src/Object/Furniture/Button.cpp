#include "Button.h"

Button::Button(const Transform* trans)
    : Furniture(NAME::BUTTON, trans)
{
}

Button::~Button(void)
{
}

void Button::Init(void)
{
    colliders_.clear();

    // 必要なら後でボタン用の当たり判定を追加する
}

void Button::Update(void)
{
}

void Button::Draw(void)
{
    trans_.Update();

    MV1DrawModel(trans_.modelId);
}