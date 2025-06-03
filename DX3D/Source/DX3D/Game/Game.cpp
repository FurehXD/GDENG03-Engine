#include <DX3D/Game/Game.h>
#include <DX3D/Window/Window.h>
#include <DX3D/Graphics/GraphicsEngine.h>
#include <DX3D/Core/Logger.h>
#include <DX3D/Game/Display.h>
#include <DX3D/Graphics/RenderSystem.h>
#include <DX3D/Graphics/SwapChain.h>
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/Primitives/Rectangle.h>
#include <DX3D/Graphics/Shaders/TransitionShader.h>

// Particle system includes
#include <DX3D/Graphics/Particles/ParticleSystem.h>
#include <DX3D/Graphics/Particles/FireParticle.h>
#include <DX3D/Graphics/Particles/ShootingStarParticle.h>
#include <DX3D/Graphics/Particles/ElectricSparkParticle.h>

#include <cmath>
#include <random>

dx3d::Game::Game(const GameDesc& desc) :
    Base({ *std::make_unique<Logger>(desc.logLevel).release() }),
    m_loggerPtr(&m_logger)
{
    m_graphicsEngine = std::make_unique<GraphicsEngine>(GraphicsEngineDesc{ m_logger });
    m_display = std::make_unique<Display>(DisplayDesc{ {m_logger,desc.windowSize},m_graphicsEngine->getRenderSystem() });

    // Initialize animation timer
    m_startTime = std::chrono::steady_clock::now();

    createRenderingResources();
    initializeParticles();

    DX3DLogInfo("Game initialized with particle systems.");
}

dx3d::Game::~Game()
{
    DX3DLogInfo("Game deallocation started.");
}

void dx3d::Game::createRenderingResources()
{
    auto& renderSystem = m_graphicsEngine->getRenderSystem();
    auto resourceDesc = renderSystem.getGraphicsResourceDesc();

    m_rectangles.clear();

    // Create initial rectangle (commented out for now)
    // m_rectangles.push_back(Rectangle::CreateAt(resourceDesc, 0.0f, 0.0f, 0.6f, 0.8f));

    // Create transition shader that handles the color blending internally
    m_transitionVertexShader = std::make_shared<VertexShader>(resourceDesc, TransitionShader::GetVertexShaderCode());
    m_transitionPixelShader = std::make_shared<PixelShader>(resourceDesc, TransitionShader::GetPixelShaderCode());

    DX3DLogInfo("Rendering resources created successfully.");
}

void dx3d::Game::initializeParticles()
{
    auto& renderSystem = m_graphicsEngine->getRenderSystem();
    auto resourceDesc = renderSystem.getGraphicsResourceDesc();

    // Create particle system
    m_particleSystem = std::make_unique<ParticleSystem>(resourceDesc);

    // Create fire emitter at left side
    m_fireEmitter = std::make_shared<FireEmitter>(200);
    m_fireEmitter->setPosition(Vec2(-0.6f, -0.7f));
    m_fireEmitter->setFlameHeight(0.5f);
    m_fireEmitter->setFlameWidth(0.25f);
    m_fireEmitter->setIntensity(1.0f);
    m_particleSystem->addEmitter(m_fireEmitter);

    // Create shooting star emitter (will launch from top)
    m_shootingStarEmitter = std::make_shared<ShootingStarEmitter>(100);
    m_particleSystem->addEmitter(m_shootingStarEmitter);

    // Create electric spark emitter at right side
    m_electricSparkEmitter = std::make_shared<ElectricSparkEmitter>(150);
    m_electricSparkEmitter->setPosition(Vec2(0.6f, 0.0f));
    m_electricSparkEmitter->setSparkRadius(0.15f);
    m_electricSparkEmitter->setContinuous(true);  // Continuous sparking
    m_electricSparkEmitter->setSparkIntensity(0.8f);
    m_particleSystem->addEmitter(m_electricSparkEmitter);

    DX3DLogInfo("Particle systems initialized.");
}

float dx3d::Game::lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

float dx3d::Game::smoothstep(float t)
{
    // Smooth cubic interpolation for natural easing
    return t * t * (3.0f - 2.0f * t);
}

void dx3d::Game::updateAnimation()
{
    // Animation code commented out for now
    /*
    auto currentTime = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_startTime);
    m_animationTime = elapsed.count() / 1000.0f;

    // 8-second cycle: 4 seconds to transform, 4 seconds to transform back (slower)
    float cycleDuration = 8.0f;
    float cycleTime = fmod(m_animationTime, cycleDuration) / cycleDuration;

    // Create smooth back-and-forth motion
    float animPhase;
    if (cycleTime < 0.5f) {
        // First half: rectangle to parallelogram
        animPhase = cycleTime * 2.0f;
    }
    else {
        // Second half: parallelogram back to rectangle
        animPhase = 2.0f - (cycleTime * 2.0f);
    }

    // Apply smooth easing
    float smoothPhase = smoothstep(animPhase);

    // Calculate skew amount (0.0 = rectangle, 1.0 = parallelogram)
    float skewAmount = smoothPhase;

    // Update shape parameters with rightward movement
    m_currentX = lerp(-0.3f, 0.3f, smoothPhase); // Move from left to right
    m_currentY = 0.0f;
    m_currentWidth = 0.6f;
    m_currentHeight = 0.8f;

    updateRectangleVertices(skewAmount);
    */
}

void dx3d::Game::updateRectangleVertices()
{
    // This is the old method - we'll override it
    updateRectangleVertices(0.0f);
}

