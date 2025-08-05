#pragma once

#include "UIElement.h"
#include <string>

class TextElement : public UIElement {
public:
    TextElement(const std::string& text) : text(text) {}

    void Render() override {
        ImGui::Text(text.c_str());
    }

private:
    std::string text;
};