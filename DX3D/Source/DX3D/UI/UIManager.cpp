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
    m_getSceneFilesCallback = deps.getSavedSceneFiles;
    m_loadSceneCallback = deps.onLoadScene;

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
    menuCallbacks.onSpawnDirectionalLight = callbacks.onSpawnDirectionalLight;
    menuCallbacks.onSpawnPointLight = callbacks.onSpawnPointLight;
    menuCallbacks.onSpawnSpotLight = callbacks.onSpawnSpotLight;
    menuCallbacks.onSaveScene = callbacks.onSaveScene;
    menuCallbacks.onLoadScene = callbacks.onLoadScene;

    // Set the callback for the "Load Scene" menu item
    menuCallbacks.onShowLoadSceneDialog = [this]() {
        if (m_getSceneFilesCallback) {
            m_sceneFiles = m_getSceneFilesCallback(); // Get the latest list of files
        }
        m_isLoadScenePopupOpen = true; // Set the flag to open the popup
        };


    m_mainMenuBar->render(menuCallbacks);
    m_viewport->renderGameView();
    m_viewport->renderSceneView();
    m_sceneControls->render();
    m_sceneOutliner->render(deltaTime);
    m_inspector->render();
    m_debugConsole->render();

    // Render our new popup if it's supposed to be open
    renderLoadScenePopup();
}

void UIManager::applyLayout()
{
}

void UIManager::renderLoadScenePopup()
{
    if (m_isLoadScenePopupOpen)
    {
        ImGui::OpenPopup("Load Scene");
        m_isLoadScenePopupOpen = false; // Reset flag so it only opens once
    }

    if (ImGui::BeginPopupModal("Load Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Select a scene file to load:");
        ImGui::Separator();

        if (m_sceneFiles.empty())
        {
            ImGui::Text("No saved scenes found in the 'Saved Scenes' folder.");
        }
        else
        {
            for (const auto& filename : m_sceneFiles)
            {
                if (ImGui::Selectable(filename.c_str()))
                {
                    if (m_loadSceneCallback)
                    {
                        m_loadSceneCallback(filename); // Call the Game::loadScene function
                    }
                    ImGui::CloseCurrentPopup();
                }
            }
        }

        ImGui::Separator();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
