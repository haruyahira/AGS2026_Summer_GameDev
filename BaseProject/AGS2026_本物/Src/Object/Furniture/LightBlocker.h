#pragma once

#include <DxLib.h>

class LightBlocker
{
public:

    virtual ~LightBlocker() {}

    virtual VECTOR GetPos() const = 0;

    virtual VECTOR GetAxisX() const = 0;

    virtual VECTOR GetAxisZ() const = 0;

    virtual VECTOR GetHalfSize() const = 0;

    virtual void DrawDebug(unsigned int color) const = 0;
};