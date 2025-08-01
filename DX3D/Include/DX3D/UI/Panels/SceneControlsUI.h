#pragma once

namespace dx3d
{
    class UIController;
    class SceneStateManager;
    class UndoRedoSystem;

    class SceneControlsUI
    {
    public:
        SceneControlsUI(
            UIController& controller,
            SceneStateManager& sceneStateManager,
            UndoRedoSystem& undoRedoSystem
        );

        void render();

    private:
        UIController& m_controller;
        SceneStateManager& m_sceneStateManager;
        UndoRedoSystem& m_undoRedoSystem;
    };
}