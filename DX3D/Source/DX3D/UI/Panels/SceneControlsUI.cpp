#include <DX3D/UI/Panels/SceneControlsUI.h>
#include <DX3D/UI/UIController.h>
#include <DX3D/Scene/SceneStateManager.h>
#include <DX3D/Game/UndoRedoSystem.h>
#include <DX3D/Core/Logger.h>
#include <imgui.h>

using namespace dx3d;

SceneControlsUI::SceneControlsUI(
    UIController& controller,
    SceneStateManager& sceneStateManager,
    UndoRedoSystem& undoRedoSystem)
    : m_controller(controller)
    , m_sceneStateManager(sceneStateManager)
    , m_undoRedoSystem(undoRedoSystem)
{
}

void SceneControlsUI::render()
{
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, 20));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, 100));
    ImGui::Begin("Scene Controls", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    const char* stateText = "";
    switch (m_sceneStateManager.getCurrentState())
    {
    case SceneState::Edit: stateText = "Edit Mode"; break;
    case SceneState::Play: stateText = "Play Mode"; break;
    case SceneState::Pause: stateText = "Pause Mode"; break;
    }

    ImGui::Text("Current State: %s", stateText);
    ImGui::Separator();

    bool isPlaying = m_sceneStateManager.isPlayMode();
    if (isPlaying)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.6f, 0.0f, 1.0f));
    }

    if (ImGui::Button("Play"))
    {
        m_controller.onPlayClicked();
    }

    if (isPlaying)
    {
        ImGui::PopStyleColor(3);
    }

    ImGui::SameLine();

    bool canFrameStep = m_sceneStateManager.isPauseMode();
    if (!canFrameStep)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.6f);
    }

    if (ImGui::Button("Frame Step"))
    {
        m_controller.onFrameStepClicked();
    }

    if (!canFrameStep)
    {
        ImGui::PopStyleVar();
    }

    ImGui::SameLine();

    bool canStop = !m_sceneStateManager.isEditMode();
    if (!canStop)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.6f);
    }

    if (ImGui::Button("Stop"))
    {
        m_controller.onStopClicked();
    }

    if (!canStop)
    {
        ImGui::PopStyleVar();
    }

    if (m_sceneStateManager.isEditMode())
    {
        ImGui::Separator();
        ImGui::Text("Quick Actions:");

        bool canUndo = m_undoRedoSystem.canUndo();
        bool canRedo = m_undoRedoSystem.canRedo();

        if (!canUndo)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.6f);
        }

        if (ImGui::Button("Undo"))
        {
            m_controller.onUndoClicked();
        }

        if (!canUndo)
        {
            ImGui::PopStyleVar();
        }

        ImGui::SameLine();

        if (!canRedo)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.6f);
        }

        if (ImGui::Button("Redo"))
        {
            m_controller.onRedoClicked();
        }

        if (!canRedo)
        {
            ImGui::PopStyleVar();
        }
    }

    ImGui::End();
}