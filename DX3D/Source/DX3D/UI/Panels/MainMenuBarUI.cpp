#include <DX3D/UI/Panels/MainMenuBarUI.h>
#include <DX3D/UI/UIController.h>
#include <DX3D/Game/UndoRedoSystem.h>
#include <DX3D/Game/SelectionSystem.h>
#include <DX3D/Scene/SceneStateManager.h>
#include <DX3D/Core/Logger.h>
#include <imgui.h>

using namespace dx3d;

MainMenuBarUI::MainMenuBarUI(
    UIController& controller,
    UndoRedoSystem& undoRedoSystem,
    SelectionSystem& selectionSystem,
    SceneStateManager& sceneStateManager)
    : m_controller(controller)
    , m_undoRedoSystem(undoRedoSystem)
    , m_selectionSystem(selectionSystem)
    , m_sceneStateManager(sceneStateManager)
{
}

void MainMenuBarUI::render(const Callbacks& callbacks)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Edit"))
        {
            bool canUndo = m_undoRedoSystem.canUndo();
            bool canRedo = m_undoRedoSystem.canRedo();
            bool isEditMode = m_sceneStateManager.isEditMode();

            if (!canUndo || !isEditMode)
            {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.6f);
            }

            if (ImGui::MenuItem("Undo", "Ctrl+Z", false, canUndo && isEditMode))
            {
                m_controller.onUndoClicked();
            }

            if (!canUndo || !isEditMode)
            {
                ImGui::PopStyleVar();
            }

            if (canUndo && isEditMode)
            {
                ImGui::SameLine();
                ImGui::TextDisabled("(%s)", m_undoRedoSystem.getUndoDescription().c_str());
            }

            if (!canRedo || !isEditMode)
            {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.6f);
            }

            if (ImGui::MenuItem("Redo", "Ctrl+Shift+Z", false, canRedo && isEditMode))
            {
                m_controller.onRedoClicked();
            }

            if (!canRedo || !isEditMode)
            {
                ImGui::PopStyleVar();
            }

            if (canRedo && isEditMode)
            {
                ImGui::SameLine();
                ImGui::TextDisabled("(%s)", m_undoRedoSystem.getRedoDescription().c_str());
            }

            ImGui::Separator();

            auto selectedObject = m_selectionSystem.getSelectedObject();
            bool hasSelection = selectedObject != nullptr;

            if (!hasSelection || !isEditMode)
            {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.6f);
            }

            if (ImGui::MenuItem("Delete", "Delete", false, hasSelection && isEditMode))
            {
                m_controller.onDeleteClicked();
            }

            if (!hasSelection || !isEditMode)
            {
                ImGui::PopStyleVar();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("GameObjects"))
        {
            bool isEditMode = m_sceneStateManager.isEditMode();

            if (ImGui::BeginMenu("Primitives"))
            {
                if (ImGui::MenuItem("Cube", nullptr, false, isEditMode))
                {
                    if (callbacks.onSpawnCube)
                        callbacks.onSpawnCube();
                }

                if (ImGui::MenuItem("Sphere", nullptr, false, isEditMode))
                {
                    if (callbacks.onSpawnSphere)
                        callbacks.onSpawnSphere();
                }

                if (ImGui::MenuItem("Capsule", nullptr, false, isEditMode))
                {
                    if (callbacks.onSpawnCapsule)
                        callbacks.onSpawnCapsule();
                }

                if (ImGui::MenuItem("Cylinder", nullptr, false, isEditMode))
                {
                    if (callbacks.onSpawnCylinder)
                        callbacks.onSpawnCylinder();
                }

                if (ImGui::MenuItem("Plane", nullptr, false, isEditMode))
                {
                    if (callbacks.onSpawnPlane)
                        callbacks.onSpawnPlane();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Models"))
            {
                if (ImGui::MenuItem("Bunny", nullptr, false, isEditMode))
                {
                    if (callbacks.onSpawnModel)
                        callbacks.onSpawnModel("bunny.obj");
                }

                if (ImGui::MenuItem("Armadillo", nullptr, false, isEditMode))
                {
                    if (callbacks.onSpawnModel)
                        callbacks.onSpawnModel("armadillo.obj");
                }

                if (ImGui::MenuItem("Teapot", nullptr, false, isEditMode))
                {
                    if (callbacks.onSpawnModel)
                        callbacks.onSpawnModel("teapot.obj");
                }

                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Cube Demo"))
            {
                if (callbacks.onSpawnCubeDemo)
                    callbacks.onSpawnCubeDemo();
            }

            ImGui::EndMenu();
        }

        if (m_sceneStateManager.isEditMode())
        {
            ImGui::SameLine(ImGui::GetWindowWidth() - 200);
            ImGui::Text("Undo: %d | Redo: %d",
                m_undoRedoSystem.getUndoCount(),
                m_undoRedoSystem.getRedoCount());
        }

        ImGui::EndMainMenuBar();
    }
}