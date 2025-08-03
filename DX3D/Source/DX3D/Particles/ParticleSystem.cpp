#include <DX3D/Particles/ParticleSystem.h>
#include <DX3D/Particles/ParticleEmitter.h>
#include <DX3D/Graphics/GraphicsEngine.h>
#include <DX3D/Graphics/RenderSystem.h>
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/Shaders/ParticleShader.h>
#include <DX3D/Game/SceneCamera.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

using namespace dx3d;

// Constant buffer structure for view/projection matrices and camera vectors
struct ParticleConstantBuffer
{
    Matrix4x4 view;
    Matrix4x4 projection;
    Vector3 cameraRight;
    float _pad0;
    Vector3 cameraUp;
    float _pad1;
};

void ParticleSystem::initialize(GraphicsEngine& graphicsEngine)
{
    if (m_initialized)
        return;

    createRenderingResources(graphicsEngine);

    m_blendMode = BlendMode::Alpha;
    m_initialized = true;
}

void ParticleSystem::shutdown()
{
    m_emitters.clear();
    m_quadVertexBuffer.reset();
    m_quadIndexBuffer.reset();
    m_pixelShader.reset();
    m_viewProjConstantBuffer.reset();
    m_particleVertexShader.Reset();
    m_particleInputLayout.Reset();
    m_instanceBuffer.Reset();
    m_initialized = false;
}

std::shared_ptr<ParticleEmitter> ParticleSystem::createEmitter(
    const std::string& name,
    const ParticleEmitter::EmitterConfig& config,
    ParticleEmitter::ParticleFactory factory)
{
    auto emitter = std::make_shared<ParticleEmitter>(config, factory);
    m_emitters[name] = emitter;
    return emitter;
}

void ParticleSystem::removeEmitter(const std::string& name)
{
    m_emitters.erase(name);
}

std::shared_ptr<ParticleEmitter> ParticleSystem::getEmitter(const std::string& name)
{
    auto it = m_emitters.find(name);
    return (it != m_emitters.end()) ? it->second : nullptr;
}

void ParticleSystem::update(float deltaTime)
{
    for (auto& pair : m_emitters)
    {
        pair.second->update(deltaTime);
    }
}

void ParticleSystem::render(DeviceContext& deviceContext, const SceneCamera& camera, const Matrix4x4& projectionMatrix)
{
    if (!m_initialized)
        return;

    // Collect all particle instance data
    std::vector<ParticleInstanceData> allParticles;
    for (const auto& pair : m_emitters)
    {
        pair.second->fillInstanceData(allParticles, camera);
    }

    if (allParticles.empty())
        return;

    // Update instance buffer
    updateInstanceBuffer(deviceContext, allParticles);

    // Set up rendering state
    auto d3dContext = deviceContext.getDeviceContext();

    // Set shaders using raw pointers
    d3dContext->VSSetShader(m_particleVertexShader.Get(), nullptr, 0);
    deviceContext.setPixelShader(m_pixelShader->getShader());
    d3dContext->IASetInputLayout(m_particleInputLayout.Get());

    // Update constant buffer with camera data
    ParticleConstantBuffer cbData;
    cbData.view = Matrix4x4::fromXMMatrix(DirectX::XMMatrixTranspose(camera.getViewMatrix().toXMMatrix()));
    cbData.projection = Matrix4x4::fromXMMatrix(DirectX::XMMatrixTranspose(projectionMatrix.toXMMatrix()));
    cbData.cameraRight = camera.getRight();
    cbData.cameraUp = camera.getUp();

    m_viewProjConstantBuffer->update(deviceContext, &cbData);
    ID3D11Buffer* cb = m_viewProjConstantBuffer->getBuffer();
    d3dContext->VSSetConstantBuffers(0, 1, &cb);

    // Set vertex buffer (quad) and instance buffer
    ID3D11Buffer* buffers[2] = { m_quadVertexBuffer->getBuffer(), m_instanceBuffer.Get() };
    UINT strides[2] = { sizeof(Vertex), sizeof(ParticleInstanceData) };
    UINT offsets[2] = { 0, 0 };
    d3dContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);

    // Set index buffer
    deviceContext.setIndexBuffer(*m_quadIndexBuffer);

    // Draw all particles with instancing
    d3dContext->DrawIndexedInstanced(6, static_cast<UINT>(allParticles.size()), 0, 0, 0);
}

