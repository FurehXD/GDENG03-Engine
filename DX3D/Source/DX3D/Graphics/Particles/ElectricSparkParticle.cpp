#include <DX3D/Graphics/Particles/ElectricSparkParticle.h>
#include <cmath>

using namespace dx3d;

// ElectricSparkParticle Implementation
ElectricSparkParticle::ElectricSparkParticle()
{
    // Electric spark defaults
    m_maxLife = 0.3f;  // Short lived
    m_fadeSpeed = 3.0f; // Fast fade

    // Electric blue-white colors
    m_startColor = Vec4(0.7f, 0.9f, 1.0f, 1.0f);  // Bright blue-white
    m_endColor = Vec4(0.2f, 0.4f, 1.0f, 0.0f);    // Deep blue, transparent

    m_startSize = 0.04f;
    m_endSize = 0.01f;
}

void ElectricSparkParticle::update(float deltaTime)
{
    if (!m_active || m_life <= 0.0f)
        return;

    // Base particle update
    Particle::update(deltaTime);

    // Electric spark behavior
    m_flickerTimer += deltaTime;

    if (m_flickerTimer >= m_flickerRate)
    {
        m_flickerTimer = 0.0f;

        // Simple brightness oscillation instead of random
        m_brightness = 0.5f + 0.5f * std::sin(m_life * 20.0f);

        // Add jitter to position using sine/cosine for variation
        float jitterAmount = 0.01f;
        m_jitter.x = jitterAmount * std::sin(m_life * 30.0f);
        m_jitter.y = jitterAmount * std::cos(m_life * 25.0f);
    }

    // Apply brightness flicker
    float lifeRatio = getLifeRatio();
    m_color.w = m_startColor.w * m_brightness * lifeRatio;

    // Make the color more intense (whiter) when brighter
    float intensity = m_brightness;
    m_color.x = m_startColor.x + (1.0f - m_startColor.x) * intensity * 0.5f;
    m_color.y = m_startColor.y + (1.0f - m_startColor.y) * intensity * 0.5f;

    // Apply position jitter
    m_position += m_jitter;

    // Electric particles slow down quickly
    m_velocity *= (1.0f - 2.0f * deltaTime);
}

void ElectricSparkParticle::reset(const Vec2& position, const Vec2& velocity)
{
    Particle::reset(position, velocity);

    m_flickerTimer = 0.0f;
    m_brightness = 1.0f;
    m_jitter = Vec2(0.0f, 0.0f);
}

// ElectricSparkEmitter Implementation
ElectricSparkEmitter::ElectricSparkEmitter(size_t maxParticles)
    : ParticleEmitter(maxParticles)
{
}

void ElectricSparkEmitter::update(float deltaTime)
{
    ParticleEmitter::update(deltaTime);

    // Continuous sparking mode
    if (m_continuous && m_active)
    {
        m_continuousTimer += deltaTime;

        if (m_continuousTimer >= m_continuousRate)
        {
            m_continuousTimer = 0.0f;
            spark(m_position, m_sparkIntensity);
        }
    }
}

void ElectricSparkEmitter::spark(const Vec2& position, float intensity)
{
    if (!m_active)
        return;

    // Number of sparks based on intensity
    int sparkCount = static_cast<int>(20 * intensity);

    for (int i = 0; i < sparkCount; ++i)
    {
        if (m_particles.size() >= m_maxParticles)
            break;

        auto particle = createParticle();
        if (particle)
        {
            initializeParticle(particle.get(), position);
            m_particles.push_back(std::move(particle));
        }
    }
}

std::unique_ptr<Particle> ElectricSparkEmitter::createParticle()
{
    return std::make_unique<ElectricSparkParticle>();
}

void ElectricSparkEmitter::initializeParticle(Particle* particle, const Vec2& emitPosition)
{
    ElectricSparkParticle* sparkParticle = static_cast<ElectricSparkParticle*>(particle);

    // Random direction in a cone or sphere
    Vec2 direction = randomDirection();
    float speed = randomFloat(0.5f, 2.0f) * m_sparkIntensity;
    Vec2 velocity = direction * speed;

    // Start position with small random offset
    float offsetRadius = randomFloat(0.0f, m_sparkRadius * 0.1f);
    Vec2 offset = randomDirection() * offsetRadius;
    Vec2 pos = emitPosition + offset;

    sparkParticle->reset(pos, velocity);

    // Set spark properties using setters
    sparkParticle->setLifetime(randomFloat(0.1f, 0.4f));

    float startSize = randomFloat(0.02f, 0.06f) * m_sparkIntensity;
    sparkParticle->setSizeRange(startSize, startSize * 0.2f);

    // Some sparks are brighter/whiter
    if (randomFloat(0.0f, 1.0f) > 0.7f)
    {
        // Bright white spark
        sparkParticle->setColorRange(
            Vec4(0.9f, 0.95f, 1.0f, 1.0f),
            Vec4(0.4f, 0.6f, 1.0f, 0.0f)
        );
    }
    else
    {
        // Normal blue spark
        sparkParticle->setColorRange(
            Vec4(0.5f, 0.8f, 1.0f, 1.0f),
            Vec4(0.2f, 0.4f, 1.0f, 0.0f)
        );
    }

    // Flicker rate variation
    sparkParticle->setFlickerRate(randomFloat(0.01f, 0.03f));
}