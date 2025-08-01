#pragma once
#include <DX3D/Core/Core.h>
#include <DX3D/Math/Math.h>
#include <memory>

namespace dx3d
{
    class AGameObject;
    class SelectionSystem;
    class UIController;
    class SceneStateManager;

    struct TransformTracking
    {
        bool isDragging = false;
        Vector3 originalPosition{ 0, 0, 0 };
        Vector3 originalRotation{ 0, 0, 0 };
        Vector3 originalScale{ 1, 1, 1 };
        std::weak_ptr<AGameObject> trackedObject;
    };

    class InspectorUI
    {
    public:
        InspectorUI(
            UIController& controller,
            SelectionSystem& selectionSystem,
            SceneStateManager& sceneStateManager
        );

        void render();

    private:
        void renderTransform(std::shared_ptr<AGameObject> object);
        void renderCamera(std::shared_ptr<AGameObject> object);
        void renderPhysics(std::shared_ptr<AGameObject> object);
        void renderObjectInfo(std::shared_ptr<AGameObject> object);

    private:
        UIController& m_controller;
        SelectionSystem& m_selectionSystem;
        SceneStateManager& m_sceneStateManager;
        TransformTracking m_transformTracking;
    };
}