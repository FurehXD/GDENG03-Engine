#pragma once
#include <DX3D/Graphics/Particles/Particles.h>
#include <DX3D/Graphics/Particles/ParticleEmitter.h>

namespace dx3d
{
    class StarTrailParticle : public Particle
    {
    public:
        StarTrailParticle();

        void update(float deltaTime) override;
        void reset(const Vec2& position, const Vec2& velocity) override;

        void setTrailIndex(int index) { m_trailIndex = index; }
        int getTrailIndex() const { return m_trailIndex; }

    private:
        int m_trailIndex{ 0 };
        float m_sparkle{ 0.0f };
    };

    class ShootingStarEmitter : public ParticleEmitter
    {
    public:
        ShootingStarEmitter(size_t maxParticles = 50);

        void update(float deltaTime) override;

        // Launch a new shooting star
        void launchStar(const Vec2& startPos, const Vec2& direction, float speed = 2.0f);

        // Star settings
        void setTrailLength(int length) { m_trailLength = std::max(1, length); }
        void setStarSize(float size) { m_starSize = size; }

    protected:
        std::unique_ptr<Particle> createParticle() override;
        void initializeParticle(Particle* particle, const Vec2& emitPosition) override;

    private:
        void emitTrail();

    private:
        Vec2 m_starPosition{ 0.0f, 0.0f };
        Vec2 m_starVelocity{ 0.0f, 0.0f };
        bool m_starActive{ false };
        float m_starLife{ 0.0f };
        float m_starMaxLife{ 2.0f };

        int m_trailLength{ 20 };
        float m_starSize{ 0.08f };
        float m_trailTimer{ 0.0f };
        float m_trailEmissionRate{ 0.005f };
        int m_currentTrailIndex{ 0 };
    };
}