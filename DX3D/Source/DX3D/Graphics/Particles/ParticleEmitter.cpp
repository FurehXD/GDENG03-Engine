#include <DX3D/Graphics/Particles/ParticleEmitter.h>
#include <algorithm>
#include <cmath>

using namespace dx3d;

ParticleEmitter::ParticleEmitter(size_t maxParticles)
    : m_maxParticles(maxParticles)
    , m_randomEngine(std::random_device{}())
{
    m_particles.reserve(maxParticles);
}

void ParticleEmitter::update(float deltaTime)
{
    for (auto& particle : m_particles)
    {
        if (particle && particle->isActive())
        {
            particle->update(deltaTime);
        }
    }

    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(),
            [](const std::unique_ptr<Particle>& p) {
                return !p || p->isDead();
            }),
        m_particles.end()
    );
}

void ParticleEmitter::emit(const Vec2& position, int count)
{
    if (!m_active)
        return;

    for (int i = 0; i < count; ++i)
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

float ParticleEmitter::randomFloat(float min, float max)
{
    return min + (max - min) * m_uniformDist(m_randomEngine);
}

Vec2 ParticleEmitter::randomDirection()
{
    float angle = randomFloat(0.0f, 2.0f * 3.14159265f);
    return Vec2(std::cos(angle), std::sin(angle));
}