#include <DX3D/Game/Game.h>
#include <DX3D/Window/Window.h>
#include <DX3D/Graphics/GraphicsEngine.h>
#include <DX3D/Core/Logger.h>
#include <DX3D/Game/Display.h>
#include <DX3D/Game/Camera.h>
#include <DX3D/Input/Input.h>
#include <DX3D/Graphics/RenderSystem.h>
#include <DX3D/Graphics/SwapChain.h>
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/VertexBuffer.h>
#include <DX3D/Graphics/IndexBuffer.h>
#include <DX3D/Graphics/ConstantBuffer.h>
#include <DX3D/Graphics/DepthBuffer.h>
#include <DX3D/Graphics/Primitives/AGameObject.h>
#include <DX3D/Graphics/Primitives/Cube.h>
#include <DX3D/Graphics/Primitives/Plane.h>
#include <DX3D/Graphics/Primitives/Sphere.h>
#include <DX3D/Graphics/Primitives/Cylinder.h>
#include <DX3D/Graphics/Primitives/Capsule.h>
#include <DX3D/Graphics/Shaders/Rainbow3DShader.h>
#include <DX3D/Graphics/Shaders/WhiteShader.h>
#include <DX3D/Math/Math.h>
#include <DX3D/Particles/ParticleSystem.h>
#include <DX3D/Particles/ParticleEffects/SnowParticle.h>
#include <cmath>
#include <random>
#include <string>
#include <cstdio>
#include <DirectXMath.h>

dx3d::Game::Game(const GameDesc& desc) :
    Base({ *std::make_unique<Logger>(desc.logLevel).release() }),
    m_loggerPtr(&m_logger)
{
    m_graphicsEngine = std::make_unique<GraphicsEngine>(GraphicsEngineDesc{ m_logger });
    m_display = std::make_unique<Display>(DisplayDesc{ {m_logger,desc.windowSize},m_graphicsEngine->getRenderSystem() });

    m_previousTime = std::chrono::steady_clock::now();

    createRenderingResources();

    DX3DLogInfo("Game initialized with Camera, Input system, and Particle system.");
}

dx3d::Game::~Game()
{
    DX3DLogInfo("Game deallocation started.");

    // Shutdown particle system
    ParticleSystem::getInstance().shutdown();

    // Release depth states
    if (m_particleDepthState) m_particleDepthState->Release();
    if (m_solidDepthState) m_solidDepthState->Release();
}

