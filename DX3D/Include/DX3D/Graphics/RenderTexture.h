#pragma once
#include <DX3D/Graphics/GraphicsResource.h>
#include <DX3D/Core/Common.h>

namespace dx3d
{
    class DeviceContext;  

    class RenderTexture final : public GraphicsResource
    {
    public:
        RenderTexture(ui32 width, ui32 height, const GraphicsResourceDesc& desc);
        ~RenderTexture();

        void resize(ui32 width, ui32 height);
        void clear(DeviceContext& deviceContext, float r, float g, float b, float a);
        void setAsRenderTarget(DeviceContext& deviceContext);

        ID3D11ShaderResourceView* getShaderResourceView() const { return m_shaderResourceView.Get(); }
        ID3D11RenderTargetView* getRenderTargetView() const { return m_renderTargetView.Get(); }
        ID3D11DepthStencilView* getDepthStencilView() const { return m_depthStencilView.Get(); }

        Rect getSize() const;

    private:
        void createResources(ui32 width, ui32 height);
        void releaseResources();

    private:
        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthTexture;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
        ui32 m_width;
        ui32 m_height;
    };
}