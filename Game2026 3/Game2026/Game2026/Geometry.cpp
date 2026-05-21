#include "Geometry.h"
#include <cmath>
#include<cassert>
float Vector2::Length() const
{
    return std::hypot(x,y);
}

void Vector2::Normalize()
{
	auto len = Length();
	if (len == 0.0f) {
		return;
	}

	x /= len;
	y /= len;
}

Vector2 Vector2::Normalied() const
{
	auto len = Length();
	if (len == 0.0f) {
		return{};
	}

	return{ x / len, y / len };
}

Vector2 Vector2::operator*(float scale) const
{
	return { x * scale, y * scale };
}

Vector2 Vector2::operator/(float div) const
{

	assert(div != 0.0f);
	return {x/div,y/div};
}

Vector2 Vector2::operator-() const
{
	return {-x,-y};
}

Vector2 Vector2::operator+(const Vector2& val) const
{
	return { x + val.x,y + val.y };
}

Vector2 Vector2::operator-(const Vector2& val) const
{
	return { x - val.x,y - val.y };
}

void Vector2::operator+=(const Vector2& val)
{
	x += val.x;
	y += val.y;
}

void Vector2::operator-=(const Vector2& val)
{
	x -= val.x;
	y -= val.y;
}

void Vector2::operator*=(float scale)
{
	x *= scale;
	y *= scale;
}

void Vector2::operator/=(float div)
{
}
