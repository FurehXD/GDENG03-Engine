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


// Forward declarations
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
        void createRenderingResources();
        void update();
        void processInput(float deltaTime);
        void updateSnowEmitter();
        void renderCreditsScreen();

    private:
        std::unique_ptr<Logger> m_loggerPtr{};
        std::unique_ptr<GraphicsEngine> m_graphicsEngine{};
        std::unique_ptr<Display> m_display{};
        bool m_isRunning{ true };
        bool m_showCreditsScreen{ false };

        // Camera
        std::unique_ptr<Camera> m_camera{};
        float m_cameraSpeed{ 5.0f };  // Units per second
        float m_mouseSensitivity{ 0.3f };
        float m_cubeRotationSpeed{ 3.0f };

        // --- 3D Object Rendering Resources ---
        std::vector<std::shared_ptr<AGameObject>> m_gameObjects;
        std::vector<Vector3> m_objectRotationDeltas;

        // Vertex and Index buffers for each primitive type
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

        // Rainbow shader for 3D objects
        std::shared_ptr<VertexShader> m_rainbowVertexShader;
        std::shared_ptr<PixelShader> m_rainbowPixelShader;

        // White shader for planes
        std::shared_ptr<VertexShader> m_whiteVertexShader;
        std::shared_ptr<PixelShader> m_whitePixelShader;

        // Fog shader for everything else.
        std::shared_ptr<VertexShader> m_fogVertexShader;
        std::shared_ptr<PixelShader> m_fogPixelShader;
        std::shared_ptr<ConstantBuffer> m_fogConstantBuffer;
        std::shared_ptr<ConstantBuffer> m_materialConstantBuffer;
        FogDesc m_fogDesc;

        SnowConfig m_snowConfig;

        // Constant buffer for transformation matrices
        std::shared_ptr<ConstantBuffer> m_transformConstantBuffer;

        // Depth buffer for proper 3D depth testing
        std::shared_ptr<DepthBuffer> m_depthBuffer;

        // FIX: Declare the depth stencil states as member variables
        ID3D11DepthStencilState* m_solidDepthState = nullptr;
        ID3D11DepthStencilState* m_particleDepthState = nullptr;

        // Projection matrix (view matrix now comes from Camera)
        Matrix4x4 m_projectionMatrix;

        // Animation variables
        std::chrono::steady_clock::time_point m_previousTime;
        float m_deltaTime{ 0.0f };
    };
}