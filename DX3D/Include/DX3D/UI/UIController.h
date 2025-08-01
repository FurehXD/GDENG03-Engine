#pragma once
#include <DX3D/Core/Core.h>
#include <DX3D/Math/Math.h>
#include <memory>
#include <vector>

namespace dx3d
{
    class UndoRedoSystem;
    class SelectionSystem;
    class SceneStateManager;
    class AGameObject;

    class UIController
    {
    public:
        UIController(
            UndoRedoSystem& undoRedoSystem,
            SelectionSystem& selectionSystem,
            SceneStateManager& sceneStateManager,
            std::vector<std::shared_ptr<AGameObject>>& gameObjects
        );

        void onUndoClicked();
        void onRedoClicked();
        void onDeleteClicked();
        void onPlayClicked();
        void onStopClicked();
        void onPauseClicked();
        void onFrameStepClicked();
        void onObjectSelected(std::shared_ptr<AGameObject> object);
        void onTransformChanged(std::shared_ptr<AGameObject> object,
            const Vector3& oldPos, const Vector3& newPos,
            const Vector3& oldRot, const Vector3& newRot,
            const Vector3& oldScale, const Vector3& newScale);

    private:
        UndoRedoSystem& m_undoRedoSystem;
        SelectionSystem& m_selectionSystem;
        SceneStateManager& m_sceneStateManager;
        std::vector<std::shared_ptr<AGameObject>>& m_gameObjects;
    };
}