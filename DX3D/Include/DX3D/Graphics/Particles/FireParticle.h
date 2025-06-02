#pragma once
#include <DX3D/Graphics/Particles/Particles.h>
#include <DX3D/Graphics/Particles/ParticleEmitter.h>

namespace dx3d
{
    class FireParticle : public Particle
    {
    public:
        FireParticle();

        void update(float deltaTime) override;
        void reset(const Vec2& position, const Vec2& velocity) override;

        // Initialize with fire-specific parameters
        void initializeFireProperties(float turbulence, float riseSpeed);

    private:
        float m_turbulence{ 0.0f };
        float m_riseSpeed{ 0.0f };
    };

    class FireEmitter : public ParticleEmitter
    {
    public:
        FireEmitter(size_t maxParticles = 150);

        void update(float deltaTime) override;

        // Fire-specific settings
        void setFlameHeight(float height) { m_flameHeight = height; }
        void setFlameWidth(float width) { m_flameWidth = width; }
        void setIntensity(float intensity) { m_intensity = intensity; }

    protected:
        std::unique_ptr<Particle> createParticle() override;
        void initializeParticle(Particle* particle, const Vec2& emitPosition) override;

    private:
        float m_flameHeight{ 0.5f };
        float m_flameWidth{ 0.2f };
        float m_intensity{ 1.0f };
        float m_emissionTimer{ 0.0f };
        float m_emissionRate{ 0.01f }; // Emit every 0.01 seconds
    };
}