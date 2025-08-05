#include <DX3D/UI/Elements/ImageElement.h>
#include <wincodec.h> // For WIC (Windows Imaging Component)
#include <stdexcept>
#include <wrl/client.h>
#include <vector>

#pragma comment(lib, "windowscodecs.lib")

// Helper function to convert std::string to std::wstring
std::wstring to_wide_string(const std::string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

ImageElement::ImageElement(ID3D11Device* device, const char* imagePath)
{
    if (!device || !imagePath) return;

    // Initialize COM
    HRESULT hr = CoInitialize(nullptr);

    // Create WIC factory
    Microsoft::WRL::ComPtr<IWICImagingFactory> wicFactory;
    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&wicFactory)
    );
    if (FAILED(hr)) {
        CoUninitialize();
        return;
    }

    std::wstring widePath = to_wide_string(imagePath);

    // Create a decoder
    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    hr = wicFactory->CreateDecoderFromFilename(
        widePath.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &decoder
    );
    if (FAILED(hr)) {
        CoUninitialize();
        return;
    }

    // Get the first frame
    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) {
        CoUninitialize();
        return;
    }

    // Create a format converter
    Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
    hr = wicFactory->CreateFormatConverter(&converter);
    if (FAILED(hr)) {
        CoUninitialize();
        return;
    }

    // Initialize the format converter
    hr = converter->Initialize(
        frame.Get(),
        GUID_WICPixelFormat32bppRGBA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeMedianCut
    );
    if (FAILED(hr)) {
        CoUninitialize();
        return;
    }

    UINT u_width, u_height;
    converter->GetSize(&u_width, &u_height);
    width = static_cast<int>(u_width);
    height = static_cast<int>(u_height);

    std::vector<uint8_t> pixels(static_cast<size_t>(width) * height * 4);
    converter->CopyPixels(nullptr, width * 4, static_cast<UINT>(pixels.size()), pixels.data());

    // Create texture
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA subResource = {};
    subResource.pSysMem = pixels.data();
    subResource.SysMemPitch = desc.Width * 4;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture;
    device->CreateTexture2D(&desc, &subResource, &pTexture);

    // Create texture view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    if (pTexture)
    {
        device->CreateShaderResourceView(pTexture.Get(), &srvDesc, &textureView);
    }

    CoUninitialize();
}

ImageElement::~ImageElement()
{
    if (textureView) {
        textureView->Release();
    }
}