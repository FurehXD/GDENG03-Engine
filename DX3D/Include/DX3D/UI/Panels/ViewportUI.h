#pragma once
#include <DX3D/Math/Math.h>

namespace dx3d
{
    class ViewportManager;
    enum class ViewportType;

    class ViewportUI
    {
    public:
        explicit ViewportUI(ViewportManager& viewportManager);

        void renderGameView();
        void renderSceneView();

    private:
        void renderViewport(ViewportType type, const char* title, const Vector2& position, const Vector2& size);

    private:
        ViewportManager& m_viewportManager;
    };
}