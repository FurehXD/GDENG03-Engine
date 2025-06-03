#pragma once
#include <DX3D/Graphics/Particles/ParticleEmitter.h>
#include <DX3D/Graphics/VertexBuffer.h>
#include <DX3D/Graphics/GraphicsResource.h>
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/Shaders/Shaders.h>
#include <memory>
#include <vector>

namespace dx3d
{
    class ParticleSystem : public Base
    {
    public:
        ParticleSystem(const GraphicsResourceDesc& resourceDesc);
        ~ParticleSystem();

        // Update all emitters
        void update(float deltaTime);

        // Render all particles
        void render(DeviceContext& deviceContext);

        // Add emitters
        void addEmitter(std::shared_ptr<ParticleEmitter> emitter);
        void removeEmitter(std::shared_ptr<ParticleEmitter> emitter);
        void clearEmitters();

        // Get emitters
        const std::vector<std::shared_ptr<ParticleEmitter>>& getEmitters() const { return m_emitters; }

    private:
        // Create vertex buffer for particles
        std::shared_ptr<VertexBuffer> createParticleVertexBuffer();

        // Update vertex buffer with current particle data
        void updateVertexBuffer();

    private:
        GraphicsResourceDesc m_resourceDesc;
        std::vector<std::shared_ptr<ParticleEmitter>> m_emitters;
        std::shared_ptr<VertexBuffer> m_vertexBuffer;
        std::vector<Vertex> m_vertices;

        // Shaders for particle rendering
        std::shared_ptr<VertexShader> m_particleVertexShader;
        std::shared_ptr<PixelShader> m_particlePixelShader;

        // New: Alpha blend state for particles
        Microsoft::WRL::ComPtr<ID3D11BlendState> m_alphaBlendState{};
    };
}