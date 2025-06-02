#include <DX3D/Graphics/Particles/FireParticle.h>
#include <cmath>

using namespace dx3d;

// FireParticle Implementation
FireParticle::FireParticle()
{
    // Fire-specific defaults
    m_maxLife = 1.5f;
    m_fadeSpeed = 1.0f;

    // Fire colors (yellow-orange to red to dark smoke)
    m_startColor = Vec4(1.0f, 0.8f, 0.2f, 1.0f);  // Bright yellow-orange
    m_endColor = Vec4(0.2f, 0.0f, 0.0f, 0.0f);    // Dark red, transparent

    m_startSize = 0.08f;
    m_endSize = 0.02f;
}

void FireParticle::update(float deltaTime)
{
    if (!m_active || m_life <= 0.0f)
        return;

    // Base particle update
    Particle::update(deltaTime);

    // Fire-specific behavior
    float lifeRatio = getLifeRatio();

    // Add turbulence (side-to-side motion)
    float turbulenceAmount = (1.0f - lifeRatio) * m_turbulence;
    m_velocity.x += std::sin(m_life * 10.0f) * turbulenceAmount * deltaTime;

    // Accelerate upward (fire rises)
    m_velocity.y += m_riseSpeed * deltaTime;

    // Color transition through fire spectrum
    if (lifeRatio > 0.7f)
    {
        // Hot core: white to yellow
        float t = (lifeRatio - 0.7f) / 0.3f;
        m_color = Vec4::lerp(
            Vec4(1.0f, 0.8f, 0.2f, 1.0f),  // Yellow
            Vec4(1.0f, 1.0f, 0.9f, 1.0f),  // Almost white
            t
        );
    }
    else if (lifeRatio > 0.4f)
    {
        // Mid flame: yellow to orange
        float t = (lifeRatio - 0.4f) / 0.3f;
        m_color = Vec4::lerp(
            Vec4(1.0f, 0.4f, 0.1f, 0.8f),  // Orange
            Vec4(1.0f, 0.8f, 0.2f, 1.0f),  // Yellow
            t
        );
    }
    else
    {
        // Cooling: orange to dark smoke
        float t = lifeRatio / 0.4f;
        m_color = Vec4::lerp(
            Vec4(0.1f, 0.1f, 0.1f, 0.0f),  // Dark smoke, transparent
            Vec4(1.0f, 0.4f, 0.1f, 0.8f),  // Orange
            t
        );
    }
}

void FireParticle::reset(const Vec2& position, const Vec2& velocity)
{
    Particle::reset(position, velocity);

    // Reset fire-specific properties
    m_turbulence = 0.5f;
    m_riseSpeed = 0.8f;
}

void FireParticle::initializeFireProperties(float turbulence, float riseSpeed)
{
    m_turbulence = turbulence;
    m_riseSpeed = riseSpeed;
}

// FireEmitter Implementation
FireEmitter::FireEmitter(size_t maxParticles)
    : ParticleEmitter(maxParticles)
{
}

void FireEmitter::update(float deltaTime)
{
    ParticleEmitter::update(deltaTime);

    // Continuous emission
    if (m_active)
    {
        m_emissionTimer += deltaTime;

        while (m_emissionTimer >= m_emissionRate)
        {
            m_emissionTimer -= m_emissionRate;

            // Emit particles based on intensity
            int particlesToEmit = static_cast<int>(m_intensity * 2.0f);
            emit(m_position, particlesToEmit);
        }
    }
}

std::unique_ptr<Particle> FireEmitter::createParticle()
{
    return std::make_unique<FireParticle>();
}

void FireEmitter::initializeParticle(Particle* particle, const Vec2& emitPosition)
{
    FireParticle* fireParticle = static_cast<FireParticle*>(particle);

    // Random position within flame base
    float offsetX = randomFloat(-m_flameWidth * 0.5f, m_flameWidth * 0.5f);
    Vec2 pos = emitPosition + Vec2(offsetX, 0.0f);

    // Initial upward velocity with some randomness
    float upSpeed = randomFloat(0.3f, 0.6f) * m_intensity;
    float sideSpeed = randomFloat(-0.1f, 0.1f);
    Vec2 vel(sideSpeed, upSpeed);

    fireParticle->reset(pos, vel);

    // Set fire properties using setters
    fireParticle->setLifetime(randomFloat(1.0f, 2.0f));

    float startSize = randomFloat(0.05f, 0.1f) * m_intensity;
    fireParticle->setSizeRange(startSize, startSize * 0.2f);

    fireParticle->setRotationSpeed(randomFloat(-2.0f, 2.0f));

    // Fire-specific properties
    float riseSpeed = randomFloat(0.6f, 1.0f) * m_intensity;
    float turbulence = randomFloat(0.3f, 0.7f);
    fireParticle->initializeFireProperties(turbulence, riseSpeed);
}