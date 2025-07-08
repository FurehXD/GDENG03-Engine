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
#include <DX3D/Graphics/RenderTexture.h>
#include <DX3D/Graphics/Primitives/AGameObject.h>
#include <DX3D/Graphics/Primitives/Cube.h>
#include <DX3D/Graphics/Primitives/Plane.h>
#include <DX3D/Graphics/Primitives/Sphere.h>
#include <DX3D/Graphics/Primitives/Cylinder.h>
#include <DX3D/Graphics/Primitives/Capsule.h>
#include <DX3D/Graphics/Primitives/CameraObject.h>
#include <DX3D/Graphics/Shaders/Rainbow3DShader.h>
#include <DX3D/Graphics/Shaders/WhiteShader.h>
#include <DX3D/Graphics/Shaders/FogShader.h>
#include <DX3D/Graphics/Primitives/CameraGizmo.h>
#include <DX3D/Graphics/Primitives/CameraIcon.h>
#include <DX3D/Graphics/Shaders/TextureShader.h>
#include <DX3D/Math/Math.h>
#include <DX3D/Particles/ParticleSystem.h>
#include <DX3D/Particles/ParticleEffects/SnowParticle.h>
#include <DX3D/Game/ViewportManager.h>
#include <DX3D/Game/SelectionSystem.h>
#include <cmath>
#include <random>
#include <string>
#include <cstdio>
#include <DirectXMath.h>
#define STB_IMAGE_IMPLEMENTATION
#include <External/stb_image.h>

dx3d::Game::Game(const GameDesc& desc) :
    Base({ *std::make_unique<Logger>(desc.logLevel).release() }),
    m_loggerPtr(&m_logger)
{
    m_graphicsEngine = std::make_unique<GraphicsEngine>(GraphicsEngineDesc{ m_logger });
    m_display = std::make_unique<Display>(DisplayDesc{ {m_logger,desc.windowSize},m_graphicsEngine->getRenderSystem() });

    m_previousTime = std::chrono::steady_clock::now();

    createRenderingResources();

    DX3DLogInfo("Game initialized with dual viewport system.");
}

