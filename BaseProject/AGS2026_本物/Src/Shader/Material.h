#pragma once
#include <DxLib.h>

class Material
{
public:

    VECTOR diffuse_ =
        VGet(1, 1, 1);

    VECTOR ambient_ =
        VGet(0.05f, 0.05f, 0.05f);

    VECTOR specular_ =
        VGet(1, 1, 1);

    VECTOR emission_ =
        VGet(0, 0, 0);

    float shininess_ = 32.0f;
};