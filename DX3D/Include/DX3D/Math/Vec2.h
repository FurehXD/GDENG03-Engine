#pragma once
#include <cmath>

namespace dx3d
{
    struct Vec2
    {
        float x, y;

        Vec2() : x(0), y(0) {}
        Vec2(float x, float y) : x(x), y(y) {}

        Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
        Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
        Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }
        Vec2 operator/(float scalar) const { return Vec2(x / scalar, y / scalar); }

        Vec2& operator+=(const Vec2& other) { x += other.x; y += other.y; return *this; }
        Vec2& operator-=(const Vec2& other) { x -= other.x; y -= other.y; return *this; }
        Vec2& operator*=(float scalar) { x *= scalar; y *= scalar; return *this; }
        Vec2& operator/=(float scalar) { x /= scalar; y /= scalar; return *this; }

        float length() const { return std::sqrt(x * x + y * y); }
        float lengthSquared() const { return x * x + y * y; }

        Vec2 normalized() const
        {
            float len = length();
            return len > 0.0f ? Vec2(x / len, y / len) : Vec2(0, 0);
        }

        static Vec2 lerp(const Vec2& a, const Vec2& b, float t)
        {
            return Vec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
        }
    };

    struct Vec4
    {
        float x, y, z, w;

        Vec4() : x(0), y(0), z(0), w(0) {}
        Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

        Vec4 operator+(const Vec4& other) const
        {
            return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
        }

        Vec4 operator*(float scalar) const
        {
            return Vec4(x * scalar, y * scalar, z * scalar, w * scalar);
        }

        static Vec4 lerp(const Vec4& a, const Vec4& b, float t)
        {
            return Vec4(
                a.x + (b.x - a.x) * t,
                a.y + (b.y - a.y) * t,
                a.z + (b.z - a.z) * t,
                a.w + (b.w - a.w) * t
            );
        }
    };
}