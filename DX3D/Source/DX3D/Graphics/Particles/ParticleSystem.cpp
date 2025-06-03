#include <DX3D/Graphics/Particles/ParticleSystem.h>
#include <DX3D/Graphics/Shaders/ParticleShader.h>
#include <DX3D/Graphics/DeviceContext.h>
#include <algorithm>
#include <cstdio>  // for sprintf_s

using namespace dx3d;

ParticleSystem::ParticleSystem(const GraphicsResourceDesc& resourceDesc)
    : Base(resourceDesc.base)
    , m_resourceDesc(resourceDesc)
{
    // Reserve space for vertices
    m_vertices.reserve(10000);  // Can handle up to ~3333 particles (3 vertices each for triangles)

    // Create shaders for particle rendering
    m_particleVertexShader = std::make_shared<VertexShader>(
        m_resourceDesc,
        ParticleShader::GetVertexShaderCode()
    );

    m_particlePixelShader = std::make_shared<PixelShader>(
        m_resourceDesc,
        ParticleShader::GetPixelShaderCode()
    );

    // New: Create and configure alpha blend state for particles
    D3D11_BLEND_DESC blendDesc{};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;

    blendDesc.RenderTarget[0].BlendEnable = TRUE; // Enable blending for the first render target
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; // Source blend factor: (As, As, As, As)
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // Destination blend factor: (1-As, 1-As, 1-As, 1-As)
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; // Add source and destination
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE; // Source alpha blend factor
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO; // Destination alpha blend factor (keep existing alpha)
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD; // Add alpha
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // Write all color channels

    DX3DGraphicsLogErrorAndThrow(m_resourceDesc.device.CreateBlendState(&blendDesc, &m_alphaBlendState),
        "Failed to create alpha blend state for particles.");

    // Create initial vertex buffer
    m_vertexBuffer = createParticleVertexBuffer();
}

ParticleSystem::~ParticleSystem()
{
    clearEmitters();
}

void ParticleSystem::update(float deltaTime)
{
    // Update all emitters
    for (auto& emitter : m_emitters)
    {
        if (emitter)
        {
            emitter->update(deltaTime);
        }
    }
}

void ParticleSystem::render(DeviceContext& deviceContext)
{
    // Clear vertex data from the previous frame
    m_vertices.clear();

    // Collect vertices from all particles in all emitters
    for (const auto& emitter : m_emitters)
    {
        if (!emitter)
            continue;

        const auto& particles = emitter->getParticles();

        for (const auto& particle : particles)
        {
            if (!particle || !particle->isActive())
                continue;

            // Get particle properties
            Vec2 pos = particle->getPosition();
            float size = particle->getSize();
            Vec4 color = particle->getColor();
            // The alpha channel is already handled by Particle::update and the ParticleShader,
            // so we keep it as is.
            float rotation = particle->getRotation();

            // Create a quad using two triangles
            // Calculate rotated corners
            float cos_r = std::cos(rotation);
            float sin_r = std::sin(rotation);

            // Define quad corners before rotation (relative to center)
            Vec2 corners[4] = {
                Vec2(-size * 0.5f, -size * 0.5f),  // Bottom-left
                Vec2(size * 0.5f, -size * 0.5f),   // Bottom-right
                Vec2(-size * 0.5f, size * 0.5f),   // Top-left
                Vec2(size * 0.5f, size * 0.5f)     // Top-right
            };

            // Rotate and translate corners
            Vec2 rotatedCorners[4];
            for (int i = 0; i < 4; ++i)
            {
                float x = corners[i].x * cos_r - corners[i].y * sin_r;
                float y = corners[i].x * sin_r + corners[i].y * cos_r;
                rotatedCorners[i] = Vec2(pos.x + x, pos.y + y);
            }

            // First triangle (bottom-left, bottom-right, top-left)
            m_vertices.push_back({
                {rotatedCorners[0].x, rotatedCorners[0].y, 0.0f},
                {color.x, color.y, color.z, color.w}
                });
            m_vertices.push_back({
                {rotatedCorners[1].x, rotatedCorners[1].y, 0.0f},
                {color.x, color.y, color.z, color.w}
                });
            m_vertices.push_back({
                {rotatedCorners[2].x, rotatedCorners[2].y, 0.0f},
                {color.x, color.y, color.z, color.w}
                });

            // Second triangle (bottom-right, top-right, top-left)
            m_vertices.push_back({
                {rotatedCorners[1].x, rotatedCorners[1].y, 0.0f},
                {color.x, color.y, color.z, color.w}
                });
            m_vertices.push_back({
                {rotatedCorners[3].x, rotatedCorners[3].y, 0.0f},
                {color.x, color.y, color.z, color.w}
                });
            m_vertices.push_back({
                {rotatedCorners[2].x, rotatedCorners[2].y, 0.0f},
                {color.x, color.y, color.z, color.w}
                });
        }
    }

    // Only render if we have particles
    if (m_vertices.empty())
        return;

    // The problematic m_vertices.clear() that discarded particle data is removed.
    // The brute-forced red quad vertices are also removed.

    // Debug log
    static int frameCount = 0;
    if (frameCount++ % 60 == 0) // Every 60 frames
    {
        char debugMsg[256];
        sprintf_s(debugMsg, "Rendering %zu vertices (%zu particles)",
            m_vertices.size(), m_vertices.size() / 6);
        getLogger().log(Logger::LogLevel::Info, debugMsg);
    }

    // Update vertex buffer
    updateVertexBuffer();

    // Set up rendering state
    deviceContext.setVertexBuffer(*m_vertexBuffer);
    deviceContext.setVertexShader(m_particleVertexShader->getShader());
    deviceContext.setPixelShader(m_particlePixelShader->getShader());
    deviceContext.setInputLayout(m_particleVertexShader->getInputLayout());

    // Set the alpha blending state
    deviceContext.getDeviceContext()->OMSetBlendState(m_alphaBlendState.Get(), nullptr, 0xffffffff);
    deviceContext.getDeviceContext()->OMSetDepthStencilState(nullptr, 0);

    // Draw all particles
    deviceContext.drawTriangleList(static_cast<ui32>(m_vertices.size()), 0);
}

void ParticleSystem::addEmitter(std::shared_ptr<ParticleEmitter> emitter)
{
    if (emitter)
    {
        m_emitters.push_back(emitter);
    }
}

void ParticleSystem::removeEmitter(std::shared_ptr<ParticleEmitter> emitter)
{
    m_emitters.erase(
        std::remove(m_emitters.begin(), m_emitters.end(), emitter),
        m_emitters.end()
    );
}

void ParticleSystem::clearEmitters()
{
    m_emitters.clear();
}

std::shared_ptr<VertexBuffer> ParticleSystem::createParticleVertexBuffer()
{
    // Create a dummy vertex buffer that we'll update later
    Vertex dummyVertices[6] = {};  // Two triangles

    return std::make_shared<VertexBuffer>(
        dummyVertices,
        sizeof(Vertex),
        6,
        m_resourceDesc
    );
}

void ParticleSystem::updateVertexBuffer()
{
    if (m_vertices.empty())
        return;

    // Recreate vertex buffer with current particle data
    m_vertexBuffer = std::make_shared<VertexBuffer>(
        m_vertices.data(),
        sizeof(Vertex),
        static_cast<ui32>(m_vertices.size()),
        m_resourceDesc
    );
}