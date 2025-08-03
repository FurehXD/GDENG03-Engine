#include <DX3D/Game/SceneCamera.h>
#include <cmath>
#include <algorithm>

using namespace dx3d;

SceneCamera::SceneCamera()
    : m_position(0.0f, 0.0f, -5.0f)
    , m_worldUp(0.0f, 1.0f, 0.0f)
    , m_yaw(0.0f)     
    , m_pitch(0.0f)
    , m_roll(0.0f)
{
    updateVectors();
    updateViewMatrix();
}

SceneCamera::SceneCamera(const Vector3& position, const Vector3& target, const Vector3& up)
    : m_position(position)
    , m_worldUp(up)
    , m_roll(0.0f)
{
    lookAt(target);
}

void SceneCamera::moveForward(float distance)
{
    m_position += m_forward * distance;
    updateViewMatrix();
}

void SceneCamera::moveBackward(float distance)
{
    m_position -= m_forward * distance;
    updateViewMatrix();
}

void SceneCamera::moveLeft(float distance)
{
    m_position -= m_right * distance;
    updateViewMatrix();
}

void SceneCamera::moveRight(float distance)
{
    m_position += m_right * distance;
    updateViewMatrix();
}

void SceneCamera::moveUp(float distance)
{
    m_position += m_worldUp * distance;
    updateViewMatrix();
}

void SceneCamera::moveDown(float distance)
{
    m_position -= m_worldUp * distance;
    updateViewMatrix();
}

void SceneCamera::rotateYaw(float angle)
{
    m_yaw += angle;
    updateVectors();
    updateViewMatrix();
}

void SceneCamera::rotatePitch(float angle)
{
    m_pitch += angle;

    // Clamp pitch to avoid gimbal lock
    const float maxPitch = 1.5533f; // ~89 degrees
    m_pitch = std::max(-maxPitch, std::min(maxPitch, m_pitch));

    updateVectors();
    updateViewMatrix();
}

void SceneCamera::rotateRoll(float angle)
{
    m_roll += angle;
    updateVectors();
    updateViewMatrix();
}

void SceneCamera::onMouseMove(float deltaX, float deltaY, float sensitivity)
{
    // Yaw rotation (left/right)
    rotateYaw(deltaX * sensitivity);

    // Pitch rotation (up/down) - inverted for natural feel
    rotatePitch(-deltaY * sensitivity);
}

void SceneCamera::update()
{
    updateViewMatrix();
}

void SceneCamera::setPosition(const Vector3& position)
{
    m_position = position;
    updateViewMatrix();
}

Vector3 SceneCamera::getForward()
{
    Vector3 forward;

    forward.x = cosf(m_pitch) * sinf(m_yaw);
    forward.y = sinf(m_pitch);
    forward.z = cosf(m_pitch) * cosf(m_yaw);

    return forward;
    //return forward.normalized(); // if you have a normalize function
}

void SceneCamera::lookAt(const Vector3& target)
{
    // Calculate direction from position to target
    Vector3 direction;
    direction.x = target.x - m_position.x;
    direction.y = target.y - m_position.y;
    direction.z = target.z - m_position.z;

    // Normalize direction
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    if (length > 0.0001f)
    {
        direction.x /= length;
        direction.y /= length;
        direction.z /= length;
    }

    // Calculate pitch from Y component
    m_pitch = std::asin(direction.y);

    // Calculate yaw from X and Z components
    m_yaw = std::atan2(direction.x, direction.z);

    updateVectors();
    updateViewMatrix();
}

void SceneCamera::updateVectors()
{
    // Calculate forward vector from yaw and pitch
    float cosPitch = std::cos(m_pitch);
    float sinPitch = std::sin(m_pitch);
    float cosYaw = std::cos(m_yaw);
    float sinYaw = std::sin(m_yaw);

    // Forward vector in DirectX left-handed coordinate system
    m_forward.x = sinYaw * cosPitch;
    m_forward.y = sinPitch;
    m_forward.z = cosYaw * cosPitch;

    // Calculate right vector (cross product of world up and forward)
    Vector3 worldUp = m_worldUp;

    m_right.x = worldUp.y * m_forward.z - worldUp.z * m_forward.y;
    m_right.y = worldUp.z * m_forward.x - worldUp.x * m_forward.z;
    m_right.z = worldUp.x * m_forward.y - worldUp.y * m_forward.x;

    // Normalize right vector
    float length = std::sqrt(m_right.x * m_right.x + m_right.y * m_right.y + m_right.z * m_right.z);
    if (length > 0.0f)
    {
        m_right.x /= length;
        m_right.y /= length;
        m_right.z /= length;
    }

    // Calculate up vector (cross product of forward and right)
    m_up.x = m_forward.y * m_right.z - m_forward.z * m_right.y;
    m_up.y = m_forward.z * m_right.x - m_forward.x * m_right.z;
    m_up.z = m_forward.x * m_right.y - m_forward.y * m_right.x;

    // Normalize up vector
    length = std::sqrt(m_up.x * m_up.x + m_up.y * m_up.y + m_up.z * m_up.z);
    if (length > 0.0f)
    {
        m_up.x /= length;
        m_up.y /= length;
        m_up.z /= length;
    }
}

void SceneCamera::updateViewMatrix()
{
    Vector3 target = m_position + m_forward;
    m_viewMatrix = Matrix4x4::CreateLookAtLH(m_position, target, m_up);
}