void dx3d::Game::createRenderingResources()
{
    auto& renderSystem = m_graphicsEngine->getRenderSystem();
    auto resourceDesc = renderSystem.getGraphicsResourceDesc();
    auto& deviceContext = renderSystem.getDeviceContext();
    auto d3dContext = deviceContext.getDeviceContext();
    ID3D11Device* device = nullptr;
    d3dContext->GetDevice(&device);
    


    // Create vertex and index buffers for all primitives
    m_cubeVertexBuffer = Cube::CreateVertexBuffer(resourceDesc);
    m_cubeIndexBuffer = Cube::CreateIndexBuffer(resourceDesc);
    m_planeVertexBuffer = Plane::CreateVertexBuffer(resourceDesc);
    m_planeIndexBuffer = Plane::CreateIndexBuffer(resourceDesc);
    m_sphereVertexBuffer = Sphere::CreateVertexBuffer(resourceDesc);
    m_sphereIndexBuffer = Sphere::CreateIndexBuffer(resourceDesc);
    m_cylinderVertexBuffer = Cylinder::CreateVertexBuffer(resourceDesc);
    m_cylinderIndexBuffer = Cylinder::CreateIndexBuffer(resourceDesc);
    m_capsuleVertexBuffer = Capsule::CreateVertexBuffer(resourceDesc);
    m_capsuleIndexBuffer = Capsule::CreateIndexBuffer(resourceDesc);

    // CREATE DEPTH BUFFER
    const auto& windowSize = m_display->getSize();
    m_depthBuffer = std::make_shared<DepthBuffer>(
        windowSize.width,
        windowSize.height,
        resourceDesc
    );

    // --- Create Depth Stencil States ---
    // Default state for solid objects (depth test and write on)
    D3D11_DEPTH_STENCIL_DESC solidDesc = {};
    solidDesc.DepthEnable = TRUE;
    solidDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    solidDesc.DepthFunc = D3D11_COMPARISON_LESS;
    solidDesc.StencilEnable = FALSE;
    device->CreateDepthStencilState(&solidDesc, &m_solidDepthState);

    // State for transparent particles (depth test on, depth write off)
    D3D11_DEPTH_STENCIL_DESC particleDesc = {};
    particleDesc.DepthEnable = TRUE;
    particleDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // Main difference here!
    particleDesc.DepthFunc = D3D11_COMPARISON_LESS;
    particleDesc.StencilEnable = FALSE;
    device->CreateDepthStencilState(&particleDesc, &m_particleDepthState);

    device->Release();

    m_rainbowVertexShader = std::make_shared<VertexShader>(resourceDesc, Rainbow3DShader::GetVertexShaderCode());
    m_rainbowPixelShader = std::make_shared<PixelShader>(resourceDesc, Rainbow3DShader::GetPixelShaderCode());
    m_whiteVertexShader = std::make_shared<VertexShader>(resourceDesc, WhiteShader::GetVertexShaderCode());
    m_whitePixelShader = std::make_shared<PixelShader>(resourceDesc, WhiteShader::GetPixelShaderCode());

    m_transformConstantBuffer = std::make_shared<ConstantBuffer>(sizeof(TransformationMatrices), resourceDesc);

    // Create game objects - arrange them on the plane
    m_gameObjects.reserve(6); // Increased for new primitives
    m_objectRotationDeltas.reserve(6);

    // Cube - center
    m_gameObjects.push_back(std::make_shared<Cube>(
        Vector3(0.0f, 1.0f, 0.0f),
        Vector3(0.0f, 0.0f, 0.0f),
        Vector3(1.5f, 1.5f, 1.5f)
    ));
    m_objectRotationDeltas.push_back(Vector3(0.0f, 0.8f, 0.0f));

    // Sphere - left
    m_gameObjects.push_back(std::make_shared<Sphere>(
        Vector3(-3.0f, 1.0f, 0.0f),
        Vector3(0.0f, 0.0f, 0.0f),
        Vector3(1.5f, 1.5f, 1.5f)
    ));
    m_objectRotationDeltas.push_back(Vector3(0.3f, 0.5f, 0.0f));

    // Cylinder - right
    m_gameObjects.push_back(std::make_shared<Cylinder>(
        Vector3(3.0f, 1.0f, 0.0f),
        Vector3(0.0f, 0.0f, 0.0f),
        Vector3(1.5f, 1.5f, 1.5f)
    ));
    m_objectRotationDeltas.push_back(Vector3(-0.2f, 0.6f, 0.0f));

    // Capsule - front
    m_gameObjects.push_back(std::make_shared<Capsule>(
        Vector3(0.0f, 1.0f, 3.0f),
        Vector3(0.0f, 0.0f, 0.0f),
        Vector3(10.5f, 10.5f, 1.5f)
    ));
    m_objectRotationDeltas.push_back(Vector3(0.4f, 0.3f, 0.2f));

    // Second row of objects
    // Another Sphere - back left
    m_gameObjects.push_back(std::make_shared<Sphere>(
        Vector3(-3.0f, 1.0f, -3.0f),
        Vector3(0.0f, 0.0f, 0.0f),
        Vector3(1.2f, 1.2f, 1.2f)
    ));
    m_objectRotationDeltas.push_back(Vector3(0.0f, -0.7f, 0.3f));

    // Plane - expanded to accommodate all objects
    m_gameObjects.push_back(std::make_shared<Plane>(
        Vector3(0.0f, 0.0f, 0.0f),      // Center at origin
        Vector3(-1.5708f, 0.0f, 0.0f),  // Horizontal
        Vector3(10.0f, 10.0f, 1.0f)     // Large plane
    ));
    m_objectRotationDeltas.push_back(Vector3(0.0f, 0.0f, 0.0f)); // Static plane

    // Create camera
    m_camera = std::make_unique<Camera>(
        Vector3(8.0f, 6.0f, -8.0f),     // Initial position
        Vector3(0.0f, 1.0f, 0.0f)       // Look at slightly above origin
    );

    // Setup projection matrix
    float aspectRatio = static_cast<float>(windowSize.width) / static_cast<float>(windowSize.height);
    m_projectionMatrix = Matrix4x4::CreatePerspectiveFovLH(
        1.0472f,        // 60 degrees
        aspectRatio,
        0.1f,
        100.0f
    );

    // Initialize particle system
    ParticleSystem::getInstance().initialize(*m_graphicsEngine);

    // Create a snow emitter
    ParticleEmitter::EmitterConfig snowConfig;
    snowConfig.position = Vector3(0.0f, 10.0f, 0.0f); // Start snowing from above
    snowConfig.positionVariance = Vector3(20.0f, 0.0f, 20.0f); // Spread out over an area
    snowConfig.velocity = Vector3(0.0f, -2.0f, 0.0f);
    snowConfig.velocityVariance = Vector3(0.5f, 0.5f, 0.5f);
    snowConfig.acceleration = Vector3(0.0f, -0.5f, 0.0f);
    snowConfig.startColor = Vector4(1.0f, 1.0f, 1.0f, 0.8f);
    snowConfig.endColor = Vector4(0.9f, 0.9f, 1.0f, 0.0f);
    snowConfig.startSize = 0.2f;
    snowConfig.endSize = 0.1f;
    snowConfig.lifetime = 8.0f;
    snowConfig.lifetimeVariance = 2.0f;
    snowConfig.emissionRate = 50.0f;
    snowConfig.maxParticles = 2000;

    auto snowEmitter = ParticleSystem::getInstance().createEmitter(
        "snow",
        snowConfig,
        createSnowParticle
    );

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    HWND hwnd = m_display->getWindowHandle();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, d3dContext);

    DX3DLogInfo("All primitives created: Cube, Sphere, Cylinder, Capsule, and Plane.");
    DX3DLogInfo("Camera created. Hold right mouse button + WASD to move camera.");
}

