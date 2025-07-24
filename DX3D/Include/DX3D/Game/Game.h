#pragma once
#include <DX3D/Core/Base.h>
#include <DX3D/Core/Core.h>
#include <DX3D/Math/Math.h>
#include <chrono>
#include <memory>
#include <vector>
#include <d3d11.h> 
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <DX3D/Graphics/Shaders/ModelVertexShader.h>


namespace dx3d
{
    class VertexBuffer;
    class IndexBuffer;
    class ConstantBuffer;
    class VertexShader;
    class PixelShader;
    class AGameObject;
    class Cube;
    class Plane;
    class Sphere;
    class Cylinder;
    class Capsule;
    class DepthBuffer;
    class Camera;
    class FogShader;
    class CameraObject;
    class ViewportManager;
    class SelectionSystem;
}

namespace dx3d {
    struct FogDesc
    {
        bool enabled = true;
        Vector4 color = { 0.2f, 0.3f, 0.4f, 1.0f };
        f32 start = 5.0f;
        f32 end = 25.0f;
    };

    struct SnowConfig
    {
        Vector3 position{ 0.0f, 10.0f, 0.0f };
        Vector3 positionVariance{ 20.0f, 0.0f, 20.0f };
        Vector3 velocity{ 0.0f, -2.0f, 0.0f };
        Vector3 velocityVariance{ 0.5f, 0.5f, 0.5f };
        Vector3 acceleration{ 0.0f, -0.5f, 0.0f };
        Vector4 startColor{ 1.0f, 1.0f, 1.0f, 0.8f };
        Vector4 endColor{ 0.9f, 0.9f, 1.0f, 0.0f };
        float startSize = 0.2f;
        float endSize = 0.1f;
        float lifetime = 8.0f;
        float lifetimeVariance = 2.0f;
        float emissionRate = 50.0f;
        bool active = true;
    };
}

namespace dx3d
{
    class Game : public Base
    {
    public:
        explicit Game(const GameDesc& desc);
        virtual ~Game() override;

        virtual void run() final;
    private:
        void render();
        void renderScene(Camera& camera, const Matrix4x4& projMatrix, RenderTexture* renderTarget = nullptr);
        void renderUI();
        void createRenderingResources();
        void update();
        void processInput(float deltaTime);
        void updateSnowEmitter();
        void debugRenderInfo();
        void renderSceneHierarchy();        
        void renderInspector();
        void spawnCubeDemo();

        std::string getObjectDisplayName(std::shared_ptr<AGameObject> object, int index);
        std::string getObjectIcon(std::shared_ptr<AGameObject> object);
        std::shared_ptr<AGameObject> createObjectCopy(std::shared_ptr<AGameObject> original);

    private:
        std::unique_ptr<Logger> m_loggerPtr{};
        std::unique_ptr<GraphicsEngine> m_graphicsEngine{};
        std::unique_ptr<Display> m_display{};
        bool m_isRunning{ true };

        std::unique_ptr<Camera> m_sceneCamera{};
        std::shared_ptr<CameraObject> m_gameCamera{};
        std::unique_ptr<ViewportManager> m_viewportManager{};
        std::unique_ptr<SelectionSystem> m_selectionSystem{};

        float m_cameraSpeed{ 5.0f };
        float m_mouseSensitivity{ 0.3f };
        float m_cubeRotationSpeed{ 3.0f };

        std::vector<std::shared_ptr<AGameObject>> m_gameObjects;
        std::vector<Vector3> m_objectRotationDeltas;

        std::shared_ptr<VertexBuffer> m_cubeVertexBuffer;
        std::shared_ptr<IndexBuffer> m_cubeIndexBuffer;
        std::shared_ptr<VertexBuffer> m_planeVertexBuffer;
        std::shared_ptr<IndexBuffer> m_planeIndexBuffer;
        std::shared_ptr<VertexBuffer> m_sphereVertexBuffer;
        std::shared_ptr<IndexBuffer> m_sphereIndexBuffer;
        std::shared_ptr<VertexBuffer> m_cylinderVertexBuffer;
        std::shared_ptr<IndexBuffer> m_cylinderIndexBuffer;
        std::shared_ptr<VertexBuffer> m_capsuleVertexBuffer;
        std::shared_ptr<IndexBuffer> m_capsuleIndexBuffer;
        std::shared_ptr<VertexBuffer> m_cameraGizmoVertexBuffer;
        std::shared_ptr<IndexBuffer> m_cameraGizmoIndexBuffer;

        std::shared_ptr<VertexShader> m_rainbowVertexShader;
        std::shared_ptr<PixelShader> m_rainbowPixelShader;
        std::shared_ptr<VertexShader> m_whiteVertexShader;
        std::shared_ptr<PixelShader> m_whitePixelShader;
        std::shared_ptr<VertexShader> m_fogVertexShader;
        std::shared_ptr<PixelShader> m_fogPixelShader;
        std::shared_ptr<ConstantBuffer> m_fogConstantBuffer;
        std::shared_ptr<ConstantBuffer> m_materialConstantBuffer;
        std::shared_ptr<ConstantBuffer> m_modelMaterialConstantBuffer;

        std::shared_ptr<VertexShader> m_modelVertexShader;
        std::shared_ptr<PixelShader> m_modelPixelShader;

        SnowConfig m_snowConfig;
        FogDesc m_fogDesc;

        std::shared_ptr<ConstantBuffer> m_transformConstantBuffer;
        std::shared_ptr<DepthBuffer> m_depthBuffer;

        ID3D11DepthStencilState* m_solidDepthState = nullptr;
        ID3D11DepthStencilState* m_particleDepthState = nullptr;

        Matrix4x4 m_projectionMatrix;

        std::chrono::steady_clock::time_point m_previousTime;
        float m_deltaTime{ 0.0f };
    };
}