#pragma once
#pragma once

#include "imgui.h"

class UIElement {
public:
    virtual ~UIElement() = default;
    virtual void Render() = 0;
};