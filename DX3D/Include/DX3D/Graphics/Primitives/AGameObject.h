#pragma once
#include <DX3D/Graphics/VertexBuffer.h>
#include <DX3D/Graphics/IndexBuffer.h>
#include <DX3D/Math/Math.h>
#include <DX3D/ECS/Entity.h>
#include <DX3D/ECS/Components/TransformComponent.h>
#include <DX3D/ECS/Components/PhysicsComponent.h>
#include <memory>
#include <typeinfo>

namespace dx3d
{
    class AGameObject
    {
    public:
        struct Transform
        {
            Vector3 position{ 0.0f, 0.0f, 0.0f };
            Vector3 rotation{ 0.0f, 0.0f, 0.0f }; // Euler angles in radians
            Vector3 scale{ 1.0f, 1.0f, 1.0f };

            Matrix4x4 getWorldMatrix() const;
        };

    public:
        // Constructors
        AGameObject();
        AGameObject(const Vector3& position, const Vector3& rotation = Vector3(0, 0, 0), const Vector3& scale = Vector3(1, 1, 1));
        virtual ~AGameObject();

        // Transform operations (now route through ECS)
        void setPosition(const Vector3& position);
        void setRotation(const Vector3& rotation);
        void setScale(const Vector3& scale);

        const Vector3& getPosition() const;
        const Vector3& getRotation() const;
        const Vector3& getScale() const;

        Matrix4x4 getWorldMatrix() const;

        // Animation helpers
        void rotate(const Vector3& deltaRotation);
        void translate(const Vector3& deltaPosition);

        // Enable/Disable functionality
        void setEnabled(bool enabled);
        bool isEnabled() const { return m_enabled; }

        // Physics management
        void enablePhysics(PhysicsBodyType bodyType = PhysicsBodyType::Dynamic);
        void disablePhysics();
        bool hasPhysics() const;

        void setPhysicsMass(float mass);
        void setPhysicsRestitution(float restitution);
        void setPhysicsFriction(float friction);

        void applyForce(const Vector3& force);
        void applyImpulse(const Vector3& impulse);

        Vector3 getLinearVelocity() const;
        void setLinearVelocity(const Vector3& velocity);

        // ECS integration
        Entity getEntity() const { return m_entity; }

        // Virtual methods that can be overridden by derived classes
        virtual void update(float deltaTime) {}
        virtual void render() {}

        std::string getObjectType();

    protected:
        // For derived classes to override collision shape creation
        virtual CollisionShapeType getCollisionShapeType() const = 0;
        virtual PhysicsComponent createPhysicsComponent() const;

        void syncTransformFromECS();
        void syncTransformToECS();

    protected:
        Transform m_transform; // Kept for backward compatibility
        Entity m_entity;
        bool m_enabled = true;
        bool m_hadPhysicsBeforeDisable = false;
        PhysicsBodyType m_previousBodyType = PhysicsBodyType::Dynamic;

    private:
        static EntityID s_nextEntityID;
    };
}