#pragma once
#include <DX3D/Graphics/GraphicsResource.h>
#include <DX3D/Graphics/Vertex.h>
#include <d3d11.h>
#include <wrl.h>

namespace dx3d
{
    class VertexBuffer;
    class SwapChain;

    class DeviceContext final : public GraphicsResource
    {
    public:
        explicit DeviceContext(const GraphicsResourceDesc& desc, ID3D11DeviceContext* deviceContext);
        ~DeviceContext();

        void clearRenderTargetColor(SwapChain& swapChain, float red, float green, float blue, float alpha);
        void setVertexBuffer(const VertexBuffer& vertexBuffer);
        void setViewportSize(ui32 width, ui32 height);
        void setVertexShader(ID3D11VertexShader* vertexShader);
        void setPixelShader(ID3D11PixelShader* pixelShader);
        void setInputLayout(ID3D11InputLayout* inputLayout);
        void setRenderTargets(SwapChain& swapChain);
        void drawTriangleList(ui32 vertexCount, ui32 startVertexIndex);
        void drawTriangleStrip(ui32 vertexCount, ui32 startVertexIndex);
        void present(SwapChain& swapChain);

        ID3D11DeviceContext* getDeviceContext();

    private:
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
    };
}