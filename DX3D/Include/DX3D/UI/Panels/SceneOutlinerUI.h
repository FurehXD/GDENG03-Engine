#pragma once
#include <memory>
#include <vector>
#include <string>
namespace dx3d
{
    class AGameObject;
    class SelectionSystem;
    class UIController;
    class SceneStateManager;
    class UndoRedoSystem;

    class SceneOutlinerUI
    {
    public:
        SceneOutlinerUI(
            UIController& controller,
            SelectionSystem& selectionSystem,
            SceneStateManager& sceneStateManager,
            UndoRedoSystem& undoRedoSystem,
            const std::vector<std::shared_ptr<AGameObject>>& gameObjects
        );

        void render(float deltaTime);

    private:
        std::string getObjectDisplayName(std::shared_ptr<AGameObject> object, int index);
        std::string getObjectIcon(std::shared_ptr<AGameObject> object);

    private:
        UIController& m_controller;
        SelectionSystem& m_selectionSystem;
        SceneStateManager& m_sceneStateManager;
        UndoRedoSystem& m_undoRedoSystem;
        const std::vector<std::shared_ptr<AGameObject>>& m_gameObjects;
    };
}