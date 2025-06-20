#include <DX3D/Game/Game.h>
#include <DX3D/Window/Window.h>
#include <DX3D/Graphics/GraphicsEngine.h>
#include <DX3D/Core/Logger.h>
#include <DX3D/Game/Display.h>
#include <DX3D/Graphics/RenderSystem.h>
#include <DX3D/Graphics/SwapChain.h>
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/Primitives/Rectangle.h>
#include <DX3D/Graphics/Primitives/Circle.h>
#include <DX3D/Graphics/Shaders/TransitionShader.h>
#include <d3d11.h>
#include <thread>

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

    m_startTime = std::chrono::steady_clock::now();

    // --- Initialize multiple circles ---
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> posDist(-0.8f, 0.8f);
    std::uniform_real_distribution<float> velDist(0.8f, 0.9f);
    std::uniform_int_distribution<int> signDist(0, 1);

    m_bouncingCircles.reserve(m_numCircles);
    for (int i = 0; i < m_numCircles; ++i)
    {
        BouncingCircle circle;
        circle.position = Vec2(posDist(rng), posDist(rng));
        circle.velocity = Vec2(velDist(rng) * (signDist(rng) ? 1 : -1), velDist(rng) * (signDist(rng) ? 1 : -1));
        m_bouncingCircles.push_back(circle);
    }

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

    const auto& winSize = m_display->getSize();
    m_aspectRatio = static_cast<float>(winSize.width) / static_cast<float>(winSize.height);

    m_transitionVertexShader = std::make_shared<VertexShader>(resourceDesc, TransitionShader::GetVertexShaderCode());
    m_transitionPixelShader = std::make_shared<PixelShader>(resourceDesc, TransitionShader::GetPixelShaderCode());

    DX3DLogInfo("Rendering resources created successfully.");
}

void dx3d::Game::updateCircles(float deltaTime)
{
    auto& renderSystem = m_graphicsEngine->getRenderSystem();
    auto resourceDesc = renderSystem.getGraphicsResourceDesc();

    // Clear the vertex buffers from the previous frame
    m_circleVBs.clear();
    m_circleVBs.reserve(m_numCircles);

    const float radiusX = m_circleRadius / m_aspectRatio;
    const float radiusY = m_circleRadius;

    // Loop through each circle to update its state
    for (auto& circle : m_bouncingCircles)
    {
        circle.position += circle.velocity * deltaTime;

        // Boundary collision checks
        if (circle.position.x + radiusX > 1.0f)
        {
            circle.velocity.x *= -1;
            circle.position.x = 1.0f - radiusX;
        }
        else if (circle.position.x - radiusX < -1.0f)
        {
            circle.velocity.x *= -1;
            circle.position.x = -1.0f + radiusX;
        }

        if (circle.position.y + radiusY > 1.0f)
        {
            circle.velocity.y *= -1;
            circle.position.y = 1.0f - radiusY;
        }
        else if (circle.position.y - radiusY < -1.0f)
        {
            circle.velocity.y *= -1;
            circle.position.y = -1.0f + radiusY;
        }

        // Create a new vertex buffer for the circle at its new position
        m_circleVBs.push_back(Circle::CreateAt(resourceDesc, circle.position.x, circle.position.y, m_circleRadius, 32, m_aspectRatio));
    }
}

void dx3d::Game::spawnCircle()
{
    // Re-use the random generation from the constructor
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> posDist(-0.8f, 0.8f);
    std::uniform_real_distribution<float> velDist(0.2f, 0.5f);
    std::uniform_int_distribution<int> signDist(0, 1);

    BouncingCircle circle;
    circle.position = Vec2(posDist(rng), posDist(rng));
    circle.velocity = Vec2(velDist(rng) * (signDist(rng) ? 1 : -1), velDist(rng) * (signDist(rng) ? 1 : -1));
    m_bouncingCircles.push_back(circle);
}

void dx3d::Game::removeLastCircle()
{
    if (!m_bouncingCircles.empty())
    {
        m_bouncingCircles.pop_back();
    }
}

void dx3d::Game::removeAllCircles()
{
    m_bouncingCircles.clear();
}

