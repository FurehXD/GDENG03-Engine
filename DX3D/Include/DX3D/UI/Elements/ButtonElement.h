#pragma once

#include "UIElement.h"
#include <string>
#include <functional>

class ButtonElement : public UIElement {
public:
    ButtonElement(const std::string& label, std::function<void()> onClick)
        : label(label), onClick(onClick) {
    }

    void Render() override {
        if (ImGui::Button(label.c_str())) {
            onClick();
        }
    }

private:
    std::string label;
    std::function<void()> onClick;
};