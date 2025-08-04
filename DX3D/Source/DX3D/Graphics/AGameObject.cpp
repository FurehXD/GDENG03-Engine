#include <DX3D/Graphics/Primitives/AGameObject.h>
#include <DX3D/ECS/ComponentManager.h>
#include <DX3D/Physics/PhysicsSystem.h>
#include <DirectXMath.h>

using namespace dx3d;
using namespace DirectX;

EntityID AGameObject::s_nextEntityID = 1;

AGameObject::AGameObject()
{
    // Create entity
    m_entity = Entity(s_nextEntityID++);

    // Add transform component
    auto& componentManager = ComponentManager::getInstance();
    TransformComponent transform;
    transform.position = m_transform.position;
    transform.rotation = m_transform.rotation;
    transform.scale = m_transform.scale;
    componentManager.addComponent(m_entity.getID(), transform);

}

AGameObject::AGameObject(const Vector3& position, const Vector3& rotation, const Vector3& scale)
{
    // Create entity
    m_entity = Entity(s_nextEntityID++);

    // Set transform values
    m_transform.position = position;
    m_transform.rotation = rotation;
    m_transform.scale = scale;

    // Add transform component
    auto& componentManager = ComponentManager::getInstance();
    TransformComponent transform;
    transform.position = position;
    transform.rotation = rotation;
    transform.scale = scale;
    componentManager.addComponent(m_entity.getID(), transform);
}

AGameObject::~AGameObject()
{
    // Clean up physics if enabled
    if (hasPhysics())
    {
        disablePhysics();
    }

    // Remove all components
    auto& componentManager = ComponentManager::getInstance();
    componentManager.removeEntity(m_entity.getID());
}

void AGameObject::setPosition(const Vector3& position)
{
    m_transform.position = position;
    syncTransformToECS();
}

void AGameObject::setRotation(const Vector3& rotation)
{
    m_transform.rotation = rotation;
    syncTransformToECS();
}

void AGameObject::setScale(const Vector3& scale)
{
    m_transform.scale = scale;
    syncTransformToECS();
}

const Vector3& AGameObject::getPosition() const
{
    // Always sync from ECS to ensure we have the latest physics-updated position
    const_cast<AGameObject*>(this)->syncTransformFromECS();
    return m_transform.position;
}

const Vector3& AGameObject::getRotation() const
{
    // Always sync from ECS to ensure we have the latest physics-updated rotation
    const_cast<AGameObject*>(this)->syncTransformFromECS();
    return m_transform.rotation;
}

const Vector3& AGameObject::getScale() const
{
    // Scale is not affected by physics, but sync anyway for consistency
    const_cast<AGameObject*>(this)->syncTransformFromECS();
    return m_transform.scale;
}

Matrix4x4 AGameObject::getWorldMatrix() const
{
    // Always sync from ECS first
    const_cast<AGameObject*>(this)->syncTransformFromECS();
    return m_transform.getWorldMatrix();
}

void AGameObject::rotate(const Vector3& deltaRotation)
{
    syncTransformFromECS();
    m_transform.rotation += deltaRotation;
    syncTransformToECS();
}

void AGameObject::translate(const Vector3& deltaPosition)
{
    syncTransformFromECS();
    m_transform.position += deltaPosition;
    syncTransformToECS();
}

void AGameObject::enablePhysics(PhysicsBodyType bodyType)
{
    if (hasPhysics())
    {
        disablePhysics();
    }

    // Create physics component
    PhysicsComponent physicsComp = createPhysicsComponent();
    physicsComp.bodyType = bodyType;

    // Add to physics system
    PhysicsSystem::getInstance().addPhysicsComponent(m_entity.getID(), physicsComp);
}

void AGameObject::disablePhysics()
{
    if (hasPhysics())
    {
        PhysicsSystem::getInstance().removePhysicsComponent(m_entity.getID());
    }
}

bool AGameObject::hasPhysics() const
{
    auto& componentManager = ComponentManager::getInstance();
    return componentManager.hasComponent<PhysicsComponent>(m_entity.getID());
}