void dx3d::Game::handleInput()
{
    // ESC: Close the application
    if (InputSystem::isKeyJustPressed(VK_ESCAPE))
    {
        m_isRunning = false;
    }

    // SPACEBAR: Spawn a new circle
    if (InputSystem::isKeyJustPressed(VK_SPACE))
    {
        spawnCircle();
        DX3DLogInfo("SPAWNED CIRCLE");
    }

    // BACKSPACE: Remove the last circle
    if (InputSystem::isKeyJustPressed(VK_BACK))
    {
        removeLastCircle();
        DX3DLogInfo("DELETED LAST CIRCLE");
    }

    // DELETE: Remove all circles
    if (InputSystem::isKeyJustPressed(VK_DELETE))
    {
        removeAllCircles();
        DX3DLogInfo("DELETED ALL CIRCLES");
    }
}

void dx3d::Game::initializeParticles()
{
    auto& renderSystem = m_graphicsEngine->getRenderSystem();
    auto resourceDesc = renderSystem.getGraphicsResourceDesc();

    // Create particle system
    m_particleSystem = std::make_unique<ParticleSystem>(resourceDesc);

    // Create fire emitter
    m_fireEmitter = std::make_shared<FireEmitter>(1000);
    m_fireEmitter->setPosition(Vec2(-0.6f, -0.7f));
    m_fireEmitter->setFlameHeight(0.5f);
    m_fireEmitter->setFlameWidth(0.25f);
    m_fireEmitter->setIntensity(1.0f);
    m_particleSystem->addEmitter(m_fireEmitter);

    // Create shooting star emitter
    m_shootingStarEmitter = std::make_shared<ShootingStarEmitter>(100);
    m_particleSystem->addEmitter(m_shootingStarEmitter);

    // Create electric spark emitter at right side
    m_electricSparkEmitter = std::make_shared<ElectricSparkEmitter>(150);
    m_electricSparkEmitter->setPosition(Vec2(0.6f, 0.0f));
    m_electricSparkEmitter->setSparkRadius(0.15f);
    m_electricSparkEmitter->setContinuous(true); 
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
    
}

void dx3d::Game::updateRectangleVertices()
{
    // This is the old method - we'll override it
    updateRectangleVertices(0.0f);
}

void dx3d::Game::updateRectangleVertices(float skewAmount)
{
    
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

    // Create occasional electric spark bursts
    static float sparkBurstTimer = 0.0f;
    sparkBurstTimer += deltaTime;
    if (sparkBurstTimer > 1.5f)
    {
        sparkBurstTimer = 0.0f;

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
    // --- Define our target framerate ---
    const float TARGET_FPS = 60.0f;
    const auto TARGET_FRAMETIME = std::chrono::duration<double, std::milli>(1000.0 / TARGET_FPS);
    // ---

    // Record the time at the start of the frame
    auto frameStartTime = std::chrono::steady_clock::now();


    // ---TIMING ---
    // Use a fixed delta time for all game logic to ensure consistent simulation speed
    const float deltaTime = 1.0f / TARGET_FPS;


    // --- INPUT ---
    handleInput();

    // --- UPDATE ---
    updateCircles(deltaTime);
    //updateParticles(deltaTime); // Particles are commented out as requested

    // --- RENDER ---
    auto& renderSystem = m_graphicsEngine->getRenderSystem();
    auto& deviceContext = renderSystem.getDeviceContext();
    auto& swapChain = m_display->getSwapChain();

    deviceContext.clearRenderTargetColor(swapChain, 0.0f, 0.0f, 0.0f, 1.0f);
    deviceContext.setRenderTargets(swapChain);
    deviceContext.setViewportSize(m_display->getSize().width, m_display->getSize().height);

    deviceContext.setVertexShader(m_transitionVertexShader->getShader());
    deviceContext.setPixelShader(m_transitionPixelShader->getShader());
    deviceContext.setInputLayout(m_transitionVertexShader->getInputLayout());

    for (const auto& circleVB : m_circleVBs)
    {
        if (circleVB)
        {
            deviceContext.setVertexBuffer(*circleVB);
            deviceContext.drawTriangleList(circleVB->getVertexCount(), 0);
        }
    }

    //m_particleSystem->render(deviceContext); // Particles are commented out as requested
    deviceContext.present(swapChain);

    InputSystem::update();

    // --- FRAME RATE CAPPING ---
    // Record the time at the end of the frame
    auto frameEndTime = std::chrono::steady_clock::now();
    // Calculate how long the frame actually took
    auto frameDuration = frameEndTime - frameStartTime;

    // If the frame finished faster than our target, sleep for the remaining time
    if (frameDuration < TARGET_FRAMETIME)
    {
        auto sleepDuration = TARGET_FRAMETIME - frameDuration;
        std::this_thread::sleep_for(sleepDuration);
    }
}