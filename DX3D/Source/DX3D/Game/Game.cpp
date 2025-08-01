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

#include <DX3D/Graphics/Primitives/CameraObject.h>
#include <DX3D/Graphics/Shaders/Rainbow3DShader.h>
#include <DX3D/Graphics/Shaders/WhiteShader.h>
#include <DX3D/Graphics/Shaders/FogShader.h>
#include <DX3D/Graphics/Shaders/ModelShader.h>
#include <DX3D/Graphics/Shaders/ModelVertexShader.h>
#include <DX3D/Math/Math.h>
#include <DX3D/Game/ViewportManager.h>
#include <DX3D/Game/SelectionSystem.h>
#include <DX3D/Scene/SceneStateManager.h>
#include <DX3D/Game/FPSCameraController.h>
#include <DX3D/Game/UndoRedoSystem.h>

#include <DX3D/Graphics/Primitives/AGameObject.h>
#include <DX3D/Graphics/Primitives/Cube.h>
#include <DX3D/Graphics/Primitives/Plane.h>
#include <DX3D/Graphics/Primitives/Sphere.h>
#include <DX3D/Graphics/Primitives/Capsule.h>
#include <DX3D/Graphics/Primitives/Cylinder.h>
#include <DX3D/Graphics/Primitives/Model.h>
#include <DX3D/Assets/ModelLoader.h>

#include <DX3D/ECS/ComponentManager.h>
#include <DX3D/ECS/Components/TransformComponent.h>
#include <DX3D/ECS/Components/PhysicsComponent.h>
#include <DX3D/Physics/PhysicsSystem.h>

#include <DX3D/UI/UIManager.h>

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

    m_sceneStateManager = std::make_unique<SceneStateManager>();
    m_fpsController = std::make_unique<FPSCameraController>();

    m_undoRedoSystem = std::make_unique<UndoRedoSystem>(10);

    createRenderingResources();

    m_fpsController->setCamera(m_sceneCamera.get());
    m_sceneStateManager->addStateChangeCallback([this](SceneState oldState, SceneState newState) {
        onSceneStateChanged(oldState, newState);
        });

    UIManager::Dependencies uiDeps{
    m_logger,
    *m_undoRedoSystem,
    *m_selectionSystem,
    *m_sceneStateManager,
    *m_viewportManager,
    m_gameObjects
    };
    m_uiManager = std::make_unique<UIManager>(uiDeps);

    DX3DLogInfo("Game initialized with ECS, Physics, and Scene State systems.");
}

dx3d::Game::~Game()
{
    DX3DLogInfo("Game deallocation started.");

    PhysicsSystem::getInstance().shutdown();

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

    auto& componentManager = ComponentManager::getInstance();
    componentManager.registerComponent<TransformComponent>();
    componentManager.registerComponent<PhysicsComponent>();

    PhysicsSystem::getInstance().initialize();

    DX3DLogInfo("ECS and Physics systems initialized successfully.");

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

    m_modelVertexShader = createModelVertexShader(resourceDesc);
    m_modelPixelShader = std::make_shared<PixelShader>(resourceDesc, ModelShader::GetPixelShaderCode());
    m_rainbowVertexShader = std::make_shared<VertexShader>(resourceDesc, Rainbow3DShader::GetVertexShaderCode());
    m_rainbowPixelShader = std::make_shared<PixelShader>(resourceDesc, Rainbow3DShader::GetPixelShaderCode());
    m_whiteVertexShader = std::make_shared<VertexShader>(resourceDesc, WhiteShader::GetVertexShaderCode());
    m_whitePixelShader = std::make_shared<PixelShader>(resourceDesc, WhiteShader::GetPixelShaderCode());
    m_fogVertexShader = std::make_shared<VertexShader>(resourceDesc, FogShader::GetVertexShaderCode());
    m_fogPixelShader = std::make_shared<PixelShader>(resourceDesc, FogShader::GetPixelShaderCode());

    m_fogConstantBuffer = std::make_shared<ConstantBuffer>(sizeof(FogShaderConstants), resourceDesc);
    m_materialConstantBuffer = std::make_shared<ConstantBuffer>(sizeof(FogMaterialConstants), resourceDesc);
    m_modelMaterialConstantBuffer = std::make_shared<ConstantBuffer>(sizeof(ModelMaterialConstants), resourceDesc);
    m_transformConstantBuffer = std::make_shared<ConstantBuffer>(sizeof(TransformationMatrices), resourceDesc);

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

    m_gameObjects.clear();
    m_gameObjects.reserve(100);

    m_sceneCamera = std::make_unique<Camera>(
        Vector3(10.0f, 5.0f, -10.0f),
        Vector3(0.0f, 0.0f, 0.0f)
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

    m_fogDesc.enabled = false;
    m_fogDesc.color = Vector4(0.2f, 0.3f, 0.4f, 1.0f);
    m_fogDesc.start = 10.0f;
    m_fogDesc.end = 50.0f;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    HWND hwnd = m_display->getWindowHandle();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, d3dContext);

    device->Release();

    m_gameCamera = std::make_shared<CameraObject>(
        Vector3(15.0f, 10.0f, -15.0f),
        Vector3(0.0f, 0.0f, 0.0f)
    );
    m_gameCamera->getCamera().lookAt(Vector3(0.0f, 2.0f, 0.0f));
    m_gameObjects.push_back(m_gameCamera);

    DX3DLogInfo("Empty scene initialized - use GameObjects menu to add objects!");
}

