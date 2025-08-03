#pragma once
#include <DX3D/Graphics/Primitives/AGameObject.h>
#include <DX3D/Game/SceneCamera.h>
#include <memory>

namespace dx3d
{
    class CameraObject : public AGameObject
    {
    public:
        CameraObject();
        CameraObject(const Vector3& position, const Vector3& rotation = Vector3(0, 0, 0));
        virtual ~CameraObject() = default;

        virtual void update(float deltaTime) override;

        SceneCamera& getCamera() { return *m_camera; }
        const SceneCamera& getCamera() const { return *m_camera; }

        void setFOV(float fov) { m_fov = fov; }
        void setNearPlane(float nearPlane) { m_nearPlane = nearPlane; }
        void setFarPlane(float farPlane) { m_farPlane = farPlane; }

        float getFOV() const { return m_fov; }
        float getNearPlane() const { return m_nearPlane; }
        float getFarPlane() const { return m_farPlane; }

        Matrix4x4 getProjectionMatrix(float aspectRatio) const;
        void alignWithView(const SceneCamera& viewCamera);

    protected:
        virtual CollisionShapeType getCollisionShapeType() const override
        {
            return CollisionShapeType::Box;
        }

    private:
        void syncCameraTransform();

    private:
        std::unique_ptr<SceneCamera> m_camera;
        float m_fov = 1.0472f;
        float m_nearPlane = 0.1f;
        float m_farPlane = 100.0f;
    };
}