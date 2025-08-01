#pragma once
#include <functional>
#include <string>

namespace dx3d
{
    class UndoRedoSystem;
    class SelectionSystem;
    class SceneStateManager;
    class UIController;

    class MainMenuBarUI
    {
    public:
        struct Callbacks
        {
            std::function<void()> onSpawnCube;
            std::function<void()> onSpawnSphere;
            std::function<void()> onSpawnCapsule;
            std::function<void()> onSpawnCylinder;
            std::function<void()> onSpawnPlane;
            std::function<void(const std::string&)> onSpawnModel;
            std::function<void()> onSpawnCubeDemo;
        };

        MainMenuBarUI(
            UIController& controller,
            UndoRedoSystem& undoRedoSystem,
            SelectionSystem& selectionSystem,
            SceneStateManager& sceneStateManager
        );

        void render(const Callbacks& callbacks);

    private:
        UIController& m_controller;
        UndoRedoSystem& m_undoRedoSystem;
        SelectionSystem& m_selectionSystem;
        SceneStateManager& m_sceneStateManager;
    };
}