void dx3d::Game::processInput(float deltaTime)
{
    auto& input = Input::getInstance();

    // Undo/Redo shortcuts (only in edit mode)
    if (m_sceneStateManager->isEditMode())
    {
        if (input.isKeyJustPressedWithShiftCtrl(KeyCode::Z))
        {
            // Redo: Shift + Ctrl + Z
            if (m_undoRedoSystem->canRedo())
            {
                m_undoRedoSystem->redo();
                DX3DLogInfo("Redo action performed");
            }
        }
        else if (input.isKeyJustPressedWithCtrl(KeyCode::Z))
        {
            // Undo: Ctrl + Z
            if (m_undoRedoSystem->canUndo())
            {
                m_undoRedoSystem->undo();
                DX3DLogInfo("Undo action performed");
            }
        }

        // Delete selected object
        if (input.isKeyJustPressed(KeyCode::Delete))
        {
            auto selectedObject = m_selectionSystem->getSelectedObject();
            if (selectedObject)
            {
                // Create delete action and execute it through undo system
                auto deleteAction = std::make_unique<DeleteAction>(selectedObject, m_gameObjects);
                m_undoRedoSystem->executeAction(std::move(deleteAction));

                // Clear selection since object is deleted
                m_selectionSystem->setSelectedObject(nullptr);
                DX3DLogInfo("Deleted selected object");
            }
        }
    }

    if (input.isKeyJustPressed(KeyCode::F5))
    {
        if (m_sceneStateManager->isEditMode())
        {
            m_sceneStateManager->transitionToPlay();
        }
        else if (m_sceneStateManager->isPlayMode() || m_sceneStateManager->isPauseMode())
        {
            m_sceneStateManager->transitionToEdit();
        }
    }

    if (input.isKeyJustPressed(KeyCode::Space) && m_sceneStateManager->isPlayMode())
    {
        m_sceneStateManager->transitionToPause();
    }

    if (input.isKeyJustPressed(KeyCode::F10) && m_sceneStateManager->isPauseMode())
    {
        m_sceneStateManager->frameStep();
    }

    if (input.isKeyJustPressed(KeyCode::F5) && m_sceneStateManager->isPauseMode())
    {
        m_sceneStateManager->transitionToPlay();
    }

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

    // Object selection only in edit mode
    if (m_sceneStateManager->isEditMode())
    {
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
    }


    // Create new cube (only in edit mode)
    if (input.isKeyJustPressed(KeyCode::Space) && m_sceneStateManager->isEditMode())
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> posX(-5.0f, 5.0f);
        std::uniform_real_distribution<float> posZ(-5.0f, 5.0f);
        std::uniform_real_distribution<float> scale(0.5f, 2.0f);

        Vector3 position(posX(gen), 15.0f, posZ(gen));
        Vector3 cubeScale(scale(gen), scale(gen), scale(gen));

        auto cube = std::make_shared<Cube>(position, Vector3(0, 0, 0), cubeScale);
        cube->enablePhysics(PhysicsBodyType::Dynamic);
        cube->setPhysicsMass(1.0f);
        cube->setPhysicsRestitution(0.5f);
        cube->setPhysicsFriction(0.5f);

        // Create through undo system
        auto createAction = std::make_unique<CreateAction>(cube, m_gameObjects);
        m_undoRedoSystem->executeAction(std::move(createAction));
        DX3DLogInfo("Added new physics cube!");
    }

    if (input.isKeyJustPressed(KeyCode::F))
    {
        auto selectedObject = m_selectionSystem->getSelectedObject();
        if (selectedObject && selectedObject->hasPhysics())
        {
            selectedObject->applyImpulse(Vector3(0.0f, 10.0f, 0.0f));
            DX3DLogInfo("Applied upward impulse to selected object!");
        }
    }

    if (input.isKeyJustPressed(KeyCode::R))
    {
        m_sceneCamera->setPosition(Vector3(20.0f, 15.0f, -20.0f));
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

    m_sceneStateManager->update(m_deltaTime);

    processInput(m_deltaTime);

    m_sceneCamera->update();

    if (m_sceneStateManager->isPlayMode())
    {
        m_fpsController->update(m_deltaTime);
    }

    updatePhysics(m_deltaTime);

    for (auto& gameObject : m_gameObjects)
    {
        gameObject->update(m_deltaTime);
    }

    static float debugTimer = 0.0f;
    debugTimer += m_deltaTime;
    if (debugTimer >= 5.0f)
    {
        DX3DLogInfo(("Physics demo running - " + std::to_string(m_gameObjects.size()) + " objects").c_str());
        debugTimer = 0.0f;
    }
}

