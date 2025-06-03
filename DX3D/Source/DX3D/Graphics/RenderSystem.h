#pragma once
#include <DX3D/Graphics/GraphicsResource.h>
#include <DX3D/Core/Common.h>
#include <DX3D/Core/Base.h>
#include <d3d11.h>
#include <wrl.h>

namespace dx3d
{
    class SwapChain;
    class DeviceContext;
    using SwapChainPtr = std::shared_ptr<SwapChain>;
    using DeviceContextPtr = std::shared_ptr<DeviceContext>;

    class RenderSystem final : public Base, public std::enable_shared_from_this<RenderSystem>
    {
    public:
        explicit RenderSystem(const RenderSystemDesc& desc);
        virtual ~RenderSystem() override;

        SwapChainPtr createSwapChain(const SwapChainDesc& desc) const;

        GraphicsResourceDesc getGraphicsResourceDescForInit() const noexcept;

        GraphicsResourceDesc getGraphicsResourceDesc() const noexcept;

        DeviceContext& getDeviceContext() const noexcept;

    private:
        void initializeDeviceContext();

    private:
        Microsoft::WRL::ComPtr<ID3D11Device> m_d3dDevice{};
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_d3dContext{};
        Microsoft::WRL::ComPtr<IDXGIDevice> m_dxgiDevice{};
        Microsoft::WRL::ComPtr<IDXGIAdapter> m_dxgiAdapter{};
        Microsoft::WRL::ComPtr<IDXGIFactory> m_dxgiFactory{};
        DeviceContextPtr m_deviceContextPtr{};
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterState{}; // New: Rasterizer state
    };
}