void dx3d::Game::processInput(float deltaTime)
{
    auto& input = Input::getInstance();

    if (input.isMouseButtonPressed(MouseButton::Right))
    {
        float moveSpeed = m_cameraSpeed * deltaTime;
        if (input.isKeyPressed(KeyCode::W)) m_camera->moveForward(moveSpeed);
        if (input.isKeyPressed(KeyCode::S)) m_camera->moveBackward(moveSpeed);
        if (input.isKeyPressed(KeyCode::A)) m_camera->moveLeft(moveSpeed);
        if (input.isKeyPressed(KeyCode::D)) m_camera->moveRight(moveSpeed);
        if (input.isKeyPressed(KeyCode::Q)) m_camera->moveDown(moveSpeed);
        if (input.isKeyPressed(KeyCode::E)) m_camera->moveUp(moveSpeed);

        float mouseDeltaX = static_cast<float>(input.getMouseDeltaX());
        float mouseDeltaY = static_cast<float>(input.getMouseDeltaY());

        if (mouseDeltaX != 0.0f || mouseDeltaY != 0.0f)
        {
            m_camera->onMouseMove(mouseDeltaX, mouseDeltaY, m_mouseSensitivity * 0.01f);
        }
    }

    if (input.isKeyJustPressed(KeyCode::R))
    {
        m_camera->setPosition(Vector3(8.0f, 6.0f, -8.0f));
        m_camera->lookAt(Vector3(0.0f, 1.0f, 0.0f));
        DX3DLogInfo("Camera reset to initial position");
    }

    if (input.isKeyPressed(KeyCode::Escape))
    {
        m_isRunning = false;
    }
}

void dx3d::Game::update()
{
    auto currentTime = std::chrono::steady_clock::now();
    m_deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - m_previousTime).count() / 1000000.0f;
    m_previousTime = currentTime;

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();

    processInput(m_deltaTime);
    m_camera->update();

    // Update all objects except the plane (last object)
    for (size_t i = 0; i < m_gameObjects.size() - 1; ++i)
    {
        m_gameObjects[i]->rotate(m_objectRotationDeltas[i] * m_deltaTime);
        m_gameObjects[i]->update(m_deltaTime);
    }

    ParticleSystem::getInstance().update(m_deltaTime);

    // Update emitter to follow above the camera for a continuous snow effect
    if (auto snowEmitter = ParticleSystem::getInstance().getEmitter("snow"))
    {
        Vector3 emitterPos = m_camera->getPosition();
        emitterPos.y += 10.0f; // Keep the emitter 10 units above the camera
        snowEmitter->setPosition(emitterPos);
    }
}