void dx3d::Game::onSceneStateChanged(SceneState oldState, SceneState newState)
{
    switch (newState)
    {
    case SceneState::Edit:
        if (oldState == SceneState::Play || oldState == SceneState::Pause)
        {
            m_sceneStateManager->restoreObjectStates(m_gameObjects);
        }
        m_fpsController->disable();
        m_fpsController->lockCursor(false);
        m_physicsUpdateEnabled = false;
        ShowCursor(TRUE);
        break;

    case SceneState::Play:
        if (oldState == SceneState::Edit)
        {
            m_sceneStateManager->saveObjectStates(m_gameObjects);
        }
        m_fpsController->enable();
        m_fpsController->lockCursor(false);
        m_physicsUpdateEnabled = true;
        ShowCursor(TRUE);
        break;

    case SceneState::Pause:
        m_fpsController->lockCursor(false);
        m_physicsUpdateEnabled = false;
        ShowCursor(TRUE);
        break;
    }
}

void dx3d::Game::updatePhysics(float deltaTime)
{
    if (!m_physicsUpdateEnabled)
    {
        // Handle frame step in pause mode
        if (m_sceneStateManager->isPauseMode() && m_sceneStateManager->isFrameStepRequested())
        {
            PhysicsSystem::getInstance().update(1.0f / 60.0f);
            // Clear the frame step request after physics update
            m_sceneStateManager->clearFrameStepRequest();
        }
        return;
    }

    if (m_sceneStateManager->isPlayMode())
    {
        PhysicsSystem::getInstance().update(deltaTime);
    }
}

