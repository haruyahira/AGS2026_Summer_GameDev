#include "CameraShake.h"
#include <cstdlib>

static int shakeTimer = 0;
static float shakePower = 0.0f;

void StartCameraShake(float power, int time)
{
    shakePower = power;
    shakeTimer = time;
}

VECTOR GetCameraShakeOffset()
{
    if (shakeTimer <= 0) return VGet(0, 0, 0);

    shakeTimer--;

    float dx = (rand() % 200 - 100) / 100.0f * shakePower;
    float dy = (rand() % 200 - 100) / 100.0f * shakePower;
    float dz = (rand() % 200 - 100) / 100.0f * shakePower;

    return VGet(dx, dy, dz);
}
