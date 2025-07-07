#include <DX3D/Graphics/Primitives/CameraObject.h>

using namespace dx3d;

CameraObject::CameraObject()
    : AGameObject()
{
    m_camera = std::make_unique<Camera>();
    syncCameraTransform();
}

CameraObject::CameraObject(const Vector3& position, const Vector3& rotation)
    : AGameObject(position, rotation)
{
    m_camera = std::make_unique<Camera>();
    syncCameraTransform();
}

void CameraObject::update(float deltaTime)
{
    AGameObject::update(deltaTime);
    syncCameraTransform();
}

void CameraObject::syncCameraTransform()
{
    m_camera->setPosition(m_transform.position);

    float currentYaw = m_camera->getYaw();
    float currentPitch = m_camera->getPitch();
    float currentRoll = m_camera->getRoll();

    float deltaYaw = m_transform.rotation.y - currentYaw;
    float deltaPitch = m_transform.rotation.x - currentPitch;
    float deltaRoll = m_transform.rotation.z - currentRoll;

    if (std::abs(deltaYaw) > 0.001f)
        m_camera->rotateYaw(deltaYaw);
    if (std::abs(deltaPitch) > 0.001f)
        m_camera->rotatePitch(deltaPitch);
    if (std::abs(deltaRoll) > 0.001f)
        m_camera->rotateRoll(deltaRoll);

}

Matrix4x4 CameraObject::getProjectionMatrix(float aspectRatio) const
{
    return Matrix4x4::CreatePerspectiveFovLH(m_fov, aspectRatio, m_nearPlane, m_farPlane);
}

void CameraObject::alignWithView(const Camera& viewCamera)
{
    m_transform.position = viewCamera.getPosition();
    m_transform.rotation.y = viewCamera.getYaw();
    m_transform.rotation.x = viewCamera.getPitch();
    syncCameraTransform();
}