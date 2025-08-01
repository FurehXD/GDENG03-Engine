#pragma once
#include <DX3D/Core/Core.h>
#include <imgui.h>

namespace dx3d
{
    struct UIState
    {
        struct PanelState
        {
            bool isOpen = true;
            ImVec2 position;
            ImVec2 size;
            bool hasCustomPosition = false;
            bool hasCustomSize = false;
        };

        PanelState sceneControls;
        PanelState sceneOutliner;
        PanelState inspector;
        PanelState debugConsole;
        PanelState gameView;
        PanelState sceneView;

        bool showMainMenuBar = true;
    };
}