dx3d::Game::~Game()
{
    DX3DLogInfo("Game deallocation started.");

    ParticleSystem::getInstance().shutdown();

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
    m_cameraGizmoVertexBuffer = CameraGizmo::CreateVertexBuffer(resourceDesc);
    m_cameraGizmoIndexBuffer = CameraGizmo::CreateIndexBuffer(resourceDesc);
    m_cameraIconVertexBuffer = CameraIcon::CreateVertexBuffer(resourceDesc);
    m_cameraIconIndexBuffer = CameraIcon::CreateIndexBuffer(resourceDesc);

    //def layout for shaders that use Position and Color
    D3D11_INPUT_ELEMENT_DESC defaultLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };


    //update existing vertex shaders
    m_rainbowVertexShader = std::make_shared<VertexShader>(resourceDesc, Rainbow3DShader::GetVertexShaderCode(), defaultLayout, 2);
    m_whiteVertexShader = std::make_shared<VertexShader>(resourceDesc, WhiteShader::GetVertexShaderCode(), defaultLayout, 2);
    m_fogVertexShader = std::make_shared<VertexShader>(resourceDesc, FogShader::GetVertexShaderCode(), defaultLayout, 2);
    m_fogConstantBuffer = std::make_shared<ConstantBuffer>(sizeof(FogShaderConstants), resourceDesc);
    m_materialConstantBuffer = std::make_shared<ConstantBuffer>(sizeof(FogMaterialConstants), resourceDesc);

    m_transformConstantBuffer = std::make_shared<ConstantBuffer>(sizeof(TransformationMatrices), resourceDesc);

    //def texture layout for shaders that use Position and TexCoord
    D3D11_INPUT_ELEMENT_DESC textureLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    //texture vertex shader
    m_textureVertexShader = std::make_shared<VertexShader>(resourceDesc, TextureShader::GetVertexShaderCode(), defaultLayout, 2);
    m_texturePixelShader = std::make_shared<PixelShader>(resourceDesc, TextureShader::GetPixelShaderCode());

    //load texture using a simple relative path
    int width, height, channels;
    const char* imagePath = "Resources/cam.png";
    unsigned char* image_data = stbi_load(imagePath, &width, &height, &channels, 4);
    if (!image_data)
    {
        const char* failure_reason = stbi_failure_reason();
        std::string error_message = "Failed to load texture file: " + std::string(imagePath) + "\nReason: " + std::string(failure_reason);
        DX3DLogErrorAndThrow(error_message.c_str());
    }

    
    //texture resource
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pSysMem = image_data;
    subresourceData.SysMemPitch = width * 4; //4 bytes per pixel because (RGBA)

    
    ID3D11Texture2D* texture = nullptr;
    HRESULT hr = device->CreateTexture2D(&textureDesc, &subresourceData, &texture);

    stbi_image_free(image_data);
    //important: Free the image data from CPU memory

    if (FAILED(hr))
    {
        DX3DLogErrorAndThrow("Failed to create 2D texture.");
    }

    //shader resource view from the texture
    hr = device->CreateShaderResourceView(texture, nullptr, &m_cameraIconTexture);
    texture->Release();

    if (FAILED(hr))
    {
        DX3DLogErrorAndThrow("Failed to create shader resource view.");
    }

    //sampler state
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = device->CreateSamplerState(&samplerDesc, &m_samplerState);
    if (FAILED(hr))
    {
        DX3DLogErrorAndThrow("Failed to create sampler state.");
    }
    

    const auto& windowSize = m_display->getSize();
    m_depthBuffer = std::make_shared<DepthBuffer>(
        windowSize.width,
        windowSize.height,
        resourceDesc
    );

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

    m_rainbowVertexShader = std::make_shared<VertexShader>(resourceDesc, Rainbow3DShader::GetVertexShaderCode());
    m_rainbowPixelShader = std::make_shared<PixelShader>(resourceDesc, Rainbow3DShader::GetPixelShaderCode());
    m_whiteVertexShader = std::make_shared<VertexShader>(resourceDesc, WhiteShader::GetVertexShaderCode());
    m_whitePixelShader = std::make_shared<PixelShader>(resourceDesc, WhiteShader::GetPixelShaderCode());
    m_fogVertexShader = std::make_shared<VertexShader>(resourceDesc, FogShader::GetVertexShaderCode());
    m_fogPixelShader = std::make_shared<PixelShader>(resourceDesc, FogShader::GetPixelShaderCode());
    m_fogConstantBuffer = std::make_shared<ConstantBuffer>(sizeof(FogShaderConstants), resourceDesc);
    m_materialConstantBuffer = std::make_shared<ConstantBuffer>(sizeof(FogMaterialConstants), resourceDesc);


    m_transformConstantBuffer = std::make_shared<ConstantBuffer>(sizeof(TransformationMatrices), resourceDesc);

    // Initialize snow configuration with proper default values
    m_snowConfig.position = Vector3(0.0f, 10.0f, 0.0f);
    m_snowConfig.positionVariance = Vector3(20.0f, 0.0f, 20.0f);
    m_snowConfig.velocity = Vector3(0.0f, -2.0f, 0.0f);
    m_snowConfig.velocityVariance = Vector3(0.5f, 0.5f, 0.5f);
    m_snowConfig.acceleration = Vector3(0.0f, -0.5f, 0.0f);
    m_snowConfig.startColor = Vector4(1.0f, 1.0f, 1.0f, 0.8f);  // White
    m_snowConfig.endColor = Vector4(0.9f, 0.9f, 1.0f, 0.0f);    // Light blue, transparent
    m_snowConfig.startSize = 0.2f;
    m_snowConfig.endSize = 0.1f;
    m_snowConfig.lifetime = 8.0f;
    m_snowConfig.lifetimeVariance = 2.0f;
    m_snowConfig.emissionRate = 50.0f;
    m_snowConfig.active = true;

    m_gameObjects.clear();
    m_gameObjects.reserve(12);

    const float radius = 6.0f;
    const int numCubes = 10;

    for (int i = 0; i < numCubes; ++i)
    {
        float angle = (static_cast<float>(i) / numCubes) * 2.0f * 3.14159265f;
        float x = radius * std::cos(angle);
        float z = radius * std::sin(angle);

        m_gameObjects.push_back(std::make_shared<Cube>(
            Vector3(x, 2.0f, z),
            Vector3(0.0f, 0.0f, 0.0f),
            Vector3(1.5f, 1.5f, 1.5f)
        ));
    }

    m_gameObjects.push_back(std::make_shared<Plane>(
        Vector3(0.0f, 0.0f, 0.0f),
        Vector3(-1.5708f, 0.0f, 0.0f),
        Vector3(15.0f, 15.0f, 1.0f)
    ));

    m_gameCamera = std::make_shared<CameraObject>(
        Vector3(12.0f, 8.0f, -12.0f),
        Vector3(0.0f, 0.0f, 0.0f)
    );
    m_gameCamera->getCamera().lookAt(Vector3(0.0f, 2.0f, 0.0f));
    m_gameObjects.push_back(m_gameCamera);

    m_sceneCamera = std::make_unique<Camera>(
        Vector3(15.0f, 10.0f, -15.0f),
        Vector3(0.0f, 2.0f, 0.0f)
    );

    float aspectRatio = static_cast<float>(windowSize.width) / static_cast<float>(windowSize.height);
    m_projectionMatrix = Matrix4x4::CreatePerspectiveFovLH(
        1.0472f,
        aspectRatio,
        0.1f,
        100.0f
    );

    m_viewportManager = std::make_unique<ViewportManager>();
    m_viewportManager->initialize(*m_graphicsEngine, 640, 480);

    m_selectionSystem = std::make_unique<SelectionSystem>();

    ParticleSystem::getInstance().initialize(*m_graphicsEngine);

    ParticleEmitter::EmitterConfig snowConfig;
    snowConfig.position = m_snowConfig.position;
    snowConfig.positionVariance = m_snowConfig.positionVariance;
    snowConfig.velocity = m_snowConfig.velocity;
    snowConfig.velocityVariance = m_snowConfig.velocityVariance;
    snowConfig.acceleration = m_snowConfig.acceleration;
    snowConfig.startColor = m_snowConfig.startColor;
    snowConfig.endColor = m_snowConfig.endColor;
    snowConfig.startSize = m_snowConfig.startSize;
    snowConfig.endSize = m_snowConfig.endSize;
    snowConfig.lifetime = m_snowConfig.lifetime;
    snowConfig.lifetimeVariance = m_snowConfig.lifetimeVariance;
    snowConfig.emissionRate = m_snowConfig.emissionRate;
    snowConfig.maxParticles = 2000;

    auto snowEmitter = ParticleSystem::getInstance().createEmitter(
        "snow",
        snowConfig,
        createSnowParticle
    );

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    HWND hwnd = m_display->getWindowHandle();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, d3dContext);

    //Blend state for UI and sprite alpha blending (for PNGs with premultiplied alpha)
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    d3dContext->GetDevice(&device);
    if (device)
    {
        device->CreateBlendState(&blendDesc, &m_alphaBlendState);
        device->Release();
    }

    // Create a rasterizer state that disables back-face culling
    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.FrontCounterClockwise = TRUE;
    device->CreateRasterizerState(&rasterizerDesc, &m_noCullRasterizerState);

    device->Release();
}

