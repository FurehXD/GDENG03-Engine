#pragma once
#include <DX3D/Core/Core.h>
#include <DX3D/Math/Vec2.h>

namespace dx3d
{
    class Particle
    {
    public:
        Particle();
        virtual ~Particle() = default;

        // Update particle state
        virtual void update(float deltaTime);

        // Check if particle should be removed
        virtual bool isDead() const { return m_life <= 0.0f || !m_active; }

        // Reset particle with new parameters
        virtual void reset(const Vec2& position, const Vec2& velocity);

        // Getters
        const Vec2& getPosition() const { return m_position; }
        const Vec2& getVelocity() const { return m_velocity; }
        const Vec4& getColor() const { return m_color; }
        float getSize() const { return m_size; }
        float getRotation() const { return m_rotation; }
        float getLife() const { return m_life; }
        float getLifeRatio() const { return m_maxLife > 0.0f ? m_life / m_maxLife : 0.0f; }
        bool isActive() const { return m_active; }

        // Setters for initialization
        void setLifetime(float maxLife) { m_maxLife = maxLife; m_life = maxLife; }
        void setSizeRange(float startSize, float endSize) { m_startSize = startSize; m_endSize = endSize; m_size = startSize; }
        void setColorRange(const Vec4& startColor, const Vec4& endColor) { m_startColor = startColor; m_endColor = endColor; m_color = startColor; }
        void setRotationSpeed(float speed) { m_rotationSpeed = speed; }
        void setAcceleration(const Vec2& accel) { m_acceleration = accel; }
        void setFadeSpeed(float speed) { m_fadeSpeed = speed; }

    protected:
        // Position and movement
        Vec2 m_position{ 0.0f, 0.0f };
        Vec2 m_velocity{ 0.0f, 0.0f };
        Vec2 m_acceleration{ 0.0f, 0.0f };

        // Appearance
        Vec4 m_color{ 1.0f, 1.0f, 1.0f, 1.0f };
        Vec4 m_startColor{ 1.0f, 1.0f, 1.0f, 1.0f };
        Vec4 m_endColor{ 1.0f, 1.0f, 1.0f, 0.0f };
        float m_size{ 0.05f };
        float m_startSize{ 0.05f };
        float m_endSize{ 0.01f };
        float m_rotation{ 0.0f };
        float m_rotationSpeed{ 0.0f };

        // Lifetime
        float m_life{ 1.0f };
        float m_maxLife{ 1.0f };
        float m_fadeSpeed{ 1.0f };

        // State
        bool m_active{ false };
    };
}