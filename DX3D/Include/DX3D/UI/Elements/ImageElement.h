#pragma once

#include "UIElement.h"
#include <string>
#include <d3d11.h>

class ImageElement : public UIElement {
public:
    ImageElement(ID3D11Device* device, const char* imagePath);
    ~ImageElement();

    void Render() override {
        if (textureView) {
            ImGui::Image((void*)textureView, ImVec2(width, height));
        }
    }

private:
    ID3D11ShaderResourceView* textureView = nullptr;
    int width = 0;
    int height = 0;
};