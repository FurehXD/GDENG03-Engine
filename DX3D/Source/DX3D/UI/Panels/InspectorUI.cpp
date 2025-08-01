#include <DX3D/UI/Panels/InspectorUI.h>
#include <DX3D/UI/UIController.h>
#include <DX3D/Game/SelectionSystem.h>
#include <DX3D/Scene/SceneStateManager.h>
#include <DX3D/Graphics/Primitives/AGameObject.h>
#include <DX3D/Graphics/Primitives/CameraObject.h>
#include <DX3D/Graphics/Primitives/Cube.h>
#include <DX3D/Graphics/Primitives/Plane.h>
#include <DX3D/Graphics/Primitives/Sphere.h>
#include <DX3D/Graphics/Primitives/Cylinder.h>
#include <DX3D/Graphics/Primitives/Capsule.h>
#include <DX3D/ECS/ComponentManager.h>
#include <DX3D/ECS/Components/PhysicsComponent.h>
#include <DX3D/Core/Logger.h>
#include <imgui.h>
#include <cmath>

using namespace dx3d;

InspectorUI::InspectorUI(
    UIController& controller,
    SelectionSystem& selectionSystem,
    SceneStateManager& sceneStateManager)
    : m_controller(controller)
    , m_selectionSystem(selectionSystem)
    , m_sceneStateManager(sceneStateManager)
{
}

void InspectorUI::render()
{
    ImGuiIO& io = ImGui::GetIO();
    float windowWidth = io.DisplaySize.x;
    float windowHeight = io.DisplaySize.y;
    float halfWidth = windowWidth * 0.5f;
    float halfHeight = windowHeight * 0.5f;
    float inspectorHeight = halfHeight * 0.6f;

    ImGui::SetNextWindowPos(ImVec2(halfWidth, halfHeight));
    ImGui::SetNextWindowSize(ImVec2(halfWidth, inspectorHeight));
    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    auto selectedObject = m_selectionSystem.getSelectedObject();
    if (selectedObject)
    {
        ImGui::Text("Selected Object");
        ImGui::Separator();

        renderTransform(selectedObject);
        renderCamera(selectedObject);
        renderPhysics(selectedObject);
        renderObjectInfo(selectedObject);

        if (m_sceneStateManager.isEditMode())
        {
            ImGui::Separator();
            if (ImGui::Button("Delete Object", ImVec2(-1, 0)))
            {
                m_controller.onDeleteClicked();
            }
        }
    }
    else
    {
        ImGui::Text("No object selected");
        ImGui::Text("Click on an object in the Scene View or Outliner to select it");
    }

    ImGui::End();
}

void InspectorUI::renderTransform(std::shared_ptr<AGameObject> object)
{
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        Vector3 pos = object->getPosition();
        Vector3 rot = object->getRotation();
        Vector3 scale = object->getScale();

        Vector3 rotDegrees = Vector3(
            rot.x * 180.0f / 3.14159f,
            rot.y * 180.0f / 3.14159f,
            rot.z * 180.0f / 3.14159f
        );

        bool transformChanged = false;

        if (m_transformTracking.trackedObject.lock() != object)
        {
            m_transformTracking.isDragging = false;
            m_transformTracking.trackedObject = object;
        }

        if (ImGui::DragFloat3("Position", &pos.x, 0.1f))
        {
            if (!m_transformTracking.isDragging)
            {
                m_transformTracking.originalPosition = object->getPosition();
                m_transformTracking.originalRotation = object->getRotation();
                m_transformTracking.originalScale = object->getScale();
                m_transformTracking.isDragging = true;
            }

            object->setPosition(pos);
            transformChanged = true;
        }

        bool positionActive = ImGui::IsItemActive();

        if (ImGui::DragFloat3("Rotation", &rotDegrees.x, 1.0f))
        {
            if (!m_transformTracking.isDragging)
            {
                m_transformTracking.originalPosition = object->getPosition();
                m_transformTracking.originalRotation = object->getRotation();
                m_transformTracking.originalScale = object->getScale();
                m_transformTracking.isDragging = true;
            }

            Vector3 rotRadians = Vector3(
                rotDegrees.x * 3.14159f / 180.0f,
                rotDegrees.y * 3.14159f / 180.0f,
                rotDegrees.z * 3.14159f / 180.0f
            );
            object->setRotation(rotRadians);
            transformChanged = true;
        }

        bool rotationActive = ImGui::IsItemActive();

        if (ImGui::DragFloat3("Scale", &scale.x, 0.01f, 0.01f, 10.0f))
        {
            if (!m_transformTracking.isDragging)
            {
                m_transformTracking.originalPosition = object->getPosition();
                m_transformTracking.originalRotation = object->getRotation();
                m_transformTracking.originalScale = object->getScale();
                m_transformTracking.isDragging = true;
            }

            object->setScale(scale);
            transformChanged = true;
        }

        bool scaleActive = ImGui::IsItemActive();
        bool anyControlActive = positionActive || rotationActive || scaleActive;

        if (m_transformTracking.isDragging && !anyControlActive)
        {
            m_controller.onTransformChanged(
                object,
                m_transformTracking.originalPosition, object->getPosition(),
                m_transformTracking.originalRotation, object->getRotation(),
                m_transformTracking.originalScale, object->getScale()
            );

            m_transformTracking.isDragging = false;
        }

        if (transformChanged)
        {
            if (object->hasPhysics() && m_sceneStateManager.isEditMode())
            {
                PhysicsBodyType bodyType = PhysicsBodyType::Dynamic;
                auto& componentManager = ComponentManager::getInstance();
                auto* physicsComp = componentManager.getComponent<PhysicsComponent>(object->getEntity().getID());
                if (physicsComp)
                {
                    bodyType = physicsComp->bodyType;
                }

                object->disablePhysics();
                object->enablePhysics(bodyType);
            }
        }
    }
}

