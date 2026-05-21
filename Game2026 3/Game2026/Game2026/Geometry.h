#pragma once
struct Vector2 {

	float x, y;
	/// <summary>
	/// ベクトルの長さを返します
	
	float Length() const;
	/// </summary>
	/// 正規化する
	/// </summary>
	void Normalize();

	/// <summary>
	/// 正規化したベクトルを返す
	/// </summary>
	/// <returns>正規化したベクトル</returns>
	Vector2 Normalied() const;

	Vector2 operator*(float scale)const;
	Vector2 operator/(float div)const;

	Vector2 operator-()const;
	Vector2 operator+(const Vector2& val)const;
	Vector2 operator-(const Vector2& val)const;	

	void operator+=(const Vector2& val);
	void operator-=(const Vector2& val);
	void operator*=(float scale);
	void operator/=(float div);

};
using Position2 = Vector2;

struct Size{
	float w, h;
};

