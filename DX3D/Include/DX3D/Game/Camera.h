#pragma once
#include <DX3D/Core/Core.h>
#include <DX3D/Math/Math.h>

namespace dx3d
{
    class Camera
    {
    public:
        Camera();
        Camera(const Vector3& position, const Vector3& target, const Vector3& up = Vector3(0.0f, 1.0f, 0.0f));
        ~Camera() = default;

        // Movement methods
        void moveForward(float distance);
        void moveBackward(float distance);
        void moveLeft(float distance);
        void moveRight(float distance);
        void moveUp(float distance);
        void moveDown(float distance);

        // Rotation methods (in radians)
        void rotateYaw(float angle);   // Rotate around Y axis (left/right)
        void rotatePitch(float angle); // Rotate around X axis (up/down)
        void rotateRoll(float angle);  // Rotate around Z axis (tilt)

        // Mouse look
        void onMouseMove(float deltaX, float deltaY, float sensitivity = 0.001f);

        // Update camera matrices
        void update();

        // Getters
        const Matrix4x4& getViewMatrix() const { return m_viewMatrix; }
        const Vector3& getPosition() const { return m_position; }
        const Vector3& getForward() const { return m_forward; }
        const Vector3& getRight() const { return m_right; }
        const Vector3& getUp() const { return m_up; }
        float getYaw() const { return m_yaw; }
        float getPitch() const { return m_pitch; }
        float getRoll() const { return m_roll; }

        Vector3 getForward();

        // Setters
        void setPosition(const Vector3& position);
        void lookAt(const Vector3& target);

    private:
        void updateVectors();
        void updateViewMatrix();

    private:
        // Camera position and orientation
        Vector3 m_position;
        Vector3 m_forward;  // Direction camera is looking
        Vector3 m_right;    // Right vector
        Vector3 m_up;       // Up vector
        Vector3 m_worldUp;  // World up vector (usually Y-axis)

        // Euler angles (in radians)
        float m_yaw;    // Rotation around Y axis
        float m_pitch;  // Rotation around X axis
        float m_roll;   // Rotation around Z axis

        // View matrix
        Matrix4x4 m_viewMatrix;
    };
}