#include <DX3D/Graphics/RenderSystem.h>
#include <DX3D/Graphics/GraphicsLogUtils.h>
#include <DX3D/Graphics/SwapChain.h>
#include <DX3D/Graphics/DeviceContext.h>

#pragma comment(lib, "d3dcompiler.lib")

using namespace dx3d;

dx3d::RenderSystem::RenderSystem(const RenderSystemDesc& desc) : Base(desc.base)
{
    D3D_FEATURE_LEVEL featureLevel{};
    UINT createDeviceFlags{};

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    DX3DGraphicsLogErrorAndThrow(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags,
        NULL, 0, D3D11_SDK_VERSION,
        &m_d3dDevice, &featureLevel, &m_d3dContext),
        "Direct3D11 initialization failed.");

    DX3DGraphicsLogErrorAndThrow(m_d3dDevice->QueryInterface(IID_PPV_ARGS(&m_dxgiDevice)),
        "QueryInterface failed to retrieve IDXGIDevice.");

    DX3DGraphicsLogErrorAndThrow(m_dxgiDevice->GetParent(IID_PPV_ARGS(&m_dxgiAdapter)),
        "GetParent failed to retrieve IDXGIAdapter.");

    DX3DGraphicsLogErrorAndThrow(m_dxgiAdapter->GetParent(IID_PPV_ARGS(&m_dxgiFactory)),
        "GetParent failed to retrieve IDXGIFactory.");

    // New: Configure and set the rasterizer state
    D3D11_RASTERIZER_DESC rasterDesc{};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_BACK; // Still cull back faces
    rasterDesc.FrontCounterClockwise = TRUE; // Set front faces to be CCW to match your vertex winding

    DX3DGraphicsLogErrorAndThrow(m_d3dDevice->CreateRasterizerState(&rasterDesc, &m_rasterState),
        "Failed to create rasterizer state.");

    // Set the created rasterizer state to the device context
    m_d3dContext->RSSetState(m_rasterState.Get());

    initializeDeviceContext();
}

dx3d::RenderSystem::~RenderSystem()
{
}

void dx3d::RenderSystem::initializeDeviceContext()
{
    m_deviceContextPtr = std::make_shared<DeviceContext>(
        getGraphicsResourceDescForInit(),
        m_d3dContext.Get()
    );
}

SwapChainPtr dx3d::RenderSystem::createSwapChain(const SwapChainDesc& desc) const
{
    return std::make_shared<SwapChain>(desc, getGraphicsResourceDesc());
}

GraphicsResourceDesc dx3d::RenderSystem::getGraphicsResourceDescForInit() const noexcept
{
    return { {m_logger}, nullptr, *m_d3dDevice.Get(), *m_dxgiFactory.Get() };
}

GraphicsResourceDesc dx3d::RenderSystem::getGraphicsResourceDesc() const noexcept
{
    return { {m_logger}, shared_from_this(), *m_d3dDevice.Get(), *m_dxgiFactory.Get() };
}

DeviceContext& dx3d::RenderSystem::getDeviceContext() const noexcept
{
    return *m_deviceContextPtr;
}