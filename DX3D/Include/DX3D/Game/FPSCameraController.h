#pragma once
#include <DX3D/Math/Math.h>

namespace dx3d
{
    class SceneCamera;
    class Input;

    class FPSCameraController
    {
    public:
        FPSCameraController();
        ~FPSCameraController();

        void setCamera(SceneCamera* camera) { m_camera = camera; }
        void setMovementSpeed(float speed) { m_movementSpeed = speed; }
        void setMouseSensitivity(float sensitivity) { m_mouseSensitivity = sensitivity; }

        void update(float deltaTime);
        void handleInput(float deltaTime);

        void enable() { m_enabled = true; }
        void disable() { m_enabled = false; }
        bool isEnabled() const { return m_enabled; }

        void lockCursor(bool lock) { m_cursorLocked = lock; }
        bool isCursorLocked() const { return m_cursorLocked; }

    private:
        SceneCamera* m_camera;
        float m_movementSpeed;
        float m_mouseSensitivity;
        bool m_enabled;
        bool m_cursorLocked;

        Vector3 m_savedCameraPosition;
        float m_savedCameraYaw;
        float m_savedCameraPitch;
    };
}