void dx3d::Game::renderScene(Camera& camera, const Matrix4x4& projMatrix, RenderTexture* renderTarget)
{
    auto& renderSystem = m_graphicsEngine->getRenderSystem();
    auto& deviceContext = renderSystem.getDeviceContext();
    auto d3dContext = deviceContext.getDeviceContext();

    if (renderTarget)
    {
        renderTarget->clear(deviceContext, 0.1f, 0.1f, 0.2f, 1.0f);
        renderTarget->setAsRenderTarget(deviceContext);
    }
    else
    {
        auto& swapChain = m_display->getSwapChain();
        deviceContext.clearRenderTargetColor(swapChain, 0.1f, 0.1f, 0.2f, 1.0f);
        deviceContext.clearDepthBuffer(*m_depthBuffer);
        deviceContext.setRenderTargetsWithDepth(swapChain, *m_depthBuffer);
    }

    ui32 viewportWidth = renderTarget ? 640 : m_display->getSize().width;
    ui32 viewportHeight = renderTarget ? 480 : m_display->getSize().height;
    deviceContext.setViewportSize(viewportWidth, viewportHeight);

    ID3D11Buffer* transformCb = m_transformConstantBuffer->getBuffer();
    d3dContext->VSSetConstantBuffers(0, 1, &transformCb);
    d3dContext->OMSetDepthStencilState(m_solidDepthState, 0);

    bool isSceneView = (&camera == m_sceneCamera.get());

    for (const auto& gameObject : m_gameObjects)
    {
        bool isCamera = std::dynamic_pointer_cast<CameraObject>(gameObject) != nullptr;
        if (!isSceneView && isCamera)
            continue;

        deviceContext.setVertexShader(m_fogVertexShader->getShader());
        deviceContext.setPixelShader(m_fogPixelShader->getShader());
        deviceContext.setInputLayout(m_fogVertexShader->getInputLayout());

        FogShaderConstants fsc = {};
        fsc.fogColor = m_fogDesc.color;
        fsc.cameraPosition = camera.getPosition();
        fsc.fogStart = m_fogDesc.start;
        fsc.fogEnd = m_fogDesc.end;
        fsc.fogEnabled = m_fogDesc.enabled;
        m_fogConstantBuffer->update(deviceContext, &fsc);

        ID3D11Buffer* fogCb = m_fogConstantBuffer->getBuffer();
        d3dContext->PSSetConstantBuffers(1, 1, &fogCb);

        FogMaterialConstants fmc = {};
        fmc.useVertexColor = true;
        fmc.baseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        m_materialConstantBuffer->update(deviceContext, &fmc);
        ID3D11Buffer* materialCb = m_materialConstantBuffer->getBuffer();
        d3dContext->PSSetConstantBuffers(2, 1, &materialCb);

        bool bufferSet = false;
        ui32 indexCount = 0;

        if (auto cube = std::dynamic_pointer_cast<Cube>(gameObject))
        {
            deviceContext.setVertexBuffer(*m_cubeVertexBuffer);
            deviceContext.setIndexBuffer(*m_cubeIndexBuffer);
            indexCount = Cube::GetIndexCount();
            bufferSet = true;
        }
        else if (auto plane = std::dynamic_pointer_cast<Plane>(gameObject))
        {
            deviceContext.setVertexBuffer(*m_planeVertexBuffer);
            deviceContext.setIndexBuffer(*m_planeIndexBuffer);
            indexCount = Plane::GetIndexCount();
            bufferSet = true;
        }
        else if (auto sphere = std::dynamic_pointer_cast<Sphere>(gameObject))
        {
            deviceContext.setVertexBuffer(*m_sphereVertexBuffer);
            deviceContext.setIndexBuffer(*m_sphereIndexBuffer);
            indexCount = Sphere::GetIndexCount();
            bufferSet = true;
        }
        else if (auto cylinder = std::dynamic_pointer_cast<Cylinder>(gameObject))
        {
            deviceContext.setVertexBuffer(*m_cylinderVertexBuffer);
            deviceContext.setIndexBuffer(*m_cylinderIndexBuffer);
            indexCount = Cylinder::GetIndexCount();
            bufferSet = true;
        }
        else if (auto capsule = std::dynamic_pointer_cast<Capsule>(gameObject))
        {
            deviceContext.setVertexBuffer(*m_capsuleVertexBuffer);
            deviceContext.setIndexBuffer(*m_capsuleIndexBuffer);
            indexCount = Capsule::GetIndexCount();
            bufferSet = true;
        }

        if (bufferSet)
        {
            TransformationMatrices transformMatrices;
            transformMatrices.world = Matrix4x4::fromXMMatrix(DirectX::XMMatrixTranspose(gameObject->getWorldMatrix().toXMMatrix()));
            transformMatrices.view = Matrix4x4::fromXMMatrix(DirectX::XMMatrixTranspose(camera.getViewMatrix().toXMMatrix()));
            transformMatrices.projection = Matrix4x4::fromXMMatrix(DirectX::XMMatrixTranspose(projMatrix.toXMMatrix()));
            m_transformConstantBuffer->update(deviceContext, &transformMatrices);

            deviceContext.drawIndexed(indexCount, 0, 0);
        }
    }
}

