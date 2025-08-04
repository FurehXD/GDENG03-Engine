#include <DX3D/Physics/PhysicsSystem.h>
#include <DX3D/ECS/ComponentManager.h>
#include <DX3D/ECS/Components/TransformComponent.h>
#include <DX3D/ECS/Components/PhysicsComponent.h>
#include <cmath>
#include <cstdio>

using namespace dx3d;

void PhysicsSystem::initialize()
{
    if (m_initialized)
        return;

    // Create physics world
    rp3d::PhysicsWorld::WorldSettings settings;
    settings.defaultVelocitySolverNbIterations = 6;
    settings.defaultPositionSolverNbIterations = 3;
    settings.isSleepingEnabled = true;
    settings.gravity = rp3d::Vector3(0, -9.81f, 0);

    m_physicsWorld = m_physicsCommon.createPhysicsWorld(settings);

    if (!m_physicsWorld)
    {
        printf("Failed to create ReactPhysics3D world\n");
        return;
    }

    m_initialized = true;
    printf("PhysicsSystem initialized successfully\n");
}

void PhysicsSystem::shutdown()
{
    if (!m_initialized)
        return;

    if (m_physicsWorld)
    {
        m_physicsCommon.destroyPhysicsWorld(m_physicsWorld);
        m_physicsWorld = nullptr;
    }

    m_initialized = false;
    printf("PhysicsSystem shutdown complete\n");
}

void PhysicsSystem::addPhysicsComponent(EntityID entity, const PhysicsComponent& component)
{
    if (!m_initialized)
    {
        printf("PhysicsSystem not initialized\n");
        return;
    }

    auto& componentManager = ComponentManager::getInstance();

    // Add the component
    PhysicsComponent physicsComp = component;
    componentManager.addComponent(entity, physicsComp);

    // Initialize the physics body
    initializePhysicsBody(entity, physicsComp);

    // Update the component with initialized physics objects
    componentManager.addComponent(entity, physicsComp);
}

void PhysicsSystem::removePhysicsComponent(EntityID entity)
{
    auto& componentManager = ComponentManager::getInstance();
    auto* physicsComp = componentManager.getComponent<PhysicsComponent>(entity);

    if (physicsComp && physicsComp->rigidBody)
    {
        m_physicsWorld->destroyRigidBody(physicsComp->rigidBody);
        physicsComp->rigidBody = nullptr;
        physicsComp->collider = nullptr;
    }

    componentManager.removeComponent<PhysicsComponent>(entity);
}

void PhysicsSystem::updatePhysicsComponent(EntityID entity, const PhysicsComponent& component)
{
    removePhysicsComponent(entity);
    addPhysicsComponent(entity, component);
}

rp3d::CollisionShape* PhysicsSystem::createCollisionShape(CollisionShapeType type, const PhysicsComponent& component)
{
    switch (type)
    {
    case CollisionShapeType::Box:
    {
        rp3d::Vector3 halfExtents = toReactVector(component.boxHalfExtents);
        return m_physicsCommon.createBoxShape(halfExtents);
    }

    case CollisionShapeType::Sphere:
    {
        return m_physicsCommon.createSphereShape(component.sphereRadius);
    }

    case CollisionShapeType::Cylinder:
    {
        return m_physicsCommon.createCapsuleShape(component.cylinderRadius, component.cylinderHeight);
    }

    case CollisionShapeType::Capsule:
    {
        return m_physicsCommon.createCapsuleShape(component.capsuleRadius, component.capsuleHeight);
    }

    case CollisionShapeType::Plane:
    {
        // Create a very thin box for plane collision
        rp3d::Vector3 halfExtents = toReactVector(component.boxHalfExtents);
        halfExtents.y = 0.01f; // Very thin
        return m_physicsCommon.createBoxShape(halfExtents);
    }

    default:
        printf("Unknown collision shape type\n");
        return m_physicsCommon.createBoxShape(rp3d::Vector3(0.5f, 0.5f, 0.5f));
    }
}

void PhysicsSystem::update(float deltaTime)
{
    if (!m_initialized)
        return;

    // Fixed timestep physics integration
    m_accumulator += deltaTime;

    while (m_accumulator >= m_fixedTimeStep)
    {
        m_physicsWorld->update(m_fixedTimeStep);
        m_accumulator -= m_fixedTimeStep;
    }

    // Sync transforms from physics to ECS
    auto& componentManager = ComponentManager::getInstance();

    auto* physicsArray = componentManager.getComponentArray<PhysicsComponent>();


    if (physicsArray)
    {
        // FIX: Use range-based for loop with non-const references
        for (auto& pair : *physicsArray)
        {
            EntityID entity = pair.first;
            const PhysicsComponent& physicsComp = pair.second;

            if (physicsComp.rigidBody && physicsComp.bodyType == PhysicsBodyType::Dynamic)
            {
                syncTransformFromPhysics(entity, physicsComp);
            }
        }
    }
}

