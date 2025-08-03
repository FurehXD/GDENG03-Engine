#include <DX3D/Game/FPSCameraController.h>
#include <DX3D/Game/SceneCamera.h>
#include <DX3D/Input/Input.h>
#include <Windows.h>

using namespace dx3d;

FPSCameraController::FPSCameraController()
    : m_camera(nullptr)
    , m_movementSpeed(5.0f)
    , m_mouseSensitivity(0.3f)
    , m_enabled(false)
    , m_cursorLocked(false)
    , m_savedCameraPosition(0, 0, 0)
    , m_savedCameraYaw(0.0f)
    , m_savedCameraPitch(0.0f)
{
}

FPSCameraController::~FPSCameraController()
{
}

void FPSCameraController::update(float deltaTime)
{
    if (!m_enabled || !m_camera)
        return;

    handleInput(deltaTime);
}

void FPSCameraController::handleInput(float deltaTime)
{
    auto& input = Input::getInstance();

    float moveSpeed = m_movementSpeed * deltaTime;

    if (input.isKeyPressed(KeyCode::W)) m_camera->moveForward(moveSpeed);
    if (input.isKeyPressed(KeyCode::S)) m_camera->moveBackward(moveSpeed);
    if (input.isKeyPressed(KeyCode::A)) m_camera->moveLeft(moveSpeed);
    if (input.isKeyPressed(KeyCode::D)) m_camera->moveRight(moveSpeed);
    if (input.isKeyPressed(KeyCode::Q)) m_camera->moveDown(moveSpeed);
    if (input.isKeyPressed(KeyCode::E)) m_camera->moveUp(moveSpeed);

    if (m_cursorLocked)
    {
        float mouseDeltaX = static_cast<float>(input.getMouseDeltaX());
        float mouseDeltaY = static_cast<float>(input.getMouseDeltaY());

        if (mouseDeltaX != 0.0f || mouseDeltaY != 0.0f)
        {
            m_camera->onMouseMove(mouseDeltaX, mouseDeltaY, m_mouseSensitivity * 0.01f);
        }

        RECT clientRect;
        HWND hwnd = GetActiveWindow();
        if (GetClientRect(hwnd, &clientRect))
        {
            POINT center;
            center.x = (clientRect.right - clientRect.left) / 2;
            center.y = (clientRect.bottom - clientRect.top) / 2;
            ClientToScreen(hwnd, &center);
            SetCursorPos(center.x, center.y);
        }
    }
}