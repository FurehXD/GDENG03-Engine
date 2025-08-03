#pragma once
#include <DX3D/Core/Core.h>
#include <DX3D/Math/Math.h>
#include <memory>
#include <vector>

namespace dx3d
{
    class AGameObject;
    class SceneCamera;

    class SelectionSystem
    {
    public:
        SelectionSystem();
        ~SelectionSystem();

        void setSelectedObject(std::shared_ptr<AGameObject> object);
        std::shared_ptr<AGameObject> getSelectedObject() const { return m_selectedObject; }

        std::shared_ptr<AGameObject> pickObject(
            const std::vector<std::shared_ptr<AGameObject>>& objects,
            const SceneCamera& camera,
            float mouseX, float mouseY,
            ui32 viewportWidth, ui32 viewportHeight);

    private:
        bool rayIntersectsAABB(const Vector3& rayOrigin, const Vector3& rayDir,
            const Vector3& aabbMin, const Vector3& aabbMax, float& t);

    private:
        std::shared_ptr<AGameObject> m_selectedObject;
    };
}