void AGameObject::setPhysicsMass(float mass)
{
    auto& componentManager = ComponentManager::getInstance();
    auto* physicsComp = componentManager.getComponent<PhysicsComponent>(m_entity.getID());

    if (physicsComp)
    {
        physicsComp->mass = mass;
        if (physicsComp->rigidBody && physicsComp->bodyType == PhysicsBodyType::Dynamic)
        {
            physicsComp->rigidBody->setMass(mass);
        }
    }
}

void AGameObject::setPhysicsRestitution(float restitution)
{
    auto& componentManager = ComponentManager::getInstance();
    auto* physicsComp = componentManager.getComponent<PhysicsComponent>(m_entity.getID());

    if (physicsComp)
    {
        physicsComp->restitution = restitution;
        if (physicsComp->collider)
        {
            physicsComp->collider->getMaterial().setBounciness(restitution);
        }
    }
}

void AGameObject::setPhysicsFriction(float friction)
{
    auto& componentManager = ComponentManager::getInstance();
    auto* physicsComp = componentManager.getComponent<PhysicsComponent>(m_entity.getID());

    if (physicsComp)
    {
        physicsComp->friction = friction;
        if (physicsComp->collider)
        {
            physicsComp->collider->getMaterial().setFrictionCoefficient(friction);
        }
    }
}

void AGameObject::applyForce(const Vector3& force)
{
    auto& componentManager = ComponentManager::getInstance();
    auto* physicsComp = componentManager.getComponent<PhysicsComponent>(m_entity.getID());

    if (physicsComp && physicsComp->rigidBody)
    {
        rp3d::Vector3 reactForce = PhysicsSystem::toReactVector(force);
        physicsComp->rigidBody->applyWorldForceAtCenterOfMass(reactForce);
    }
}

void AGameObject::applyImpulse(const Vector3& impulse)
{
    auto& componentManager = ComponentManager::getInstance();
    auto* physicsComp = componentManager.getComponent<PhysicsComponent>(m_entity.getID());

    if (physicsComp && physicsComp->rigidBody)
    {
        rp3d::Vector3 currentVelocity = physicsComp->rigidBody->getLinearVelocity();

        rp3d::Vector3 reactImpulse = PhysicsSystem::toReactVector(impulse);
        float mass = physicsComp->rigidBody->getMass();

        rp3d::Vector3 velocityChange = reactImpulse / mass;
        rp3d::Vector3 newVelocity = currentVelocity + velocityChange;

        physicsComp->rigidBody->setLinearVelocity(newVelocity);
    }
}

Vector3 AGameObject::getLinearVelocity() const
{
    auto& componentManager = ComponentManager::getInstance();
    auto* physicsComp = componentManager.getComponent<PhysicsComponent>(m_entity.getID());

    if (physicsComp && physicsComp->rigidBody)
    {
        return PhysicsSystem::fromReactVector(physicsComp->rigidBody->getLinearVelocity());
    }

    return Vector3(0, 0, 0);
}

void AGameObject::setLinearVelocity(const Vector3& velocity)
{
    auto& componentManager = ComponentManager::getInstance();
    auto* physicsComp = componentManager.getComponent<PhysicsComponent>(m_entity.getID());

    if (physicsComp && physicsComp->rigidBody)
    {
        rp3d::Vector3 reactVelocity = PhysicsSystem::toReactVector(velocity);
        physicsComp->rigidBody->setLinearVelocity(reactVelocity);
    }
}

std::string AGameObject::getObjectType() 
{
	const std::type_info& typeInfo = typeid(*this);
	std::string rawName = typeInfo.name();

	// Initially returns as "class dx3d::[GameObjectType]" but I want to make it as "[GameObjectType]"
	std::string removedPrefix = "class dx3d::";
	int removdPrefixLength = removedPrefix.length();

	size_t startPos = rawName.find(removedPrefix);

    if (startPos != std::string::npos) {
		// Remove the prefix "class dx3d::" if it exists
        rawName.erase(startPos, removdPrefixLength);
    }
	return rawName; // Returns a string representation of the type name
}

