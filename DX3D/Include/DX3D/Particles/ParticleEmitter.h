#pragma once
#include <DX3D/Core/Core.h>
#include <DX3D/Math/Math.h>
#include <DX3D/Particles/Particle.h>
#include <memory>
#include <vector>
#include <functional>

namespace dx3d
{
    // Forward declaration to resolve compiler errors
    class SceneCamera;

    // Structure to hold per-instance data for rendering
    struct ParticleInstanceData
    {
        Vector3 position;
        float size;
        Vector4 color;
        float rotation;
        float _padding[3]; // Ensure 16-byte alignment
    };

    class ParticleEmitter
    {
    public:
        struct EmitterConfig
        {
            Vector3 position = Vector3(0, 0, 0);
            Vector3 positionVariance = Vector3(0, 0, 0);
            Vector3 velocity = Vector3(0, 1, 0);
            Vector3 velocityVariance = Vector3(0.5f, 0.5f, 0.5f);
            Vector3 acceleration = Vector3(0, -9.81f, 0);  // Gravity
            Vector4 startColor = Vector4(1, 1, 1, 1);
            Vector4 endColor = Vector4(1, 1, 1, 0);
            float startSize = 1.0f;
            float endSize = 0.5f;
            float lifetime = 5.0f;
            float lifetimeVariance = 1.0f;
            float emissionRate = 10.0f;  // Particles per second
            ui32 maxParticles = 1000;
            bool localSpace = false;     // If true, particles move with emitter
            bool loop = true;           // Continuous emission
        };

        using ParticleFactory = std::function<std::unique_ptr<Particle>()>;

        ParticleEmitter(const EmitterConfig& config, ParticleFactory factory);
        ~ParticleEmitter();

        // Update all particles and spawn new ones
        void update(float deltaTime);

        // Get instance data for rendering
        void fillInstanceData(std::vector<ParticleInstanceData>& instanceData, const SceneCamera& camera);

        // Control methods
        void start() { m_active = true; }
        void stop() { m_active = false; }
        void reset();

        // Setters
        void setPosition(const Vector3& position) { m_config.position = position; }
        void setEmissionRate(float rate) { m_config.emissionRate = rate; }

        // Getters
        const Vector3& getPosition() const { return m_config.position; }
        ui32 getActiveParticleCount() const { return m_activeParticleCount; }
        bool isActive() const { return m_active; }

    private:
        void spawnParticle();
        Vector3 randomizeVector(const Vector3& base, const Vector3& variance);
        float randomFloat(float min, float max);

    private:
        EmitterConfig m_config;
        ParticleFactory m_particleFactory;
        std::vector<std::unique_ptr<Particle>> m_particles;
        ui32 m_activeParticleCount;
        float m_emissionAccumulator;
        bool m_active;
    };
}