void dx3d::Game::render()
{
    auto& renderSystem = m_graphicsEngine->getRenderSystem();
    auto& deviceContext = renderSystem.getDeviceContext();
    auto& swapChain = m_display->getSwapChain();
    auto d3dContext = deviceContext.getDeviceContext();

    deviceContext.clearRenderTargetColor(swapChain, 0.1f, 0.1f, 0.2f, 1.0f);
    deviceContext.clearDepthBuffer(*m_depthBuffer);
    deviceContext.setRenderTargetsWithDepth(swapChain, *m_depthBuffer);
    deviceContext.setViewportSize(m_display->getSize().width, m_display->getSize().height);

    ID3D11Buffer* cb = m_transformConstantBuffer->getBuffer();
    d3dContext->VSSetConstantBuffers(0, 1, &cb);

    // --- RENDER SOLID OBJECTS ---
    d3dContext->OMSetDepthStencilState(m_solidDepthState, 0);
    for (const auto& gameObject : m_gameObjects)
    {
        // Determine which buffers and shaders to use based on object type
        if (auto cube = std::dynamic_pointer_cast<Cube>(gameObject))
        {
            deviceContext.setVertexBuffer(*m_cubeVertexBuffer);
            deviceContext.setIndexBuffer(*m_cubeIndexBuffer);
            deviceContext.setVertexShader(m_rainbowVertexShader->getShader());
            deviceContext.setPixelShader(m_rainbowPixelShader->getShader());
            deviceContext.setInputLayout(m_rainbowVertexShader->getInputLayout());
        }
        else if (auto sphere = std::dynamic_pointer_cast<Sphere>(gameObject))
        {
            deviceContext.setVertexBuffer(*m_sphereVertexBuffer);
            deviceContext.setIndexBuffer(*m_sphereIndexBuffer);
            deviceContext.setVertexShader(m_rainbowVertexShader->getShader());
            deviceContext.setPixelShader(m_rainbowPixelShader->getShader());
            deviceContext.setInputLayout(m_rainbowVertexShader->getInputLayout());
        }
        else if (auto cylinder = std::dynamic_pointer_cast<Cylinder>(gameObject))
        {
            deviceContext.setVertexBuffer(*m_cylinderVertexBuffer);
            deviceContext.setIndexBuffer(*m_cylinderIndexBuffer);
            deviceContext.setVertexShader(m_rainbowVertexShader->getShader());
            deviceContext.setPixelShader(m_rainbowPixelShader->getShader());
            deviceContext.setInputLayout(m_rainbowVertexShader->getInputLayout());
        }
        else if (auto capsule = std::dynamic_pointer_cast<Capsule>(gameObject))
        {
            deviceContext.setVertexBuffer(*m_capsuleVertexBuffer);
            deviceContext.setIndexBuffer(*m_capsuleIndexBuffer);
            deviceContext.setVertexShader(m_rainbowVertexShader->getShader());
            deviceContext.setPixelShader(m_rainbowPixelShader->getShader());
            deviceContext.setInputLayout(m_rainbowVertexShader->getInputLayout());
        }
        else if (auto plane = std::dynamic_pointer_cast<Plane>(gameObject))
        {
            deviceContext.setVertexBuffer(*m_planeVertexBuffer);
            deviceContext.setIndexBuffer(*m_planeIndexBuffer);
            deviceContext.setVertexShader(m_whiteVertexShader->getShader());
            deviceContext.setPixelShader(m_whitePixelShader->getShader());
            deviceContext.setInputLayout(m_whiteVertexShader->getInputLayout());
        }

        TransformationMatrices transformMatrices;
        transformMatrices.world = Matrix4x4::fromXMMatrix(DirectX::XMMatrixTranspose(gameObject->getWorldMatrix().toXMMatrix()));
        transformMatrices.view = Matrix4x4::fromXMMatrix(DirectX::XMMatrixTranspose(m_camera->getViewMatrix().toXMMatrix()));
        transformMatrices.projection = Matrix4x4::fromXMMatrix(DirectX::XMMatrixTranspose(m_projectionMatrix.toXMMatrix()));
        m_transformConstantBuffer->update(deviceContext, &transformMatrices);

        // Draw with appropriate index count
        if (std::dynamic_pointer_cast<Cube>(gameObject))
            deviceContext.drawIndexed(Cube::GetIndexCount(), 0, 0);
        else if (std::dynamic_pointer_cast<Sphere>(gameObject))
            deviceContext.drawIndexed(Sphere::GetIndexCount(), 0, 0);
        else if (std::dynamic_pointer_cast<Cylinder>(gameObject))
            deviceContext.drawIndexed(Cylinder::GetIndexCount(), 0, 0);
        else if (std::dynamic_pointer_cast<Capsule>(gameObject))
            deviceContext.drawIndexed(Capsule::GetIndexCount(), 0, 0);
        else if (std::dynamic_pointer_cast<Plane>(gameObject))
            deviceContext.drawIndexed(Plane::GetIndexCount(), 0, 0);
    }

    // --- RENDER TRANSPARENT PARTICLES ---
    ID3D11BlendState* blendState = nullptr;
    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    ID3D11Device* device = nullptr;
    d3dContext->GetDevice(&device);
    if (device)
    {
        device->CreateBlendState(&blendDesc, &blendState);
        device->Release();
    }

    d3dContext->OMSetBlendState(blendState, blendFactor, 0xffffffff);
    d3dContext->OMSetDepthStencilState(m_particleDepthState, 0); // Use special depth state for particles

    ParticleSystem::getInstance().render(deviceContext, *m_camera, m_projectionMatrix);

    // --- RESTORE DEFAULT STATES ---
    d3dContext->OMSetBlendState(nullptr, blendFactor, 0xffffffff); // Restore default blend state
    d3dContext->OMSetDepthStencilState(m_solidDepthState, 0); // Restore default depth state
    if (blendState) blendState->Release();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    deviceContext.present(swapChain);
}

void dx3d::Game::run()
{
    MSG msg{};
    while (m_isRunning)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                m_isRunning = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!m_isRunning) break;

        update();
        render();
        Input::getInstance().update();
    }
}