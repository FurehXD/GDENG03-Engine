#pragma once
#include <DX3D/Core/Core.h>
#include <cmath>
#include <DirectXMath.h>

namespace dx3d
{

    struct Vector2
    {
        float x, y;
        Vector2() : x(0), y(0) {}
        Vector2(float x, float y) : x(x), y(y) {}
    };

    struct Vector3
    {
        float x, y, z;

        Vector3() : x(0), y(0), z(0) {}
        Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

        Vector3 operator+(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
        Vector3 operator-(const Vector3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
        Vector3 operator*(float scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }

        Vector3& operator+=(const Vector3& other) { x += other.x; y += other.y; z += other.z; return *this; }
        Vector3& operator-=(const Vector3& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
        Vector3& operator*=(float scalar) { x *= scalar; y *= scalar; z *= scalar; return *this; }

        // Calculates the cross product of two vectors.
        static Vector3 cross(const Vector3& v1, const Vector3& v2)
        {
            return Vector3(
                v1.y * v2.z - v1.z * v2.y,
                v1.z * v2.x - v1.x * v2.z,
                v1.x * v2.y - v1.y * v2.x
            );
        }

        // Calculates the cross product of this vector with another.
        Vector3 cross(const Vector3& other) const
        {
            return Vector3(
                y * other.z - z * other.y,
                z * other.x - x * other.z,
                x * other.y - y * other.x
            );
        }
    };

    struct Vector4
    {
        float x, y, z, w;

        Vector4() : x(0), y(0), z(0), w(0) {}
        Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    };

    struct Matrix4x4
    {
        float m[4][4];

        Matrix4x4()
        {
            // Initialize to identity matrix
            for (int i = 0; i < 4; i++)
                for (int j = 0; j < 4; j++)
                    m[i][j] = (i == j) ? 1.0f : 0.0f;
        }

        // Matrix multiplication
        Matrix4x4 operator*(const Matrix4x4& other) const;

        // Create transformation matrices
        static Matrix4x4 CreateTranslation(const Vector3& translation);
        static Matrix4x4 CreateRotationX(float angle);
        static Matrix4x4 CreateRotationY(float angle);
        static Matrix4x4 CreateRotationZ(float angle);
        static Matrix4x4 CreateScale(const Vector3& scale);
        static Matrix4x4 CreatePerspectiveFovLH(float fovY, float aspectRatio, float nearPlane, float farPlane);
        static Matrix4x4 CreateLookAtLH(const Vector3& eye, const Vector3& target, const Vector3& up);

        // Convert to DirectXMath for calculations
        DirectX::XMMATRIX toXMMatrix() const;
        static Matrix4x4 fromXMMatrix(const DirectX::XMMATRIX& xmMatrix);
    };

    // Constant buffer structure for transformation matrices
    struct TransformationMatrices
    {
        Matrix4x4 world;
        Matrix4x4 view;
        Matrix4x4 projection;
    };
}