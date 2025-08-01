#include <DX3D/UI/UIManager.h>
#include <DX3D/UI/UIController.h>
#include <DX3D/UI/Panels/MainMenuBarUI.h>
#include <DX3D/UI/Panels/SceneControlsUI.h>
#include <DX3D/UI/Panels/SceneOutlinerUI.h>
#include <DX3D/UI/Panels/InspectorUI.h>
#include <DX3D/UI/Panels/DebugConsoleUI.h>
#include <DX3D/UI/Panels/ViewportUI.h>
#include <imgui.h>

using namespace dx3d;

UIManager::UIManager(const Dependencies& deps)
{
    m_controller = std::make_unique<UIController>(
        deps.undoRedoSystem,
        deps.selectionSystem,
        deps.sceneStateManager,
        deps.gameObjects
    );

    m_mainMenuBar = std::make_unique<MainMenuBarUI>(
        *m_controller,
        deps.undoRedoSystem,
        deps.selectionSystem,
        deps.sceneStateManager
    );

    m_sceneControls = std::make_unique<SceneControlsUI>(
        *m_controller,
        deps.sceneStateManager,
        deps.undoRedoSystem
    );

    m_sceneOutliner = std::make_unique<SceneOutlinerUI>(
        *m_controller,
        deps.selectionSystem,
        deps.sceneStateManager,
        deps.undoRedoSystem,
        deps.gameObjects
    );

    m_inspector = std::make_unique<InspectorUI>(
        *m_controller,
        deps.selectionSystem,
        deps.sceneStateManager
    );

    m_debugConsole = std::make_unique<DebugConsoleUI>(deps.logger);
    m_viewport = std::make_unique<ViewportUI>(deps.viewportManager);
}

UIManager::~UIManager() = default;

void UIManager::render(float deltaTime, const SpawnCallbacks& callbacks)
{
    MainMenuBarUI::Callbacks menuCallbacks;
    menuCallbacks.onSpawnCube = callbacks.onSpawnCube;
    menuCallbacks.onSpawnSphere = callbacks.onSpawnSphere;
    menuCallbacks.onSpawnCapsule = callbacks.onSpawnCapsule;
    menuCallbacks.onSpawnCylinder = callbacks.onSpawnCylinder;
    menuCallbacks.onSpawnPlane = callbacks.onSpawnPlane;
    menuCallbacks.onSpawnModel = callbacks.onSpawnModel;
    menuCallbacks.onSpawnCubeDemo = callbacks.onSpawnCubeDemo;

    m_mainMenuBar->render(menuCallbacks);
    m_viewport->renderGameView();
    m_viewport->renderSceneView();
    m_sceneControls->render();
    m_sceneOutliner->render(deltaTime);
    m_inspector->render();
    m_debugConsole->render();
}

void UIManager::applyLayout()
{
}