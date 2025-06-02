#include <DX3D/Graphics/Particles/Particles.h>

using namespace dx3d;

Particle::Particle()
{
    // Default initialization is done in header
}

void Particle::update(float deltaTime)
{
    if (!m_active || m_life <= 0.0f)
        return;

    // Update lifetime
    m_life -= deltaTime * m_fadeSpeed;
    if (m_life <= 0.0f)
    {
        m_life = 0.0f;
        m_active = false;
        return;
    }

    // Calculate life ratio for interpolations
    float lifeRatio = getLifeRatio();

    // Update physics
    m_velocity += m_acceleration * deltaTime;
    m_position += m_velocity * deltaTime;

    // Update rotation
    m_rotation += m_rotationSpeed * deltaTime;

    // Interpolate size
    m_size = m_startSize + (m_endSize - m_startSize) * (1.0f - lifeRatio);

    // Interpolate color
    m_color = Vec4::lerp(m_endColor, m_startColor, lifeRatio);
}

void Particle::reset(const Vec2& position, const Vec2& velocity)
{
    m_position = position;
    m_velocity = velocity;
    m_acceleration = Vec2(0.0f, 0.0f);

    m_life = m_maxLife;
    m_size = m_startSize;
    m_color = m_startColor;
    m_rotation = 0.0f;

    m_active = true;
}