void dx3d::Game::render()
{
    auto& renderSystem = m_graphicsEngine->getRenderSystem();
    auto& deviceContext = renderSystem.getDeviceContext();
    auto& swapChain = m_display->getSwapChain();

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

    UIManager::SpawnCallbacks spawnCallbacks{
    [this]() { spawnCube(); },
    [this]() { spawnSphere(); },
    [this]() { spawnCapsule(); },
    [this]() { spawnCylinder(); },
    [this]() { spawnPlane(); },
    [this](const std::string& filename) { spawnModel(filename); },
    [this]() { spawnCubeDemo(); }
    };

    m_uiManager->render(m_deltaTime, spawnCallbacks);

    //renderUI();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    deviceContext.present(swapChain);
}

void dx3d::Game::alignGameCameraWithView()
{
    if (m_gameCamera)
    {
        m_gameCamera->alignWithView(*m_sceneCamera);
        DX3DLogInfo("Game camera aligned with scene view");
    }
}

void dx3d::Game::spawnCubeDemo()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posX(1.0f, 5.0f);
    std::uniform_real_distribution<float> posZ(1.0f, 5.0f);
    std::uniform_real_distribution<float> posY(13.0f, 15.0f);

    const int numCubes = 25;
    for (int i = 0; i < numCubes; ++i)
    {
        Vector3 position(posX(gen), posY(gen), posZ(gen));
        Vector3 cubeScale(2.0f, 2.0f, 2.0f);

        auto cube = std::make_shared<Cube>(position, Vector3(0, 0, 0), cubeScale);

        cube->enablePhysics(PhysicsBodyType::Dynamic);
        cube->setPhysicsMass(2.0f);
        cube->setPhysicsRestitution(0.8f);
        cube->setPhysicsFriction(0.8f);

        m_gameObjects.push_back(cube);
    }

    DX3DLogInfo(("Spawned a new Cube Demo with " + std::to_string(numCubes) + " cubes").c_str());
}

void dx3d::Game::spawnCube()
{
    Vector3 position(0.0f, 5.0f, 0.0f);
    Vector3 scale(1.0f, 1.0f, 1.0f);

    auto cube = std::make_shared<Cube>(position, Vector3(0, 0, 0), scale);
    cube->enablePhysics(PhysicsBodyType::Dynamic);
    cube->setPhysicsMass(1.0f);
    cube->setPhysicsRestitution(0.5f);
    cube->setPhysicsFriction(0.5f);

    auto createAction = std::make_unique<CreateAction>(cube, m_gameObjects);
    m_undoRedoSystem->executeAction(std::move(createAction));

    m_selectionSystem->setSelectedObject(cube);
    DX3DLogInfo("Spawned Cube");
}

