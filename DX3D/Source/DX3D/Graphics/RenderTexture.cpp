#include <DX3D/Graphics/RenderTexture.h>
#include <DX3D/Graphics/DeviceContext.h>

dx3d::RenderTexture::RenderTexture(ui32 width, ui32 height, const GraphicsResourceDesc& desc)
    : GraphicsResource(desc), m_width(width), m_height(height)
{
    createResources(width, height);
}

dx3d::RenderTexture::~RenderTexture()
{
}

void dx3d::RenderTexture::createResources(ui32 width, ui32 height)
{
    D3D11_TEXTURE2D_DESC textureDesc{};
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    DX3DGraphicsLogErrorAndThrow(
        m_device.CreateTexture2D(&textureDesc, nullptr, &m_texture),
        "Failed to create render texture"
    );

    DX3DGraphicsLogErrorAndThrow(
        m_device.CreateRenderTargetView(m_texture.Get(), nullptr, &m_renderTargetView),
        "Failed to create render target view"
    );

    DX3DGraphicsLogErrorAndThrow(
        m_device.CreateShaderResourceView(m_texture.Get(), nullptr, &m_shaderResourceView),
        "Failed to create shader resource view"
    );

    D3D11_TEXTURE2D_DESC depthTextureDesc{};
    depthTextureDesc.Width = width;
    depthTextureDesc.Height = height;
    depthTextureDesc.MipLevels = 1;
    depthTextureDesc.ArraySize = 1;
    depthTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthTextureDesc.SampleDesc.Count = 1;
    depthTextureDesc.SampleDesc.Quality = 0;
    depthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthTextureDesc.CPUAccessFlags = 0;
    depthTextureDesc.MiscFlags = 0;

    DX3DGraphicsLogErrorAndThrow(
        m_device.CreateTexture2D(&depthTextureDesc, nullptr, &m_depthTexture),
        "Failed to create depth texture"
    );

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = depthTextureDesc.Format;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    DX3DGraphicsLogErrorAndThrow(
        m_device.CreateDepthStencilView(m_depthTexture.Get(), &dsvDesc, &m_depthStencilView),
        "Failed to create depth stencil view"
    );
}

void dx3d::RenderTexture::releaseResources()
{
    m_depthStencilView.Reset();
    m_depthTexture.Reset();
    m_shaderResourceView.Reset();
    m_renderTargetView.Reset();
    m_texture.Reset();
}

void dx3d::RenderTexture::resize(ui32 width, ui32 height)
{
    if (m_width == width && m_height == height)
        return;

    m_width = width;
    m_height = height;
    releaseResources();
    createResources(width, height);
}

void dx3d::RenderTexture::clear(DeviceContext& deviceContext, float r, float g, float b, float a)
{
    float clearColor[] = { r, g, b, a };
    auto d3dContext = deviceContext.getDeviceContext();
    d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
    d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void dx3d::RenderTexture::setAsRenderTarget(DeviceContext& deviceContext)
{
    auto d3dContext = deviceContext.getDeviceContext();
    ID3D11RenderTargetView* rtv = m_renderTargetView.Get();
    d3dContext->OMSetRenderTargets(1, &rtv, m_depthStencilView.Get());
}

dx3d::Rect dx3d::RenderTexture::getSize() const
{
    return { (i32) m_width, (i32) m_height };
}