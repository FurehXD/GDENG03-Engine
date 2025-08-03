#pragma once
#include <DX3D/UI/UIState.h>
#include <memory>
#include <functional>

namespace dx3d
{
    class Logger;
    class UndoRedoSystem;
    class SelectionSystem;
    class SceneStateManager;
    class ViewportManager;
    class AGameObject;
    class UIController;
    class MainMenuBarUI;
    class SceneControlsUI;
    class SceneOutlinerUI;
    class InspectorUI;
    class DebugConsoleUI;
    class ViewportUI;

    class UIManager
    {
    public:
        struct Dependencies
        {
            Logger& logger;
            UndoRedoSystem& undoRedoSystem;
            SelectionSystem& selectionSystem;
            SceneStateManager& sceneStateManager;
            ViewportManager& viewportManager;
            std::vector<std::shared_ptr<AGameObject>>& gameObjects;

            std::function<std::vector<std::string>()> getSavedSceneFiles;
            std::function<void(const std::string&)> onLoadScene;
        };

        struct SpawnCallbacks
        {
            std::function<void()> onSpawnCube;
            std::function<void()> onSpawnSphere;
            std::function<void()> onSpawnCapsule;
            std::function<void()> onSpawnCylinder;
            std::function<void()> onSpawnPlane;
            std::function<void(const std::string&)> onSpawnModel;
            std::function<void()> onSpawnCubeDemo;
            std::function<void()> onSpawnDirectionalLight;
            std::function<void()> onSpawnPointLight;
            std::function<void()> onSpawnSpotLight;
            std::function<void()> onSaveScene;
            std::function<void(const std::string&)> onLoadScene;
        };

        explicit UIManager(const Dependencies& deps);
        ~UIManager();

        void render(float deltaTime, const SpawnCallbacks& callbacks);
        UIController& getController() { return *m_controller; }
        const UIState& getState() const { return m_state; }

    private:
        void applyLayout();
        void renderLoadScenePopup();

    private:
        UIState m_state;
        std::unique_ptr<UIController> m_controller;
        std::unique_ptr<MainMenuBarUI> m_mainMenuBar;
        std::unique_ptr<SceneControlsUI> m_sceneControls;
        std::unique_ptr<SceneOutlinerUI> m_sceneOutliner;
        std::unique_ptr<InspectorUI> m_inspector;
        std::unique_ptr<DebugConsoleUI> m_debugConsole;
        std::unique_ptr<ViewportUI> m_viewport;

        // Manage Pop Up States
        bool m_isLoadScenePopupOpen = false;
        std::vector<std::string> m_sceneFiles;
        std::function<std::vector<std::string>()> m_getSceneFilesCallback;
        std::function<void(const std::string&)> m_loadSceneCallback;
    };
}