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
#include <DX3D/Graphics/Shaders/WhiteShader.h>
#include <DX3D/Math/Math.h>
#include <cmath>
#include <DirectXMath.h>

// The public constructor now delegates to the private one, creating the logger
dx3d::Game::Game(const GameDesc& desc) : Game(desc, std::make_unique<Logger>(desc.logLevel))
{
}

// The private constructor that now does the actual initialization
dx3d::Game::Game(const GameDesc& desc, std::unique_ptr<Logger> logger)
    : Base({ *logger }), // Initialize Base with the logger
    m_loggerPtr(std::move(logger)) // Take ownership of the logger
{
    m_graphicsEngine = std::make_unique<GraphicsEngine>(GraphicsEngineDesc{ m_logger });
    m_display = std::make_unique<Display>(DisplayDesc{ {m_logger, desc.windowSize}, m_graphicsEngine->getRenderSystem() });

    m_previousTime = std::chrono::steady_clock::now();

    createRenderingResources();

    DX3DLogInfo("Game initialized.");
}

dx3d::Game::~Game()
{
    DX3DLogInfo("Game deallocation started.");

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

    // Create vertex and index buffers for cubes
    m_cubeVertexBuffer = Cube::CreateVertexBuffer(resourceDesc);
    m_cubeIndexBuffer = Cube::CreateIndexBuffer(resourceDesc);

    // CREATE DEPTH BUFFER
    const auto& windowSize = m_display->getSize();
    m_depthBuffer = std::make_shared<DepthBuffer>(
        windowSize.width,
        windowSize.height,
        resourceDesc
    );

    // Create Depth Stencil States
    D3D11_DEPTH_STENCIL_DESC solidDesc = {};
    solidDesc.DepthEnable = TRUE;
    solidDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    solidDesc.DepthFunc = D3D11_COMPARISON_LESS;
    solidDesc.StencilEnable = FALSE;
    device->CreateDepthStencilState(&solidDesc, &m_solidDepthState);

    D3D11_DEPTH_STENCIL_DESC particleDesc = {};
    particleDesc.DepthEnable = TRUE;
    particleDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    particleDesc.DepthFunc = D3D11_COMPARISON_LESS;
    particleDesc.StencilEnable = FALSE;
    device->CreateDepthStencilState(&particleDesc, &m_particleDepthState);

    device->Release();

    // Create white shader
    m_whiteVertexShader = std::make_shared<VertexShader>(resourceDesc, WhiteShader::GetVertexShaderCode());
    m_whitePixelShader = std::make_shared<PixelShader>(resourceDesc, WhiteShader::GetPixelShaderCode());

    m_transformConstantBuffer = std::make_shared<ConstantBuffer>(sizeof(TransformationMatrices), resourceDesc);
    //B LEFT
    m_gameObjects.push_back(std::make_shared<Cube>(
        Vector3(0.5f, 0.0f, 0.0f),      // Position
        Vector3(0.0f, 0.0f, -240.0f),      // Rotation
        Vector3(1.5f, 0.05f, 1.0f)       // Scale (width, height, depth)
    ));

    m_objectRotationDeltas.push_back(Vector3(0.0f, 0.0f, 0.0f));

    m_gameObjects.push_back(std::make_shared<Cube>(
        Vector3(0.0f, 0.0f, 0.0f),      // Position
        Vector3(0.0f, 0.0f, 240.0f),      // Rotation
        Vector3(1.5f, 0.05f, 1.0f)       // Scale (width, height, depth)
    ));

    m_objectRotationDeltas.push_back(Vector3(0.0f, 0.0f, 0.0f));

    //B MID
    m_gameObjects.push_back(std::make_shared<Cube>(
        Vector3(1.5f, 0.0f, 0.0f),      // Position
        Vector3(0.0f, 0.0f, -240.0f),      // Rotation
        Vector3(1.5f, 0.05f, 1.0f)       // Scale (width, height, depth)
    ));

    m_objectRotationDeltas.push_back(Vector3(0.0f, 0.0f, 0.0f));

    m_gameObjects.push_back(std::make_shared<Cube>(
        Vector3(1.0f, 0.0f, 0.0f),      // Position
        Vector3(0.0f, 0.0f, 240.0f),      // Rotation
        Vector3(1.5f, 0.05f, 1.0f)       // Scale (width, height, depth)
    ));

    m_objectRotationDeltas.push_back(Vector3(0.0f, 0.0f, 0.0f));

    //B RIGHT
    m_gameObjects.push_back(std::make_shared<Cube>(
        Vector3(2.5f, 0.0f, 0.0f),      // Position
        Vector3(0.0f, 0.0f, -240.0f),      // Rotation
        Vector3(1.5f, 0.05f, 1.0f)       // Scale (width, height, depth)
    ));

    m_objectRotationDeltas.push_back(Vector3(0.0f, 0.0f, 0.0f));

    m_gameObjects.push_back(std::make_shared<Cube>(
        Vector3(2.0f, 0.0f, 0.0f),      // Position
        Vector3(0.0f, 0.0f, 240.0f),      // Rotation
        Vector3(1.5f, 0.05f, 1.0f)       // Scale (width, height, depth)
    ));

    m_objectRotationDeltas.push_back(Vector3(0.0f, 0.0f, 0.0f));
    //MID BORDER L
    m_gameObjects.push_back(std::make_shared<Cube>(
        Vector3(0.6f, 0.7f, 0.0f),      // Position
        Vector3(0.0f, 0.0f, 0.0f),      // Rotation
        Vector3(1.5f, 0.05f, 1.0f)       // Scale (width, height, depth)
    ));

    m_objectRotationDeltas.push_back(Vector3(0.0f, 0.0f, 0.0f));

    //MID BORDER R
    m_gameObjects.push_back(std::make_shared<Cube>(
        Vector3(1.8f, 0.7f, 0.0f),      // Position
        Vector3(0.0f, 0.0f, 0.0f),      // Rotation
        Vector3(1.5f, 0.05f, 1.0f)       // Scale (width, height, depth)
    ));

    m_objectRotationDeltas.push_back(Vector3(0.0f, 0.0f, 0.0f));

    //MID RIGHT
    m_gameObjects.push_back(std::make_shared<Cube>(
        Vector3(1.5f, 1.4f, 0.0f),      // Position
        Vector3(0.0f, 0.0f, 240.0f),      // Rotation
        Vector3(1.5f, 0.05f, 1.0f)       // Scale (width, height, depth)
    ));

    m_objectRotationDeltas.push_back(Vector3(0.0f, 0.0f, 0.0f));

    m_gameObjects.push_back(std::make_shared<Cube>(
        Vector3(2.0f, 1.4f, 0.0f),      // Position
        Vector3(0.0f, 0.0f, -240.0f),      // Rotation
        Vector3(1.5f, 0.05f, 1.0f)       // Scale (width, height, depth)
    ));

    m_objectRotationDeltas.push_back(Vector3(0.0f, 0.0f, 0.0f));

    //MID LEFT
    m_gameObjects.push_back(std::make_shared<Cube>(
        Vector3(1.0f, 1.4f, 0.0f),      // Position
        Vector3(0.0f, 0.0f, -240.0f),      // Rotation
        Vector3(1.5f, 0.05f, 1.0f)       // Scale (width, height, depth)
    ));

    m_objectRotationDeltas.push_back(Vector3(0.0f, 0.0f, 0.0f));

    m_gameObjects.push_back(std::make_shared<Cube>(
        Vector3(0.5f, 1.4f, 0.0f),      // Position
        Vector3(0.0f, 0.0f, 240.0f),      // Rotation
        Vector3(1.5f, 0.05f, 1.0f)       // Scale (width, height, depth)
    ));

    m_objectRotationDeltas.push_back(Vector3(0.0f, 0.0f, 0.0f));

    //MID BORDER R
    m_gameObjects.push_back(std::make_shared<Cube>(
        Vector3(1.2f, 2.1f, 0.0f),      // Position
        Vector3(0.0f, 0.0f, 0.0f),      // Rotation
        Vector3(1.5f, 0.05f, 1.0f)       // Scale (width, height, depth)
    ));

    m_objectRotationDeltas.push_back(Vector3(0.0f, 0.0f, 0.0f));

    //TOP MID
    m_gameObjects.push_back(std::make_shared<Cube>(
        Vector3(1.5f, 2.8f, 0.0f),      // Position
        Vector3(0.0f, 0.0f, -240.0f),      // Rotation
        Vector3(1.5f, 0.05f, 1.0f)       // Scale (width, height, depth)
    ));

    m_objectRotationDeltas.push_back(Vector3(0.0f, 0.0f, 0.0f));

    m_gameObjects.push_back(std::make_shared<Cube>(
        Vector3(1.0f, 2.8f, 0.0f),      // Position
        Vector3(0.0f, 0.0f, 240.0f),      // Rotation
        Vector3(1.5f, 0.05f, 1.0f)       // Scale (width, height, depth)
    ));

    m_objectRotationDeltas.push_back(Vector3(0.0f, 0.0f, 0.0f));


    m_camera = std::make_unique<Camera>(
        Vector3(0.0f, 1.5f, -5.0f),      // Position
        Vector3(0.0f, 0.0f, 0.0f)        // Look at
    );

    // Setup projection matrix
    float aspectRatio = static_cast<float>(windowSize.width) / static_cast<float>(windowSize.height);
    m_projectionMatrix = Matrix4x4::CreatePerspectiveFovLH(
        1.0472f,       // 60 degrees
        aspectRatio,
        0.1f,
        100.0f
    );

    DX3DLogInfo("Rendering resources created for a single cube.");
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
        m_camera->setPosition(Vector3(0.0f, 1.5f, -5.0f));
        m_camera->lookAt(Vector3(0.0f, 0.0f, 0.0f));
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

    processInput(m_deltaTime);
    m_camera->update();

    // Update all objects
    for (size_t i = 0; i < m_gameObjects.size(); ++i)
    {
        m_gameObjects[i]->rotate(m_objectRotationDeltas[i] * m_deltaTime);
        m_gameObjects[i]->update(m_deltaTime);
    }
}

