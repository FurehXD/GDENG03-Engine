#pragma once
#include <DX3D/Scene/Scene.h>
#include <DX3D/Math/Math.h>
#include <functional>
#include <vector>
#include <memory>
#include <unordered_map>

namespace dx3d
{
    class AGameObject;
    class SceneCamera;
    class PhysicsSystem;

    struct ObjectSnapshot
    {
        Vector3 position;
        Vector3 rotation;
        Vector3 scale;
        Vector3 linearVelocity;
        Vector3 angularVelocity;
        bool hadPhysics;
    };

    class SceneStateManager
    {
    public:
        using StateChangeCallback = std::function<void(SceneState, SceneState)>;

        SceneStateManager();
        ~SceneStateManager();

        void transitionToEdit();
        void transitionToPlay();
        void transitionToPause();
        void frameStep();

        SceneState getCurrentState() const { return m_stateInfo.currentState; }
        SceneState getPreviousState() const { return m_stateInfo.previousState; }
        bool canFrameStep() const { return m_stateInfo.canFrameStep; }
        bool isFrameStepRequested() const { return m_stateInfo.frameStepRequested; }

        void saveObjectStates(const std::vector<std::shared_ptr<AGameObject>>& objects);
        void restoreObjectStates(const std::vector<std::shared_ptr<AGameObject>>& objects);

        void addStateChangeCallback(const StateChangeCallback& callback);

        void update(float deltaTime);

        bool isEditMode() const { return m_stateInfo.currentState == SceneState::Edit; }
        bool isPlayMode() const { return m_stateInfo.currentState == SceneState::Play; }
        bool isPauseMode() const { return m_stateInfo.currentState == SceneState::Pause; }
        void clearFrameStepRequest() { m_stateInfo.frameStepRequested = false; }

    private:
        void setState(SceneState newState);
        void notifyStateChange(SceneState oldState, SceneState newState);

    private:
        SceneStateInfo m_stateInfo;
        std::vector<StateChangeCallback> m_callbacks;
        std::unordered_map<uint32_t, ObjectSnapshot> m_objectSnapshots;
    };
}