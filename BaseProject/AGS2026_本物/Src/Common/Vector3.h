#pragma once

struct Vector3 {
    float x, y, z;

    // コンストラクタ（初期化が楽になります）
    Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    // 必要に応じて足し算などの演算子も追加できます
    Vector3 operator+(const Vector3& v) const {
        return Vector3(x + v.x, y + v.y, z + v.z);
    }
};