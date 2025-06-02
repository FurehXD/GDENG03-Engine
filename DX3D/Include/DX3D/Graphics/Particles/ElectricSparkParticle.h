#pragma once
#include <DX3D/Graphics/Particles/Particles.h>
#include <DX3D/Graphics/Particles/ParticleEmitter.h>

namespace dx3d
{
    class ElectricSparkParticle : public Particle
    {
    public:
        ElectricSparkParticle();

        void update(float deltaTime) override;
        void reset(const Vec2& position, const Vec2& velocity) override;

        // Initialize electric-specific properties
        void setFlickerRate(float rate) { m_flickerRate = rate; }

    private:
        float m_flickerTimer{ 0.0f };
        float m_flickerRate{ 0.02f };
        float m_brightness{ 1.0f };
        Vec2 m_jitter{ 0.0f, 0.0f };
    };

    class ElectricSparkEmitter : public ParticleEmitter
    {
    public:
        ElectricSparkEmitter(size_t maxParticles = 100);

        void update(float deltaTime) override;

        // Create a burst of electric sparks
        void spark(const Vec2& position, float intensity = 1.0f);

        // Electric spark settings
        void setSparkRadius(float radius) { m_sparkRadius = radius; }
        void setSparkIntensity(float intensity) { m_sparkIntensity = intensity; }
        void setContinuous(bool continuous) { m_continuous = continuous; }

    protected:
        std::unique_ptr<Particle> createParticle() override;
        void initializeParticle(Particle* particle, const Vec2& emitPosition) override;

    private:
        float m_sparkRadius{ 0.2f };
        float m_sparkIntensity{ 1.0f };
        bool m_continuous{ false };
        float m_continuousTimer{ 0.0f };
        float m_continuousRate{ 0.1f }; // Spark every 0.1 seconds if continuous
    };
}