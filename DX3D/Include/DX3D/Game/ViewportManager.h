#pragma once
#include <DX3D/Core/Core.h>
#include <DX3D/Math/Math.h>
#include <memory>

namespace dx3d
{
    class RenderTexture;
    class SceneCamera;
    class CameraObject;

    enum class ViewportType
    {
        Scene,
        Game
    };

    struct Viewport
    {
        ViewportType type;
        std::shared_ptr<RenderTexture> renderTexture;
        ui32 width;
        ui32 height;
        bool isHovered = false;
        bool isFocused = false;
        Vector2 mousePos;
    };

    class ViewportManager
    {
    public:
        ViewportManager();
        ~ViewportManager();

        void initialize(GraphicsEngine& graphicsEngine, ui32 width, ui32 height);
        void resize(ViewportType type, ui32 width, ui32 height);

        Viewport& getViewport(ViewportType type);
        const Viewport& getViewport(ViewportType type) const;

        void updateViewportStates(ViewportType type, bool hovered, bool focused, float mouseX, float mouseY);

    private:
        Viewport m_sceneViewport;
        Viewport m_gameViewport;
        bool m_initialized = false;
    };
}