void InspectorUI::renderCamera(std::shared_ptr<AGameObject> object)
{
    auto cameraObject = std::dynamic_pointer_cast<CameraObject>(object);
    if (!cameraObject)
        return;

    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
    {
        float fov = cameraObject->getFOV() * 180.0f / 3.14159f;
        float nearPlane = cameraObject->getNearPlane();
        float farPlane = cameraObject->getFarPlane();

        if (ImGui::SliderFloat("Field of View", &fov, 10.0f, 120.0f, "%.1f°"))
        {
            cameraObject->setFOV(fov * 3.14159f / 180.0f);
        }

        if (ImGui::DragFloat("Near Plane", &nearPlane, 0.01f, 0.01f, 10.0f, "%.3f"))
        {
            cameraObject->setNearPlane(nearPlane);
        }

        if (ImGui::DragFloat("Far Plane", &farPlane, 1.0f, 1.0f, 1000.0f, "%.1f"))
        {
            cameraObject->setFarPlane(farPlane);
        }

        ImGui::Separator();

        if (ImGui::Button("Align with View", ImVec2(-1, 0)))
        {
            //DX3DLogInfo("Align with View button would be implemented in Game class");
        }

        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Align the game camera with the current scene view camera");
        }
    }
}

void InspectorUI::renderPhysics(std::shared_ptr<AGameObject> object)
{
    if (!object->hasPhysics())
        return;

    if (ImGui::CollapsingHeader("Physics"))
    {
        Vector3 vel = object->getLinearVelocity();
        ImGui::Text("Linear Velocity: (%.2f, %.2f, %.2f)", vel.x, vel.y, vel.z);
        ImGui::Text("Speed: %.2f", std::sqrt(vel.x * vel.x + vel.y * vel.y + vel.z * vel.z));

        if (m_sceneStateManager.isEditMode())
        {
            if (ImGui::Button("Apply Upward Impulse"))
            {
                object->applyImpulse(Vector3(0.0f, 10.0f, 0.0f));
            }
        }
    }
}

void InspectorUI::renderObjectInfo(std::shared_ptr<AGameObject> object)
{
    if (ImGui::CollapsingHeader("Object Info"))
    {
        ImGui::Text("Entity ID: %u", object->getEntity().getID());

        std::string objectType = "Unknown";
        if (std::dynamic_pointer_cast<Cube>(object)) objectType = "Cube";
        else if (std::dynamic_pointer_cast<Plane>(object)) objectType = "Plane";
        else if (std::dynamic_pointer_cast<Sphere>(object)) objectType = "Sphere";
        else if (std::dynamic_pointer_cast<Cylinder>(object)) objectType = "Cylinder";
        else if (std::dynamic_pointer_cast<Capsule>(object)) objectType = "Capsule";
        else if (std::dynamic_pointer_cast<CameraObject>(object)) objectType = "Camera";

        ImGui::Text("Type: %s", objectType.c_str());
        ImGui::Text("Has Physics: %s", object->hasPhysics() ? "Yes" : "No");
    }
}