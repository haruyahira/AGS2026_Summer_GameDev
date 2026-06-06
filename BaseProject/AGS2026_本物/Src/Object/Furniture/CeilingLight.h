#pragma once

#include "../Common/Transform.h"
#include <memory>
#include <DxLib.h>

class CeilingLight
{
public:

    CeilingLight(const Transform* trans);

    ~CeilingLight();

    void Init();

    void Update();

    void Draw();

    void DrawGlow();

    VECTOR GetPos() const;

private:

    std::shared_ptr<Transform> trans_;
};
