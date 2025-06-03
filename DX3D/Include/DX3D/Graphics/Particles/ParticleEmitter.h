#pragma once
#include <DX3D/Graphics/Particles/Particles.h>
#include <vector>
#include <memory>
#include <random>

namespace dx3d
{
    class ParticleEmitter
    {
    public:
        ParticleEmitter(size_t maxParticles = 100);
        virtual ~ParticleEmitter() = default;

        virtual void update(float deltaTime);

        virtual void emit(const Vec2& position, int count = 1);

        const std::vector<std::unique_ptr<Particle>>& getParticles() const { return m_particles; }

        void setActive(bool active) { m_active = active; }
        bool isActive() const { return m_active; }

        void setPosition(const Vec2& position) { m_position = position; }
        const Vec2& getPosition() const { return m_position; }

    protected:
        virtual std::unique_ptr<Particle> createParticle() = 0;

        virtual void initializeParticle(Particle* particle, const Vec2& emitPosition) = 0;

        float randomFloat(float min, float max);

        Vec2 randomDirection();

    protected:
        std::vector<std::unique_ptr<Particle>> m_particles;
        size_t m_maxParticles;
        Vec2 m_position{ 0.0f, 0.0f };
        bool m_active{ true };

        std::mt19937 m_randomEngine;
        std::uniform_real_distribution<float> m_uniformDist{ 0.0f, 1.0f };
    };
}