void ParticleSystem::createRenderingResources(GraphicsEngine& graphicsEngine)
{
    auto& renderSystem = graphicsEngine.getRenderSystem();
    auto resourceDesc = renderSystem.getGraphicsResourceDesc();

    // Create a simple quad for particles (centered at origin)
    Vertex quadVertices[] = {
        { {-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f} }, // Bottom-left with UV
        { { 0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 0.0f, 0.0f} }, // Bottom-right with UV
        { { 0.5f,  0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f} }, // Top-right with UV
        { {-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} }  // Top-left with UV
    };

    m_quadVertexBuffer = std::make_shared<VertexBuffer>(
        quadVertices,
        sizeof(Vertex),
        4,
        resourceDesc
    );

    ui32 quadIndices[] = { 0, 2, 1, 0, 3, 2 }; // Corrected for CCW winding
    m_quadIndexBuffer = std::make_shared<IndexBuffer>(
        quadIndices,
        6,
        resourceDesc
    );

    // First, compile the vertex shader manually
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3DCompile(
        ParticleShader::GetVertexShaderCode(),
        strlen(ParticleShader::GetVertexShaderCode()),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "vs_5_0",
        D3DCOMPILE_DEBUG,
        0,
        &vsBlob,
        &errorBlob
    );

    if (FAILED(hr)) {
        if (errorBlob) {
            throw std::runtime_error(static_cast<const char*>(errorBlob->GetBufferPointer()));
        }
        throw std::runtime_error("Failed to compile particle vertex shader");
    }

    // Create vertex shader
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    // FIX: Changed auto to ID3D11Device* to be explicit
    ID3D11Device* device = nullptr;
    renderSystem.getDeviceContext().getDeviceContext()->GetDevice(&device);
    hr = device->CreateVertexShader(
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        nullptr,
        &vertexShader
    );

    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create particle vertex shader");
    }

    // Create input layout for particles with instancing
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        // Per-vertex data
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },

        // Per-instance data
        { "POSITION", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "POSITION", 2, DXGI_FORMAT_R32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "COLOR", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "POSITION", 3, DXGI_FORMAT_R32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
    };

    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    hr = device->CreateInputLayout(
        layout,
        6,
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        &inputLayout
    );

    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create particle input layout");
    }

    // Store the raw pointers for rendering
    m_particleVertexShader = vertexShader;
    m_particleInputLayout = inputLayout;

    // Create pixel shader normally
    // FIX: Pass constructor arguments directly to std::make_shared
    m_pixelShader = std::make_shared<PixelShader>(resourceDesc, ParticleShader::GetPixelShaderCode());

    // Create constant buffer for view/projection matrices
    m_viewProjConstantBuffer = std::make_shared<ConstantBuffer>(
        sizeof(ParticleConstantBuffer),
        resourceDesc
    );

    // Create initial instance buffer
    m_instanceBufferCapacity = 10000;
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = m_instanceBufferCapacity * sizeof(ParticleInstanceData);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = device->CreateBuffer(&bufferDesc, nullptr, &m_instanceBuffer);
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create particle instance buffer");
    }

    // Release the device pointer obtained with GetDevice()
    if (device)
    {
        device->Release();
    }
}

void ParticleSystem::updateInstanceBuffer(DeviceContext& deviceContext, const std::vector<ParticleInstanceData>& instanceData)
{
    auto d3dContext = deviceContext.getDeviceContext();

    // Resize buffer if needed
    if (instanceData.size() > m_instanceBufferCapacity)
    {
        m_instanceBufferCapacity = static_cast<ui32>(instanceData.size() * 1.5f); // Grow by 50%

        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.ByteWidth = m_instanceBufferCapacity * sizeof(ParticleInstanceData);
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        m_instanceBuffer.Reset();

        // FIX: Properly get the ID3D11Device pointer.
        ID3D11Device* device = nullptr;
        d3dContext->GetDevice(&device);

        // Check if the device was successfully obtained before using it.
        if (device)
        {
            HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, &m_instanceBuffer);

            // FIX: Release the device pointer after it's been used to avoid a memory leak.
            device->Release();

            if (FAILED(hr))
            {
                throw std::runtime_error("Failed to resize particle instance buffer");
            }
        }
        else
        {
            throw std::runtime_error("Failed to get D3D11Device from context");
        }
    }

    // Update buffer data
    D3D11_MAPPED_SUBRESOURCE mappedResource{};
    HRESULT hr = d3dContext->Map(m_instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (SUCCEEDED(hr))
    {
        memcpy(mappedResource.pData, instanceData.data(), instanceData.size() * sizeof(ParticleInstanceData));
        d3dContext->Unmap(m_instanceBuffer.Get(), 0);
    }
}