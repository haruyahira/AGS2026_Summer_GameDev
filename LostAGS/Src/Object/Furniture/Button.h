#pragma once

#include "Furniture.h"

class Transform;

class Button : public Furniture
{
public:
    Button(const Transform* trans);
    virtual ~Button(void);

    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;
};