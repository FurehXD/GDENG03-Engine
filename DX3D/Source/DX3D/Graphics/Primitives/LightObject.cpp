#include <DX3D/Graphics/Primitives/LightObject.h>

using namespace dx3d;

void LightObject::update(float deltaTime)
{
    AGameObject::update(deltaTime);

    m_lightData.position = getPosition();

    Matrix4x4 world = getWorldMatrix();
    m_lightData.direction = Vector3(world.m[2][0], world.m[2][1], world.m[2][2]);
}

DirectionalLight::DirectionalLight()
{
    m_lightData.type = LIGHT_TYPE_DIRECTIONAL;
    m_lightData.color = Vector3(1.0f, 1.0f, 1.0f); // White light
    m_lightData.intensity = 1.0f;
}

PointLight::PointLight()
{
    m_lightData.type = LIGHT_TYPE_POINT;
    m_lightData.color = Vector3(1.0f, 1.0f, 1.0f); // White light
    m_lightData.intensity = 1.0f;
    m_lightData.radius = 10.0f;
}

SpotLight::SpotLight()
{
    m_lightData.type = LIGHT_TYPE_SPOT;
    m_lightData.color = Vector3(1.0f, 1.0f, 1.0f); // White light
    m_lightData.intensity = 1.0f;
    m_lightData.radius = 20.0f;
}