PhysicsComponent AGameObject::createPhysicsComponent() const
{
    PhysicsComponent component;
    component.shapeType = getCollisionShapeType();

    // Set shape parameters based on current scale
    Vector3 scale = getScale();

    switch (component.shapeType)
    {
    case CollisionShapeType::Box:
        component.boxHalfExtents = Vector3(scale.x * 0.5f, scale.y * 0.5f, scale.z * 0.5f);
        break;

    case CollisionShapeType::Sphere:
        component.sphereRadius = scale.x * 0.5f; // Use X scale for radius
        break;

    case CollisionShapeType::Cylinder:
        component.cylinderRadius = scale.x * 0.5f;
        component.cylinderHeight = scale.y;
        break;

    case CollisionShapeType::Capsule:
        component.capsuleRadius = scale.x * 0.5f;
        component.capsuleHeight = scale.y;
        break;

    case CollisionShapeType::Plane:
        component.boxHalfExtents = Vector3(scale.x * 0.5f, 0.01f, scale.z * 0.5f);
        break;
    }

    return component;
}

void AGameObject::syncTransformFromECS()
{
    auto& componentManager = ComponentManager::getInstance();
    auto* transformComp = componentManager.getComponent<TransformComponent>(m_entity.getID());

    if (transformComp)
    {
        m_transform.position = transformComp->position;
        m_transform.rotation = transformComp->rotation;
        m_transform.scale = transformComp->scale;
    }
}

void AGameObject::syncTransformToECS()
{
    auto& componentManager = ComponentManager::getInstance();
    auto* transformComp = componentManager.getComponent<TransformComponent>(m_entity.getID());

    if (transformComp)
    {
        transformComp->position = m_transform.position;
        transformComp->rotation = m_transform.rotation;
        transformComp->scale = m_transform.scale;

        if (hasPhysics())
        {
            auto* physicsComp = componentManager.getComponent<PhysicsComponent>(m_entity.getID());
            if (physicsComp && physicsComp->rigidBody)
            {
                rp3d::Vector3 position = PhysicsSystem::toReactVector(m_transform.position);
                rp3d::Quaternion orientation = PhysicsSystem::toReactQuaternion(m_transform.rotation);
                rp3d::Transform transform(position, orientation);
                physicsComp->rigidBody->setTransform(transform);
            }
        }
    }
}

Matrix4x4 AGameObject::Transform::getWorldMatrix() const
{
    Matrix4x4 scaleMatrix = Matrix4x4::CreateScale(scale);
    Matrix4x4 rotationX = Matrix4x4::CreateRotationX(rotation.x);
    Matrix4x4 rotationY = Matrix4x4::CreateRotationY(rotation.y);
    Matrix4x4 rotationZ = Matrix4x4::CreateRotationZ(rotation.z);
    Matrix4x4 translationMatrix = Matrix4x4::CreateTranslation(position);

    Matrix4x4 result = scaleMatrix * rotationZ * rotationY * rotationX * translationMatrix;
    return result;
}

void AGameObject::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;

    m_enabled = enabled;

    if (!enabled)
    {
        // Remember if we had physics before disabling
        m_hadPhysicsBeforeDisable = hasPhysics();
        if (m_hadPhysicsBeforeDisable)
        {
            // Store the body type before disabling
            auto& componentManager = ComponentManager::getInstance();
            auto* physicsComp = componentManager.getComponent<PhysicsComponent>(m_entity.getID());
            if (physicsComp)
            {
                m_previousBodyType = physicsComp->bodyType;
            }
            disablePhysics();
        }
    }
    else
    {
        // Re-enable physics if we had it before
        if (m_hadPhysicsBeforeDisable)
        {
            enablePhysics(m_previousBodyType);

            // Restore physics properties if needed
            auto& componentManager = ComponentManager::getInstance();
            auto* physicsComp = componentManager.getComponent<PhysicsComponent>(m_entity.getID());
            if (physicsComp)
            {
                // The physics properties should be restored from the component
                // The enablePhysics method should handle this
            }
        }
    }
}