void dx3d::Game::render()
{
    auto& renderSystem = m_graphicsEngine->getRenderSystem();
    auto& deviceContext = renderSystem.getDeviceContext();
    auto& swapChain = m_display->getSwapChain();
    auto d3dContext = deviceContext.getDeviceContext();

    // Clear with gray-blue background like reference
    deviceContext.clearRenderTargetColor(swapChain, 0.45f, 0.55f, 0.65f, 1.0f);
    deviceContext.clearDepthBuffer(*m_depthBuffer);
    deviceContext.setRenderTargetsWithDepth(swapChain, *m_depthBuffer);
    deviceContext.setViewportSize(m_display->getSize().width, m_display->getSize().height);

    ID3D11Buffer* cb = m_transformConstantBuffer->getBuffer();
    d3dContext->VSSetConstantBuffers(0, 1, &cb);

    // Set depth state
    d3dContext->OMSetDepthStencilState(m_solidDepthState, 0);

    // Render all game objects with the white shader
    deviceContext.setVertexBuffer(*m_cubeVertexBuffer);
    deviceContext.setIndexBuffer(*m_cubeIndexBuffer);
    deviceContext.setVertexShader(m_whiteVertexShader->getShader());
    deviceContext.setPixelShader(m_whitePixelShader->getShader());
    deviceContext.setInputLayout(m_whiteVertexShader->getInputLayout());

    for (const auto& obj : m_gameObjects)
    {
        TransformationMatrices transformMatrices;
        transformMatrices.world = Matrix4x4::fromXMMatrix(DirectX::XMMatrixTranspose(obj->getWorldMatrix().toXMMatrix()));
        transformMatrices.view = Matrix4x4::fromXMMatrix(DirectX::XMMatrixTranspose(m_camera->getViewMatrix().toXMMatrix()));
        transformMatrices.projection = Matrix4x4::fromXMMatrix(DirectX::XMMatrixTranspose(m_projectionMatrix.toXMMatrix()));
        m_transformConstantBuffer->update(deviceContext, &transformMatrices);

        deviceContext.drawIndexed(Cube::GetIndexCount(), 0, 0);
    }

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