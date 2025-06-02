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

        // Update all particles
        virtual void update(float deltaTime);

        // Emit new particles
        virtual void emit(const Vec2& position, int count = 1);

        // Get active particles for rendering
        const std::vector<std::unique_ptr<Particle>>& getParticles() const { return m_particles; }

        // Control emission
        void setActive(bool active) { m_active = active; }
        bool isActive() const { return m_active; }

        // Set emitter position
        void setPosition(const Vec2& position) { m_position = position; }
        const Vec2& getPosition() const { return m_position; }

    protected:
        // Create a new particle (to be implemented by derived classes)
        virtual std::unique_ptr<Particle> createParticle() = 0;

        // Initialize a particle with emitter-specific parameters
        virtual void initializeParticle(Particle* particle, const Vec2& emitPosition) = 0;

        // Helper function to get random float in range
        float randomFloat(float min, float max);

        // Helper function to get random direction
        Vec2 randomDirection();

    protected:
        std::vector<std::unique_ptr<Particle>> m_particles;
        size_t m_maxParticles;
        Vec2 m_position{ 0.0f, 0.0f };
        bool m_active{ true };

        // Random number generation
        std::mt19937 m_randomEngine;
        std::uniform_real_distribution<float> m_uniformDist{ 0.0f, 1.0f };
    };
}