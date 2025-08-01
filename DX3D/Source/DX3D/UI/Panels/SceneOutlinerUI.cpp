#include <DX3D/UI/Panels/SceneOutlinerUI.h>
#include <DX3D/UI/UIController.h>
#include <DX3D/Game/SelectionSystem.h>
#include <DX3D/Scene/SceneStateManager.h>
#include <DX3D/Game/UndoRedoSystem.h>
#include <DX3D/Graphics/Primitives/AGameObject.h>
#include <DX3D/Graphics/Primitives/Cube.h>
#include <DX3D/Graphics/Primitives/Plane.h>
#include <DX3D/Graphics/Primitives/Sphere.h>
#include <DX3D/Graphics/Primitives/Cylinder.h>
#include <DX3D/Graphics/Primitives/Capsule.h>
#include <DX3D/Graphics/Primitives/CameraObject.h>
#include <imgui.h>

using namespace dx3d;

SceneOutlinerUI::SceneOutlinerUI(
    UIController& controller,
    SelectionSystem& selectionSystem,
    SceneStateManager& sceneStateManager,
    UndoRedoSystem& undoRedoSystem,
    const std::vector<std::shared_ptr<AGameObject>>& gameObjects)
    : m_controller(controller)
    , m_selectionSystem(selectionSystem)
    , m_sceneStateManager(sceneStateManager)
    , m_undoRedoSystem(undoRedoSystem)
    , m_gameObjects(gameObjects)
{
}

void SceneOutlinerUI::render(float deltaTime)
{
    ImGuiIO& io = ImGui::GetIO();
    float windowWidth = io.DisplaySize.x;
    float windowHeight = io.DisplaySize.y;
    float halfWidth = windowWidth * 0.5f;
    float halfHeight = windowHeight * 0.5f;

    ImGui::SetNextWindowPos(ImVec2(halfWidth, 120));
    ImGui::SetNextWindowSize(ImVec2(halfWidth, halfHeight - 120));
    ImGui::Begin("Scene Outliner", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    ImGui::Text("Physics Demo");
    ImGui::Separator();

    ImGui::Text("Objects: %zu", m_gameObjects.size());
    ImGui::Text("Delta Time: %.3f ms", deltaTime * 1000.0f);
    ImGui::Text("FPS: %.1f", 1.0f / deltaTime);

    if (m_sceneStateManager.isEditMode())
    {
        ImGui::Text("Undo Stack: %d actions", m_undoRedoSystem.getUndoCount());
        ImGui::Text("Redo Stack: %d actions", m_undoRedoSystem.getRedoCount());
    }

    ImGui::Separator();
    ImGui::Text("Scene Hierarchy");
    ImGui::BeginChild("Outliner", ImVec2(0, 0), true);

    int objectId = 0;
    for (const auto& gameObject : m_gameObjects)
    {
        std::string label = getObjectDisplayName(gameObject, objectId++);

        bool isSelected = (gameObject == m_selectionSystem.getSelectedObject());
        if (ImGui::Selectable(label.c_str(), isSelected))
        {
            m_controller.onObjectSelected(gameObject);
        }
    }
    ImGui::EndChild();

    ImGui::End();
}

std::string SceneOutlinerUI::getObjectDisplayName(std::shared_ptr<AGameObject> object, int index)
{
    std::string objectName = "Object";
    if (std::dynamic_pointer_cast<Cube>(object)) objectName = "Cube";
    else if (std::dynamic_pointer_cast<Plane>(object)) objectName = "Plane";
    else if (std::dynamic_pointer_cast<Sphere>(object)) objectName = "Sphere";
    else if (std::dynamic_pointer_cast<Cylinder>(object)) objectName = "Cylinder";
    else if (std::dynamic_pointer_cast<Capsule>(object)) objectName = "Capsule";
    else if (std::dynamic_pointer_cast<CameraObject>(object)) objectName = "Game Camera";

    return objectName + " " + std::to_string(index);
}

std::string SceneOutlinerUI::getObjectIcon(std::shared_ptr<AGameObject> object)
{
    if (std::dynamic_pointer_cast<Cube>(object)) return "[C]";
    else if (std::dynamic_pointer_cast<Plane>(object)) return "[P]";
    else if (std::dynamic_pointer_cast<Sphere>(object)) return "[S]";
    else if (std::dynamic_pointer_cast<Cylinder>(object)) return "[Y]";
    else if (std::dynamic_pointer_cast<Capsule>(object)) return "[A]";
    else if (std::dynamic_pointer_cast<CameraObject>(object)) return "[CAM]";
    return "[?]";
}