void dx3d::Game::updateRectangleVertices(float skewAmount)
{
    // Rectangle update code commented out for now
    /*
    auto& renderSystem = m_graphicsEngine->getRenderSystem();
    auto resourceDesc = renderSystem.getGraphicsResourceDesc();

    // Calculate vertices manually with skew applied
    float halfWidth = m_currentWidth * 0.5f;
    float halfHeight = m_currentHeight * 0.5f;

    // Skew offset - applied to top vertices to create parallelogram
    float skewOffset = skewAmount * 0.3f; // Maximum skew

    // Create vertices for triangle strip (same order as Rectangle class)
    Vertex vertices[] = {
        // Top-left (with skew)
        { {m_currentX - halfWidth + skewOffset, m_currentY + halfHeight, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f} },
        // Top-right (with skew)
        { {m_currentX + halfWidth + skewOffset, m_currentY + halfHeight, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f} },
        // Bottom-left (no skew)
        { {m_currentX - halfWidth, m_currentY - halfHeight, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f} },
        // Bottom-right (no skew)
        { {m_currentX + halfWidth, m_currentY - halfHeight, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f} }
    };

    // Recreate the vertex buffer with new vertices
    m_rectangles.clear();
    m_rectangles.push_back(
        std::make_shared<VertexBuffer>(
            vertices,
            sizeof(Vertex),
            4,
            resourceDesc
        )
    );
    */
}

void dx3d::Game::updateParticles(float deltaTime)
{
    // Update demo timer
    m_demoTimer += deltaTime;

    // Launch shooting stars periodically
    static float starTimer = 0.0f;
    starTimer += deltaTime;
    if (starTimer > 3.0f)  // Every 3 seconds
    {
        starTimer = 0.0f;

        // Random number generation
        static std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

        // Random start position at top of screen
        float startX = dist(gen) * 0.8f;
        Vec2 startPos(startX, 0.9f);

        // Aim downward with some angle
        float angleOffset = dist(gen) * 0.3f;
        Vec2 direction(angleOffset, -1.0f);

        m_shootingStarEmitter->launchStar(startPos, direction, 1.5f);

        DX3DLogInfo("Launched shooting star");
    }

    // Create occasional electric spark bursts (in addition to continuous)
    static float sparkBurstTimer = 0.0f;
    sparkBurstTimer += deltaTime;
    if (sparkBurstTimer > 1.5f)
    {
        sparkBurstTimer = 0.0f;

        // Random number generation
        static std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<float> distX(-0.3f, 0.3f);
        std::uniform_real_distribution<float> distY(-0.4f, 0.4f);

        // Big spark burst at center area
        float x = distX(gen);
        float y = distY(gen);
        m_electricSparkEmitter->spark(Vec2(x, y), 2.0f);  // High intensity burst
    }

    // Update particle system
    m_particleSystem->update(deltaTime);

    // Debug: Log particle counts
    static float debugTimer = 0.0f;
    debugTimer += deltaTime;
    if (debugTimer > 1.0f) // Every second
    {
        debugTimer = 0.0f;
        int fireCount = 0, starCount = 0, sparkCount = 0;

        if (m_fireEmitter)
            fireCount = static_cast<int>(m_fireEmitter->getParticles().size());
        if (m_shootingStarEmitter)
            starCount = static_cast<int>(m_shootingStarEmitter->getParticles().size());
        if (m_electricSparkEmitter)
            sparkCount = static_cast<int>(m_electricSparkEmitter->getParticles().size());

        char debugMsg[256];
        sprintf_s(debugMsg, "Active particles - Fire: %d, Stars: %d, Sparks: %d",
            fireCount, starCount, sparkCount);
        DX3DLogInfo(debugMsg);
    }
}

void dx3d::Game::render()
{
    // Calculate delta time
    auto currentTime = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_startTime);
    static auto lastTime = elapsed.count();
    auto currentTimeMs = elapsed.count();
    float deltaTime = (currentTimeMs - lastTime) / 1000.0f;
    lastTime = currentTimeMs;

    // Clamp delta time to prevent large jumps
    deltaTime = std::min(deltaTime, 0.033f); // Cap at ~30 FPS minimum

    // Update animation before rendering (commented out)
    // updateAnimation();

    auto& renderSystem = m_graphicsEngine->getRenderSystem();
    auto& deviceContext = renderSystem.getDeviceContext();
    auto& swapChain = m_display->getSwapChain();

    // Clear screen to black
    deviceContext.clearRenderTargetColor(swapChain, 0.0f, 0.0f, 1.0f, 1.0f);
    deviceContext.setRenderTargets(swapChain);
    deviceContext.setViewportSize(m_display->getSize().width, m_display->getSize().height);

    // Render the morphing rectangle with smooth color transition (commented out)
    /*
    if (!m_rectangles.empty())
    {
        deviceContext.setVertexBuffer(*m_rectangles[0]);

        // Use the transition shader that handles color blending internally
        deviceContext.setVertexShader(m_transitionVertexShader->getShader());
        deviceContext.setPixelShader(m_transitionPixelShader->getShader());
        deviceContext.setInputLayout(m_transitionVertexShader->getInputLayout());

        deviceContext.drawTriangleStrip(m_rectangles[0]->getVertexCount(), 0);
    }
    */

    // Update and render particles
    updateParticles(deltaTime);
    m_particleSystem->render(deviceContext);

    deviceContext.present(swapChain);
}