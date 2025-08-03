#pragma once
#include <DX3D/Core/Core.h>
#include <DX3D/Particles/ParticleEmitter.h>
#include <DX3D/Graphics/VertexBuffer.h>
#include <DX3D/Graphics/IndexBuffer.h>
#include <DX3D/Graphics/ConstantBuffer.h>
#include <DX3D/Graphics/Shaders/Shaders.h>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <wrl.h>

namespace dx3d
{
    class GraphicsEngine;
    class DeviceContext;
    class SceneCamera;

    class ParticleSystem
    {
    public:
        static ParticleSystem& getInstance()
        {
            static ParticleSystem instance;
            return instance;
        }

        void initialize(GraphicsEngine& graphicsEngine);
        void shutdown();
        // Update all emitters
        void update(float deltaTime);
        // Render all particles
        void render(DeviceContext& deviceContext, const SceneCamera& camera, const Matrix4x4& projectionMatrix);

        // Create and manage emitters
        std::shared_ptr<ParticleEmitter> createEmitter(
            const std::string& name,
            const ParticleEmitter::EmitterConfig& config,
            ParticleEmitter::ParticleFactory factory
        );
        void removeEmitter(const std::string& name);
        std::shared_ptr<ParticleEmitter> getEmitter(const std::string& name);
        

        // Set blend mode for particles
        enum class BlendMode
        {
            Additive,
            Alpha,
            Opaque
        };
        void setBlendMode(BlendMode mode) { m_blendMode = mode; }

    private:
        ParticleSystem() = default;
        ~ParticleSystem() = default;
        ParticleSystem(const ParticleSystem&) = delete;
        ParticleSystem& operator=(const ParticleSystem&) = delete;

        void createRenderingResources(GraphicsEngine& graphicsEngine);
        void updateInstanceBuffer(DeviceContext& deviceContext, const std::vector<ParticleInstanceData>& instanceData);

    private:
        std::unordered_map<std::string, std::shared_ptr<ParticleEmitter>> m_emitters;

        // Rendering resources
        std::shared_ptr<VertexBuffer> m_quadVertexBuffer;
        std::shared_ptr<IndexBuffer> m_quadIndexBuffer;
        std::shared_ptr<PixelShader> m_pixelShader;
        std::shared_ptr<ConstantBuffer> m_viewProjConstantBuffer;

        // Raw shader pointers for particle rendering
        Microsoft::WRL::ComPtr<ID3D11VertexShader> m_particleVertexShader;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_particleInputLayout;

        // Instance buffer for instanced rendering
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_instanceBuffer;
        ui32 m_instanceBufferCapacity;

        // Texture for particles (future enhancement)
        // std::shared_ptr<Texture2D> m_particleTexture;

        BlendMode m_blendMode;
        bool m_initialized;
    };
}