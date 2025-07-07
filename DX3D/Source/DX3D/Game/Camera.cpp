#include <DX3D/Game/Camera.h>
#include <cmath>
#include <algorithm>

using namespace dx3d;

Camera::Camera()
    : m_position(0.0f, 0.0f, -5.0f)
    , m_worldUp(0.0f, 1.0f, 0.0f)
    , m_yaw(0.0f)     
    , m_pitch(0.0f)
    , m_roll(0.0f)
{
    updateVectors();
    updateViewMatrix();
}

Camera::Camera(const Vector3& position, const Vector3& target, const Vector3& up)
    : m_position(position)
    , m_worldUp(up)
    , m_roll(0.0f)
{
    lookAt(target);
}

void Camera::moveForward(float distance)
{
    m_position += m_forward * distance;
    updateViewMatrix();
}

void Camera::moveBackward(float distance)
{
    m_position -= m_forward * distance;
    updateViewMatrix();
}

void Camera::moveLeft(float distance)
{
    m_position -= m_right * distance;
    updateViewMatrix();
}

void Camera::moveRight(float distance)
{
    m_position += m_right * distance;
    updateViewMatrix();
}

void Camera::moveUp(float distance)
{
    m_position += m_worldUp * distance;
    updateViewMatrix();
}

void Camera::moveDown(float distance)
{
    m_position -= m_worldUp * distance;
    updateViewMatrix();
}

void Camera::rotateYaw(float angle)
{
    m_yaw += angle;
    updateVectors();
    updateViewMatrix();
}

void Camera::rotatePitch(float angle)
{
    m_pitch += angle;

    // Clamp pitch to avoid gimbal lock
    const float maxPitch = 1.5533f; // ~89 degrees
    m_pitch = std::max(-maxPitch, std::min(maxPitch, m_pitch));

    updateVectors();
    updateViewMatrix();
}

void Camera::rotateRoll(float angle)
{
    m_roll += angle;
    updateVectors();
    updateViewMatrix();
}

void Camera::onMouseMove(float deltaX, float deltaY, float sensitivity)
{
    // Yaw rotation (left/right)
    rotateYaw(deltaX * sensitivity);

    // Pitch rotation (up/down) - inverted for natural feel
    rotatePitch(-deltaY * sensitivity);
}

void Camera::update()
{
    updateViewMatrix();
}

void Camera::setPosition(const Vector3& position)
{
    m_position = position;
    updateViewMatrix();
}

Vector3 Camera::getForward()
{
    Vector3 forward;

    forward.x = cosf(m_pitch) * sinf(m_yaw);
    forward.y = sinf(m_pitch);
    forward.z = cosf(m_pitch) * cosf(m_yaw);

    return forward;
    //return forward.normalized(); // if you have a normalize function
}

void Camera::lookAt(const Vector3& target)
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

void Camera::updateVectors()
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
    Vector3 tempUp;
    tempUp.x = m_forward.y * m_right.z - m_forward.z * m_right.y;
    tempUp.y = m_forward.z * m_right.x - m_forward.x * m_right.z;
    tempUp.z = m_forward.x * m_right.y - m_forward.y * m_right.x;

    // Normalize temp up vector
    length = std::sqrt(tempUp.x * tempUp.x + tempUp.y * tempUp.y + tempUp.z * tempUp.z);
    if (length > 0.0f)
    {
        tempUp.x /= length;
        tempUp.y /= length;
        tempUp.z /= length;
    }

    float cosRoll = std::cos(m_roll);
    float sinRoll = std::sin(m_roll);

    m_up.x = tempUp.x * cosRoll + m_right.x * sinRoll;
    m_up.y = tempUp.y * cosRoll + m_right.y * sinRoll;
    m_up.z = tempUp.z * cosRoll + m_right.z * sinRoll;

    m_right.x = m_right.x * cosRoll - tempUp.x * sinRoll;
    m_right.y = m_right.y * cosRoll - tempUp.y * sinRoll;
    m_right.z = m_right.z * cosRoll - tempUp.z * sinRoll;
}

void Camera::updateViewMatrix()
{
    Vector3 target = m_position + m_forward;
    m_viewMatrix = Matrix4x4::CreateLookAtLH(m_position, target, m_up);
}