void dx3d::Game::processInput(float deltaTime)
{
    auto& input = Input::getInstance();
    auto& sceneViewport = m_viewportManager->getViewport(ViewportType::Scene);

    if (sceneViewport.isFocused && input.isMouseButtonPressed(MouseButton::Right))
    {
        float moveSpeed = m_cameraSpeed * deltaTime;
        if (input.isKeyPressed(KeyCode::W)) m_sceneCamera->moveForward(moveSpeed);
        if (input.isKeyPressed(KeyCode::S)) m_sceneCamera->moveBackward(moveSpeed);
        if (input.isKeyPressed(KeyCode::A)) m_sceneCamera->moveLeft(moveSpeed);
        if (input.isKeyPressed(KeyCode::D)) m_sceneCamera->moveRight(moveSpeed);
        if (input.isKeyPressed(KeyCode::Q)) m_sceneCamera->moveDown(moveSpeed);
        if (input.isKeyPressed(KeyCode::E)) m_sceneCamera->moveUp(moveSpeed);

        float mouseDeltaX = static_cast<float>(input.getMouseDeltaX());
        float mouseDeltaY = static_cast<float>(input.getMouseDeltaY());

        if (mouseDeltaX != 0.0f || mouseDeltaY != 0.0f)
        {
            m_sceneCamera->onMouseMove(mouseDeltaX, mouseDeltaY, m_mouseSensitivity * 0.01f);
        }
    }

    if (sceneViewport.isHovered && input.isMouseButtonJustPressed(MouseButton::Left))
    {
        auto picked = m_selectionSystem->pickObject(
            m_gameObjects,
            *m_sceneCamera,
            sceneViewport.mousePos.x,
            sceneViewport.mousePos.y,
            sceneViewport.width,
            sceneViewport.height
        );
        m_selectionSystem->setSelectedObject(picked);
    }

    if (input.isKeyPressed(KeyCode::W))
    {
        Vector3 rotationDelta(m_cubeRotationSpeed * deltaTime, m_cubeRotationSpeed * deltaTime, m_cubeRotationSpeed * deltaTime);
        for (size_t i = 0; i < m_gameObjects.size() - 2; ++i)
        {
            if (std::dynamic_pointer_cast<Cube>(m_gameObjects[i]))
            {
                m_gameObjects[i]->rotate(rotationDelta);
            }
        }
    }

    if (input.isKeyPressed(KeyCode::S))
    {
        Vector3 rotationDelta(-m_cubeRotationSpeed * deltaTime, -m_cubeRotationSpeed * deltaTime, -m_cubeRotationSpeed * deltaTime);
        for (size_t i = 0; i < m_gameObjects.size() - 2; ++i)
        {
            if (std::dynamic_pointer_cast<Cube>(m_gameObjects[i]))
            {
                m_gameObjects[i]->rotate(rotationDelta);
            }
        }
    }

    if (input.isKeyJustPressed(KeyCode::R))
    {
        m_sceneCamera->setPosition(Vector3(15.0f, 10.0f, -15.0f));
        m_sceneCamera->lookAt(Vector3(0.0f, 2.0f, 0.0f));
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

    processInput(m_deltaTime);
    m_sceneCamera->update();

    for (auto& gameObject : m_gameObjects)
    {
        gameObject->update(m_deltaTime);
    }

    ParticleSystem::getInstance().update(m_deltaTime);

    if (auto snowEmitter = ParticleSystem::getInstance().getEmitter("snow"))
    {
        snowEmitter->setPosition(m_snowConfig.position);
    }
}

void dx3d::Game::updateSnowEmitter()
{
    auto snowEmitter = ParticleSystem::getInstance().getEmitter("snow");
    if (!snowEmitter)
        return;

    if (m_snowConfig.active)
    {
        snowEmitter->start();
    }
    else
    {
        snowEmitter->stop();
        return;
    }

    snowEmitter->setEmissionRate(m_snowConfig.emissionRate);

    ParticleEmitter::EmitterConfig newConfig;
    newConfig.position = m_snowConfig.position;
    newConfig.positionVariance = m_snowConfig.positionVariance;
    newConfig.velocity = m_snowConfig.velocity;
    newConfig.velocityVariance = m_snowConfig.velocityVariance;
    newConfig.acceleration = m_snowConfig.acceleration;
    newConfig.startColor = m_snowConfig.startColor;
    newConfig.endColor = m_snowConfig.endColor;
    newConfig.startSize = m_snowConfig.startSize;
    newConfig.endSize = m_snowConfig.endSize;
    newConfig.lifetime = m_snowConfig.lifetime;
    newConfig.lifetimeVariance = m_snowConfig.lifetimeVariance;
    newConfig.emissionRate = m_snowConfig.emissionRate;
    newConfig.maxParticles = 2000;
    newConfig.loop = true;

    ParticleSystem::getInstance().removeEmitter("snow");
    ParticleSystem::getInstance().createEmitter("snow", newConfig, createSnowParticle);
}

void dx3d::Game::renderScene(Camera& camera, const Matrix4x4& projMatrix, RenderTexture* renderTarget)
{
    auto& renderSystem = m_graphicsEngine->getRenderSystem();
    auto& deviceContext = renderSystem.getDeviceContext();
    auto d3dContext = deviceContext.getDeviceContext();

    if (renderTarget)
    {
        renderTarget->clear(deviceContext, m_fogDesc.color.x, m_fogDesc.color.y, m_fogDesc.color.z, 1.0f);
        renderTarget->setAsRenderTarget(deviceContext);
    }
    else
    {
        auto& swapChain = m_display->getSwapChain();
        deviceContext.clearRenderTargetColor(swapChain, m_fogDesc.color.x, m_fogDesc.color.y, m_fogDesc.color.z, 1.0f);
        deviceContext.clearDepthBuffer(*m_depthBuffer);
        deviceContext.setRenderTargetsWithDepth(swapChain, *m_depthBuffer);
    }
    
    if (renderTarget)
    {
        const auto& viewportSize = renderTarget->getSize();
        deviceContext.setViewportSize(viewportSize.width, viewportSize.height);
    }
    else
    {
        const auto& windowSize = m_display->getSize();
        deviceContext.setViewportSize(windowSize.width, windowSize.height);
    }

    /*
    deviceContext.setViewportSize(
        renderTarget ? renderTarget->getShaderResourceView() ? 640 : m_display->getSize().width : m_display->getSize().width,
        renderTarget ? renderTarget->getShaderResourceView() ? 480 : m_display->getSize().height : m_display->getSize().height
    );
    */

    ID3D11Buffer* transformCb = m_transformConstantBuffer->getBuffer();
    d3dContext->VSSetConstantBuffers(0, 1, &transformCb);

    FogShaderConstants fsc = {};
    fsc.fogColor = m_fogDesc.color;
    fsc.cameraPosition = camera.getPosition();
    fsc.fogStart = m_fogDesc.start;
    fsc.fogEnd = m_fogDesc.end;
    fsc.fogEnabled = m_fogDesc.enabled;
    m_fogConstantBuffer->update(deviceContext, &fsc);

    ID3D11Buffer* fogCb = m_fogConstantBuffer->getBuffer();
    d3dContext->PSSetConstantBuffers(1, 1, &fogCb);
    ID3D11Buffer* materialCb = m_materialConstantBuffer->getBuffer();
    d3dContext->PSSetConstantBuffers(2, 1, &materialCb);

    d3dContext->OMSetDepthStencilState(m_solidDepthState, 0);

    deviceContext.setVertexShader(m_fogVertexShader->getShader());
    deviceContext.setPixelShader(m_fogPixelShader->getShader());
    deviceContext.setInputLayout(m_fogVertexShader->getInputLayout());

    bool isSceneView = (&camera == m_sceneCamera.get());

    for (const auto& gameObject : m_gameObjects)
    {
        bool isCamera = std::dynamic_pointer_cast<CameraObject>(gameObject) != nullptr;
        if (!isSceneView && isCamera)
            continue;

        FogMaterialConstants fmc = {};
        if (std::dynamic_pointer_cast<Plane>(gameObject))
        {
            fmc.useVertexColor = false;
            fmc.baseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        }
        else
        {
            fmc.useVertexColor = true;
        }
        m_materialConstantBuffer->update(deviceContext, &fmc);

        if (auto cube = std::dynamic_pointer_cast<Cube>(gameObject)) {
            deviceContext.setVertexBuffer(*m_cubeVertexBuffer);
            deviceContext.setIndexBuffer(*m_cubeIndexBuffer);
        }
        else if (auto sphere = std::dynamic_pointer_cast<Sphere>(gameObject)) {
            deviceContext.setVertexBuffer(*m_sphereVertexBuffer);
            deviceContext.setIndexBuffer(*m_sphereIndexBuffer);
        }
        else if (auto cylinder = std::dynamic_pointer_cast<Cylinder>(gameObject)) {
            deviceContext.setVertexBuffer(*m_cylinderVertexBuffer);
            deviceContext.setIndexBuffer(*m_cylinderIndexBuffer);
        }
        else if (auto capsule = std::dynamic_pointer_cast<Capsule>(gameObject)) {
            deviceContext.setVertexBuffer(*m_capsuleVertexBuffer);
            deviceContext.setIndexBuffer(*m_capsuleIndexBuffer);
        }
        else if (auto plane = std::dynamic_pointer_cast<Plane>(gameObject)) {
            deviceContext.setVertexBuffer(*m_planeVertexBuffer);
            deviceContext.setIndexBuffer(*m_planeIndexBuffer);
            deviceContext.drawIndexed(Plane::GetIndexCount(), 0, 0);
        }
        else if (isCamera && isSceneView && m_showGizmos) {
            {// --- 1. Render the 3D Camera Gizmo ---
           
                // Set gizmo geometry
                deviceContext.setVertexBuffer(*m_cameraGizmoVertexBuffer);
                deviceContext.setIndexBuffer(*m_cameraGizmoIndexBuffer);

                //set gizmo transformations
                TransformationMatrices transformMatrices;
                Matrix4x4 world = Matrix4x4::CreateRotationX(gameObject->getRotation().x) *
                    Matrix4x4::CreateRotationY(gameObject->getRotation().y) *
                    Matrix4x4::CreateRotationZ(gameObject->getRotation().z) *
                    Matrix4x4::CreateTranslation(gameObject->getPosition());

                transformMatrices.world = Matrix4x4::fromXMMatrix(DirectX::XMMatrixTranspose(world.toXMMatrix()));
                transformMatrices.view = Matrix4x4::fromXMMatrix(DirectX::XMMatrixTranspose(camera.getViewMatrix().toXMMatrix()));
                transformMatrices.projection = Matrix4x4::fromXMMatrix(DirectX::XMMatrixTranspose(projMatrix.toXMMatrix()));
                m_transformConstantBuffer->update(deviceContext, &transformMatrices);

                deviceContext.drawIndexed(CameraGizmo::GetIndexCount(), 0, 0);
            }

            
            {// --- 2. Render the 2D Camera Icon ---
                //store original states
                ID3D11RasterizerState* originalRasterizerState = nullptr;
                d3dContext->RSGetState(&originalRasterizerState);

                //spply states for transparency
                d3dContext->OMSetBlendState(m_alphaBlendState, nullptr, 0xffffffff);
                d3dContext->RSSetState(m_noCullRasterizerState);
                d3dContext->OMSetDepthStencilState(m_particleDepthState, 0);

                //shaders and resources
                deviceContext.setVertexShader(m_textureVertexShader->getShader());
                deviceContext.setPixelShader(m_texturePixelShader->getShader());
                deviceContext.setInputLayout(m_textureVertexShader->getInputLayout());
                deviceContext.setVertexBuffer(*m_cameraIconVertexBuffer);
                deviceContext.setIndexBuffer(*m_cameraIconIndexBuffer);
                d3dContext->PSSetShaderResources(0, 1, &m_cameraIconTexture);
                d3dContext->PSSetSamplers(0, 1, &m_samplerState);

                //set transformations for the icon w/ billboarding effect
                TransformationMatrices transformMatrices;
                Vector3 iconPos = gameObject->getPosition();
                Vector3 camToIconDir = iconPos - camera.getPosition();
                float angleY = atan2(camToIconDir.x, camToIconDir.z);
                Matrix4x4 world = Matrix4x4::CreateScale({ 0.75f, 0.75f, 0.75f }) * 
                    Matrix4x4::CreateRotationY(angleY) * Matrix4x4::CreateTranslation(iconPos);

                transformMatrices.world = Matrix4x4::fromXMMatrix(DirectX::XMMatrixTranspose(world.toXMMatrix()));
                transformMatrices.view = Matrix4x4::fromXMMatrix(DirectX::XMMatrixTranspose(camera.getViewMatrix().toXMMatrix()));
                transformMatrices.projection = Matrix4x4::fromXMMatrix(DirectX::XMMatrixTranspose(projMatrix.toXMMatrix()));
                m_transformConstantBuffer->update(deviceContext, &transformMatrices);

                deviceContext.drawIndexed(CameraIcon::GetIndexCount(), 0, 0);

                //restore original states
                d3dContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
                d3dContext->RSSetState(originalRasterizerState);
                d3dContext->OMSetDepthStencilState(m_solidDepthState, 0);
                if (originalRasterizerState) originalRasterizerState->Release();
            }
        }

        TransformationMatrices transformMatrices;
        transformMatrices.world = Matrix4x4::fromXMMatrix(DirectX::XMMatrixTranspose(gameObject->getWorldMatrix().toXMMatrix()));
        transformMatrices.view = Matrix4x4::fromXMMatrix(DirectX::XMMatrixTranspose(camera.getViewMatrix().toXMMatrix()));
        transformMatrices.projection = Matrix4x4::fromXMMatrix(DirectX::XMMatrixTranspose(projMatrix.toXMMatrix()));
        m_transformConstantBuffer->update(deviceContext, &transformMatrices);

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
    d3dContext->OMSetDepthStencilState(m_particleDepthState, 0);

    ParticleSystem::getInstance().render(deviceContext, camera, projMatrix);

    d3dContext->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
    d3dContext->OMSetDepthStencilState(m_solidDepthState, 0);
    if (blendState) blendState->Release();
}

void dx3d::Game::renderUI()
{
    ImGuiIO& io = ImGui::GetIO();
    float windowWidth = io.DisplaySize.x;
    float windowHeight = io.DisplaySize.y;

    float leftPanelWidth = windowWidth * 0.75f;
    float viewportHeight = windowHeight * 0.5f;
    float rightPanelWidth = windowWidth - leftPanelWidth;

    // Scene View - Top Left
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(leftPanelWidth, viewportHeight));
    ImGui::Begin("Scene View", nullptr,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);

    ImVec2 sceneViewportSize = ImGui::GetContentRegionAvail();
    if (sceneViewportSize.x > 0 && sceneViewportSize.y > 0)
    {
        m_viewportManager->resize(ViewportType::Scene,
            static_cast<ui32>(sceneViewportSize.x),
            static_cast<ui32>(sceneViewportSize.y));

        auto& sceneViewport = m_viewportManager->getViewport(ViewportType::Scene);
        ImGui::Image((void*)sceneViewport.renderTexture->getShaderResourceView(), sceneViewportSize);

        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 windowPos = ImGui::GetWindowPos();
        float localX = mousePos.x - windowPos.x - 8;
        float localY = mousePos.y - windowPos.y - ImGui::GetFrameHeight() - 4;

        m_viewportManager->updateViewportStates(
            ViewportType::Scene,
            ImGui::IsWindowHovered(),
            ImGui::IsWindowFocused(),
            localX,
            localY
        );
    }
    ImGui::End();

    // Game View - Bottom Left
    ImGui::SetNextWindowPos(ImVec2(0, viewportHeight));
    ImGui::SetNextWindowSize(ImVec2(leftPanelWidth, viewportHeight));
    ImGui::Begin("Game View", nullptr,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);

    ImVec2 gameViewportSize = ImGui::GetContentRegionAvail();
    if (gameViewportSize.x > 0 && gameViewportSize.y > 0)
    {
        m_viewportManager->resize(ViewportType::Game,
            static_cast<ui32>(gameViewportSize.x),
            static_cast<ui32>(gameViewportSize.y));

        auto& gameViewport = m_viewportManager->getViewport(ViewportType::Game);
        ImGui::Image((void*)gameViewport.renderTexture->getShaderResourceView(), gameViewportSize);
    }
    ImGui::End();

    // Settings Panel - Right Full Height
    ImGui::SetNextWindowPos(ImVec2(leftPanelWidth, 0));
    ImGui::SetNextWindowSize(ImVec2(rightPanelWidth, windowHeight));
    ImGui::Begin("Settings", nullptr,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);

    // Inspector Section
    if (ImGui::CollapsingHeader("Inspector", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (auto selected = m_selectionSystem->getSelectedObject())
        {
            ImGui::Text("Selected: %s", typeid(*selected).name());

            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                Vector3 pos = selected->getPosition();
                if (ImGui::DragFloat3("Position", &pos.x, 0.1f))
                {
                    selected->setPosition(pos);
                }

                Vector3 rot = selected->getRotation();
                if (ImGui::DragFloat3("Rotation", &rot.x, 0.01f))
                {
                    selected->setRotation(rot);
                }

                Vector3 scale = selected->getScale();
                if (ImGui::DragFloat3("Scale", &scale.x, 0.1f))
                {
                    selected->setScale(scale);
                }
            }

            if (auto camera = std::dynamic_pointer_cast<CameraObject>(selected))
            {
                if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    float fov = camera->getFOV() * 180.0f / 3.14159265f;
                    if (ImGui::SliderFloat("FOV", &fov, 30.0f, 120.0f))
                    {
                        camera->setFOV(fov * 3.14159265f / 180.0f);
                    }

                    float nearPlane = camera->getNearPlane();
                    if (ImGui::DragFloat("Near", &nearPlane, 0.01f, 0.01f, 10.0f))
                    {
                        camera->setNearPlane(nearPlane);
                    }

                    float farPlane = camera->getFarPlane();
                    if (ImGui::DragFloat("Far", &farPlane, 1.0f, 10.0f, 1000.0f))
                    {
                        camera->setFarPlane(farPlane);
                    }

                    if (ImGui::Button("Align with View"))
                    {
                        camera->alignWithView(*m_sceneCamera);
                    }
                }
            }
        }
        else
        {
            ImGui::Text("No object selected");
        }
    }

    // Effects Settings Section
    if (ImGui::CollapsingHeader("Effects", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::CollapsingHeader("Fog", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("Enable Fog", &m_fogDesc.enabled);
            ImGui::SliderFloat("Fog Start", &m_fogDesc.start, 0.1f, 50.0f);
            ImGui::SliderFloat("Fog End", &m_fogDesc.end, 1.0f, 100.0f);
            ImGui::ColorEdit3("Fog Color", &m_fogDesc.color.x);
        }

        if (ImGui::CollapsingHeader("Snow Particles", ImGuiTreeNodeFlags_DefaultOpen))
        {
            bool snowSettingsChanged = false;

            if (ImGui::Checkbox("Active", &m_snowConfig.active))
                snowSettingsChanged = true;

            if (ImGui::DragFloat3("Position", &m_snowConfig.position.x, 0.5f))
                snowSettingsChanged = true;

            if (ImGui::ColorEdit4("Start Color", &m_snowConfig.startColor.x))
                snowSettingsChanged = true;

            if (ImGui::ColorEdit4("End Color", &m_snowConfig.endColor.x))
                snowSettingsChanged = true;

            if (ImGui::SliderFloat("Emission Rate", &m_snowConfig.emissionRate, 1.0f, 200.0f))
                snowSettingsChanged = true;

            if (ImGui::SliderFloat("Start Size", &m_snowConfig.startSize, 0.1f, 2.0f))
                snowSettingsChanged = true;

            if (ImGui::SliderFloat("End Size", &m_snowConfig.endSize, 0.05f, 1.0f))
                snowSettingsChanged = true;

            if (ImGui::SliderFloat("Lifetime", &m_snowConfig.lifetime, 1.0f, 20.0f))
                snowSettingsChanged = true;

            if (snowSettingsChanged)
            {
                updateSnowEmitter();
            }
        }
    }

    // Camera Controls Section
    if (ImGui::CollapsingHeader("Camera Controls", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Toggle Gizmos On/Off
        ImGui::Checkbox("Show Gizmos", &m_showGizmos);

        if (ImGui::CollapsingHeader("Scene Camera", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Speed: %.1f", m_cameraSpeed);
            ImGui::SliderFloat("##CameraSpeed", &m_cameraSpeed, 1.0f, 20.0f);

            ImGui::Text("Mouse Sens: %.2f", m_mouseSensitivity);
            ImGui::SliderFloat("##MouseSens", &m_mouseSensitivity, 0.1f, 2.0f);

            if (ImGui::Button("Reset Scene Camera"))
            {
                m_sceneCamera->setPosition(Vector3(15.0f, 10.0f, -15.0f));
                m_sceneCamera->lookAt(Vector3(0.0f, 2.0f, 0.0f));
            }
        }

        if (ImGui::CollapsingHeader("Game Camera", ImGuiTreeNodeFlags_DefaultOpen))
        {
            Vector3 gameCamPos = m_gameCamera->getPosition();
            if (ImGui::DragFloat3("Position", &gameCamPos.x, 0.1f))
            {
                m_gameCamera->setPosition(gameCamPos);
            }

            Vector3 gameCamRot = m_gameCamera->getRotation();
            Vector3 gameCamRotDeg = Vector3(
                gameCamRot.x * 180.0f / 3.14159265f,
                gameCamRot.y * 180.0f / 3.14159265f,
                gameCamRot.z * 180.0f / 3.14159265f
            );
            if (ImGui::DragFloat3("Rotation", &gameCamRotDeg.x, 1.0f))
            {
                Vector3 newRotRad = Vector3(
                    gameCamRotDeg.x * 3.14159265f / 180.0f,
                    gameCamRotDeg.y * 3.14159265f / 180.0f,
                    gameCamRotDeg.z * 3.14159265f / 180.0f
                );
                m_gameCamera->setRotation(newRotRad);
            }

            if (ImGui::Button("Reset Game Camera"))
            {
                m_gameCamera->setPosition(Vector3(12.0f, 8.0f, -12.0f));
                m_gameCamera->setRotation(Vector3(0.0f, 0.0f, 0.0f));
                m_gameCamera->getCamera().lookAt(Vector3(0.0f, 2.0f, 0.0f));
            }

            if (ImGui::Button("Align Game Camera To View"))
            {
                m_gameCamera->alignWithView(*m_sceneCamera.get());
            }
        }
    }

    ImGui::End();
}



void dx3d::Game::render()
{
    auto& renderSystem = m_graphicsEngine->getRenderSystem();
    auto& deviceContext = renderSystem.getDeviceContext();
    auto& swapChain = m_display->getSwapChain();
    auto d3dContext = deviceContext.getDeviceContext();

    auto& sceneViewport = m_viewportManager->getViewport(ViewportType::Scene);
    auto& gameViewport = m_viewportManager->getViewport(ViewportType::Game);

    float aspectRatio = static_cast<float>(sceneViewport.width) / static_cast<float>(sceneViewport.height);
    Matrix4x4 sceneProjMatrix = Matrix4x4::CreatePerspectiveFovLH(1.0472f, aspectRatio, 0.1f, 100.0f);
    renderScene(*m_sceneCamera, sceneProjMatrix, sceneViewport.renderTexture.get());

    aspectRatio = static_cast<float>(gameViewport.width) / static_cast<float>(gameViewport.height);
    Matrix4x4 gameProjMatrix = m_gameCamera->getProjectionMatrix(aspectRatio);
    renderScene(m_gameCamera->getCamera(), gameProjMatrix, gameViewport.renderTexture.get());

    deviceContext.clearRenderTargetColor(swapChain, 0.1f, 0.1f, 0.1f, 1.0f);
    deviceContext.clearDepthBuffer(*m_depthBuffer);
    deviceContext.setRenderTargetsWithDepth(swapChain, *m_depthBuffer);
    deviceContext.setViewportSize(m_display->getSize().width, m_display->getSize().height);

    renderUI();

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