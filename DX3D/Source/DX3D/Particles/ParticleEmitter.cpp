#include <DX3D/Particles/ParticleEmitter.h>
#include <DX3D/Game/SceneCamera.h>
#include <random>
#include <algorithm>

using namespace dx3d;

ParticleEmitter::ParticleEmitter(const EmitterConfig& config, ParticleFactory factory)
    : m_config(config)
    , m_particleFactory(factory)
    , m_activeParticleCount(0)
    , m_emissionAccumulator(0.0f)
    , m_active(true)
{
    // Pre-allocate particle pool
    m_particles.reserve(config.maxParticles);
    for (ui32 i = 0; i < config.maxParticles; ++i)
    {
        m_particles.push_back(m_particleFactory());
    }
}

ParticleEmitter::~ParticleEmitter()
{
}

void ParticleEmitter::update(float deltaTime)
{
    // Update existing particles
    m_activeParticleCount = 0;
    for (auto& particle : m_particles)
    {
        if (particle->isAlive())
        {
            if (particle->update(deltaTime))
            {
                m_activeParticleCount++;

                // Update particle properties based on age
                float lifeRatio = particle->getAge() / particle->getLifetime();

                // Interpolate color
                Vector4 color;
                color.x = m_config.startColor.x + (m_config.endColor.x - m_config.startColor.x) * lifeRatio;
                color.y = m_config.startColor.y + (m_config.endColor.y - m_config.startColor.y) * lifeRatio;
                color.z = m_config.startColor.z + (m_config.endColor.z - m_config.startColor.z) * lifeRatio;
                color.w = m_config.startColor.w + (m_config.endColor.w - m_config.startColor.w) * lifeRatio;
                particle->setColor(color);

                // Interpolate size
                float size = m_config.startSize + (m_config.endSize - m_config.startSize) * lifeRatio;
                particle->setSize(size);
            }
        }
    }

    // Spawn new particles if emitter is active
    if (m_active && m_config.loop)
    {
        m_emissionAccumulator += m_config.emissionRate * deltaTime;

        while (m_emissionAccumulator >= 1.0f && m_activeParticleCount < m_config.maxParticles)
        {
            spawnParticle();
            m_emissionAccumulator -= 1.0f;
        }
    }
}

void ParticleEmitter::fillInstanceData(std::vector<ParticleInstanceData>& instanceData, const SceneCamera& camera)
{
    for (const auto& particle : m_particles)
    {
        if (particle->isAlive())
        {
            ParticleInstanceData data;
            data.position = particle->getPosition();
            data.size = particle->getSize();
            data.color = particle->getColor();
            data.rotation = particle->getRotation();

            instanceData.push_back(data);
        }
    }
}

void ParticleEmitter::reset()
{
    for (auto& particle : m_particles)
    {
        particle->reset();
    }
    m_activeParticleCount = 0;
    m_emissionAccumulator = 0.0f;
}

void ParticleEmitter::spawnParticle()
{
    // Find an inactive particle to reuse
    for (auto& particle : m_particles)
    {
        if (!particle->isAlive())
        {
            // Initialize with randomized parameters
            Vector3 position = randomizeVector(m_config.position, m_config.positionVariance);
            Vector3 velocity = randomizeVector(m_config.velocity, m_config.velocityVariance);

            particle->initialize(position, velocity);
            particle->setAcceleration(m_config.acceleration);
            particle->setColor(m_config.startColor);
            particle->setSize(m_config.startSize);

            float lifetime = m_config.lifetime + randomFloat(-m_config.lifetimeVariance, m_config.lifetimeVariance);
            particle->setLifetime(std::max(0.1f, lifetime));

            m_activeParticleCount++;
            break;
        }
    }
}

Vector3 ParticleEmitter::randomizeVector(const Vector3& base, const Vector3& variance)
{
    return Vector3(
        base.x + randomFloat(-variance.x, variance.x),
        base.y + randomFloat(-variance.y, variance.y),
        base.z + randomFloat(-variance.z, variance.z)
    );
}

float ParticleEmitter::randomFloat(float min, float max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(gen);
}
