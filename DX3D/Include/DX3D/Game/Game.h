#pragma once
#include <DX3D/Core/Base.h>
#include <DX3D/Core/Core.h>
#include <DX3D/Math/Vec2.h>
#include <vector>
#include <chrono>
#include <random> 
#include <DX3D/Input/InputSystem.h>

// Forward declarations
namespace dx3d
{
    class VertexBuffer;
    class VertexShader;
    class PixelShader;
    class ParticleSystem;
    class FireEmitter;
    class ShootingStarEmitter;
    class ElectricSparkEmitter;
    class Circle;
}

namespace dx3d
{
    // A simple structure to hold the state of each bouncing circle
    struct BouncingCircle
    {
        Vec2 position;
        Vec2 velocity;
    };

    class Game : public Base
    {
    public:
        explicit Game(const GameDesc& desc);
        virtual ~Game() override;

        virtual void run() final;
    private:
        void handleInput();
        void render();
        void createRenderingResources();
        void updateAnimation();
        void updateRectangleVertices();
        void updateRectangleVertices(float skewAmount);
        float lerp(float a, float b, float t);
        float smoothstep(float t);

        // Particle system methods
        void initializeParticles();
        void updateParticles(float deltaTime);

        // Update the function to handle multiple circles
        void updateCircles(float deltaTime);
        void spawnCircle();
        void removeLastCircle();
        void removeAllCircles();

    private:
        std::unique_ptr<Logger> m_loggerPtr{};
        std::unique_ptr<GraphicsEngine> m_graphicsEngine{};
        std::unique_ptr<Display> m_display{};
        bool m_isRunning{ true };

        std::vector<std::shared_ptr<VertexBuffer>> m_rectangles{};

        // --- Replace single circle members with collections ---
        int m_numCircles = 15; // Set the number of circles to generate
        std::vector<BouncingCircle> m_bouncingCircles;
        std::vector<std::shared_ptr<VertexBuffer>> m_circleVBs;
        float m_circleRadius = 0.08f;
        float m_aspectRatio = 1.0f;
        // ---------------------------------------------------

        // Shaders
        std::shared_ptr<VertexShader> m_transitionVertexShader{};
        std::shared_ptr<PixelShader> m_transitionPixelShader{};

        // Animation variables
        std::chrono::steady_clock::time_point m_startTime;
        float m_animationTime{ 0.0f };

        // Rectangle shape parameters for animation
        float m_currentWidth{ 0.6f };
        float m_currentHeight{ 0.8f };
        float m_currentX{ 0.0f };
        float m_currentY{ 0.0f };

        // Particle system
        std::unique_ptr<ParticleSystem> m_particleSystem;

        // Particle emitters
        std::shared_ptr<FireEmitter> m_fireEmitter;
        std::shared_ptr<ShootingStarEmitter> m_shootingStarEmitter;
        std::shared_ptr<ElectricSparkEmitter> m_electricSparkEmitter;

        // Demo timer
        float m_demoTimer{ 0.0f };
    };
}