void PhysicsSystem::initializePhysicsBody(EntityID entity, PhysicsComponent& component)
{
    auto& componentManager = ComponentManager::getInstance();
    auto* transformComp = componentManager.getComponent<TransformComponent>(entity);

    if (!transformComp)
    {
        printf("Entity missing TransformComponent for physics initialization\n");
        return;
    }

    // Create collision shape
    rp3d::CollisionShape* shape = createCollisionShape(component.shapeType, component);
    if (!shape)
    {
        printf("Failed to create collision shape\n");
        return;
    }

    // Create rigid body
    rp3d::Vector3 position = toReactVector(transformComp->position);
    rp3d::Quaternion orientation = toReactQuaternion(transformComp->rotation);
    rp3d::Transform transform(position, orientation);

    component.rigidBody = m_physicsWorld->createRigidBody(transform);

    if (!component.rigidBody)
    {
        printf("Failed to create rigid body\n");
        return;
    }

    // Set body type
    switch (component.bodyType)
    {
    case PhysicsBodyType::Static:
        component.rigidBody->setType(rp3d::BodyType::STATIC);
        break;
    case PhysicsBodyType::Kinematic:
        component.rigidBody->setType(rp3d::BodyType::KINEMATIC);
        break;
    case PhysicsBodyType::Dynamic:
        component.rigidBody->setType(rp3d::BodyType::DYNAMIC);
        break;
    }

    // Add collider
    component.collider = component.rigidBody->addCollider(shape, rp3d::Transform::identity());

    if (!component.collider)
    {
        printf("Failed to add collider to rigid body\n");
        return;
    }

    // Set physics properties
    if (component.bodyType == PhysicsBodyType::Dynamic)
    {
        component.rigidBody->setMass(component.mass);
    }

    rp3d::Material& material = component.collider->getMaterial();
    material.setBounciness(component.restitution);
    material.setFrictionCoefficient(component.friction);

    component.isInitialized = true;
}

void PhysicsSystem::syncTransformFromPhysics(EntityID entity, const PhysicsComponent& component)
{
    if (!component.rigidBody)
        return;

    auto& componentManager = ComponentManager::getInstance();
    auto* transformComp = componentManager.getComponent<TransformComponent>(entity);

    if (!transformComp)
        return;

    // Get physics transform
    const rp3d::Transform& physicsTransform = component.rigidBody->getTransform();

    // Update transform component
    transformComp->position = fromReactVector(physicsTransform.getPosition());
    transformComp->rotation = fromReactQuaternion(physicsTransform.getOrientation());

    // Scale is not affected by physics
}

// Utility conversion functions
rp3d::Vector3 PhysicsSystem::toReactVector(const Vector3& vec)
{
    return rp3d::Vector3(vec.x, vec.y, vec.z);
}

Vector3 PhysicsSystem::fromReactVector(const rp3d::Vector3& vec)
{
    return Vector3(vec.x, vec.y, vec.z);
}

rp3d::Quaternion PhysicsSystem::toReactQuaternion(const Vector3& eulerAngles)
{
    // Convert Euler angles to quaternion
    float cx = std::cos(eulerAngles.x * 0.5f);
    float sx = std::sin(eulerAngles.x * 0.5f);
    float cy = std::cos(eulerAngles.y * 0.5f);
    float sy = std::sin(eulerAngles.y * 0.5f);
    float cz = std::cos(eulerAngles.z * 0.5f);
    float sz = std::sin(eulerAngles.z * 0.5f);

    rp3d::Quaternion quat;
    quat.w = cx * cy * cz + sx * sy * sz;
    quat.x = sx * cy * cz - cx * sy * sz;
    quat.y = cx * sy * cz + sx * cy * sz;
    quat.z = cx * cy * sz - sx * sy * cz;

    return quat;
}

Vector3 PhysicsSystem::fromReactQuaternion(const rp3d::Quaternion& quat)
{
    // Convert quaternion to Euler angles
    Vector3 euler;

    // Roll (x-axis rotation)
    float sinr_cosp = 2 * (quat.w * quat.x + quat.y * quat.z);
    float cosr_cosp = 1 - 2 * (quat.x * quat.x + quat.y * quat.y);
    euler.x = std::atan2(sinr_cosp, cosr_cosp);

    // Pitch (y-axis rotation)
    float sinp = 2 * (quat.w * quat.y - quat.z * quat.x);
    if (std::abs(sinp) >= 1)
        euler.y = std::copysign(3.14159265f / 2, sinp);
    else
        euler.y = std::asin(sinp);

    // Yaw (z-axis rotation)
    float siny_cosp = 2 * (quat.w * quat.z + quat.x * quat.y);
    float cosy_cosp = 1 - 2 * (quat.y * quat.y + quat.z * quat.z);
    euler.z = std::atan2(siny_cosp, cosy_cosp);

    return euler;
}