void dx3d::Game::spawnSphere()
{
    Vector3 position(0.0f, 5.0f, 0.0f);
    Vector3 scale(1.0f, 1.0f, 1.0f);

    auto sphere = std::make_shared<Sphere>(position, Vector3(0, 0, 0), scale);
    sphere->enablePhysics(PhysicsBodyType::Dynamic);
    sphere->setPhysicsMass(1.0f);
    sphere->setPhysicsRestitution(0.7f);
    sphere->setPhysicsFriction(0.3f);

    auto createAction = std::make_unique<CreateAction>(sphere, m_gameObjects);
    m_undoRedoSystem->executeAction(std::move(createAction));

    m_selectionSystem->setSelectedObject(sphere);
    DX3DLogInfo("Spawned Sphere");
}

void dx3d::Game::spawnCapsule()
{
    Vector3 position(0.0f, 5.0f, 0.0f);
    Vector3 scale(1.0f, 1.0f, 1.0f);

    auto capsule = std::make_shared<Capsule>(position, Vector3(0, 0, 0), scale);
    capsule->enablePhysics(PhysicsBodyType::Dynamic);
    capsule->setPhysicsMass(1.0f);
    capsule->setPhysicsRestitution(0.4f);
    capsule->setPhysicsFriction(0.6f);

    auto createAction = std::make_unique<CreateAction>(capsule, m_gameObjects);
    m_undoRedoSystem->executeAction(std::move(createAction));

    m_selectionSystem->setSelectedObject(capsule);
    DX3DLogInfo("Spawned Capsule");
}

void dx3d::Game::spawnCylinder()
{
    Vector3 position(0.0f, 5.0f, 0.0f);
    Vector3 scale(1.0f, 1.0f, 1.0f);

    auto cylinder = std::make_shared<Cylinder>(position, Vector3(0, 0, 0), scale);
    cylinder->enablePhysics(PhysicsBodyType::Dynamic);
    cylinder->setPhysicsMass(1.0f);
    cylinder->setPhysicsRestitution(0.3f);
    cylinder->setPhysicsFriction(0.7f);

    auto createAction = std::make_unique<CreateAction>(cylinder, m_gameObjects);
    m_undoRedoSystem->executeAction(std::move(createAction));

    m_selectionSystem->setSelectedObject(cylinder);
    DX3DLogInfo("Spawned Cylinder");
}

void dx3d::Game::spawnModel(const std::string& filename)
{
    try
    {
        auto& renderSystem = m_graphicsEngine->getRenderSystem();
        auto resourceDesc = renderSystem.getGraphicsResourceDesc();

        auto model = Model::LoadFromFile(filename, resourceDesc);

        if (model && model->isReadyForRendering())
        {
            Vector3 position(0.0f, 5.0f, 0.0f);
            Vector3 scale(1.0f, 1.0f, 1.0f);

            model->setPosition(position);
            model->setScale(scale);
            model->enablePhysics(PhysicsBodyType::Dynamic);
            model->setPhysicsMass(1.0f);
            model->setPhysicsRestitution(0.3f);
            model->setPhysicsFriction(0.6f);

            auto createAction = std::make_unique<CreateAction>(model, m_gameObjects);
            m_undoRedoSystem->executeAction(std::move(createAction));

            m_selectionSystem->setSelectedObject(model);
            DX3DLogInfo(("Spawned Model: " + filename).c_str());
        }
        else
        {
            DX3DLogError(("Failed to load model: " + filename).c_str());
        }
    }
    catch (const std::exception& e)
    {
        DX3DLogError(("Error loading model " + filename + ": " + e.what()).c_str());
    }
}

void dx3d::Game::spawnPlane()
{
    Vector3 position(0.0f, 0.0f, 0.0f);
    Vector3 scale(10.0f, 1.0f, 10.0f);

    auto plane = std::make_shared<Plane>(position, Vector3(0, 0, 0), scale);
    plane->enablePhysics(PhysicsBodyType::Static);
    plane->setPhysicsRestitution(0.0f);
    plane->setPhysicsFriction(0.7f);

    auto createAction = std::make_unique<CreateAction>(plane, m_gameObjects);
    m_undoRedoSystem->executeAction(std::move(createAction));

    m_selectionSystem->setSelectedObject(plane);
    DX3DLogInfo("Spawned Plane");
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