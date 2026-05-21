#pragma once
#include "Scene.h"
class KeyconfigScene :
    public Scene
{
private:
    int frame_;
    Input& input_;
    using UpdateFunc_t = void(KeyconfigScene::*)(Input& input);
    using DrawFunc_t = void(KeyconfigScene::*)();

    UpdateFunc_t update_;

    DrawFunc_t draw_;

    void AppearUpdate(Input&);// キーコンフィグ枠表示時のUpdate
    void NormalUpdate(Input&);// 通常Update
    void EditingUpdate(Input&);// キーを固定して編集中Update
    void DisappearUpdate(Input&);

    void ExpandDraw();
    void NormalDraw();
    void EditingDraw();

    void DrawFrame(float rate);
public:
    KeyconfigScene(SceneController& controller, Input& input);
    void Update(Input& input);
    void Draw();

};

