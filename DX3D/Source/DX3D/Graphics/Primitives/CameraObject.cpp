#include <DX3D/Graphics/Primitives/CameraObject.h>

using namespace dx3d;

CameraObject::CameraObject()
    : AGameObject()
{
    m_camera = std::make_unique<SceneCamera>();
    syncCameraTransform();
}

CameraObject::CameraObject(const Vector3& position, const Vector3& rotation)
    : AGameObject(position, rotation)
{
    m_camera = std::make_unique<SceneCamera>();
    syncCameraTransform();
}

void CameraObject::update(float deltaTime)
{
    AGameObject::update(deltaTime);
    syncCameraTransform();
}

void CameraObject::syncCameraTransform()
{
    m_camera->setPosition(getPosition());

    Vector3 rotation = getRotation();
    float yaw = rotation.y;
    float pitch = rotation.x;

    Vector3 forward;
    forward.x = std::sin(yaw) * std::cos(pitch);
    forward.y = std::sin(pitch);
    forward.z = std::cos(yaw) * std::cos(pitch);

    Vector3 target = getPosition() + forward;
    m_camera->lookAt(target);
}

Matrix4x4 CameraObject::getProjectionMatrix(float aspectRatio) const
{
    return Matrix4x4::CreatePerspectiveFovLH(m_fov, aspectRatio, m_nearPlane, m_farPlane);
}

void CameraObject::alignWithView(const SceneCamera& viewCamera)
{
    setPosition(viewCamera.getPosition());

    Vector3 newRotation = getRotation();
    newRotation.y = viewCamera.getYaw();
    newRotation.x = viewCamera.getPitch();
    setRotation(newRotation);
}