#include <DX3D/UI/Panels/ViewportUI.h>
#include <DX3D/Game/ViewportManager.h>
#include <DX3D/Graphics/RenderTexture.h>
#include <imgui.h>

using namespace dx3d;

ViewportUI::ViewportUI(ViewportManager& viewportManager)
    : m_viewportManager(viewportManager)
{
}

void ViewportUI::renderGameView()
{
    ImGuiIO& io = ImGui::GetIO();
    float windowWidth = io.DisplaySize.x;
    float windowHeight = io.DisplaySize.y;
    float halfWidth = windowWidth * 0.5f;
    float halfHeight = windowHeight * 0.5f;

    renderViewport(ViewportType::Game, "Game View", Vector2(0, 20), Vector2(halfWidth, halfHeight - 20));
}

void ViewportUI::renderSceneView()
{
    ImGuiIO& io = ImGui::GetIO();
    float windowWidth = io.DisplaySize.x;
    float windowHeight = io.DisplaySize.y;
    float halfWidth = windowWidth * 0.5f;
    float halfHeight = windowHeight * 0.5f;

    renderViewport(ViewportType::Scene, "Scene View", Vector2(0, halfHeight), Vector2(halfWidth, halfHeight));
}

void ViewportUI::renderViewport(ViewportType type, const char* title, const Vector2& position, const Vector2& size)
{
    ImGui::SetNextWindowPos(ImVec2(position.x, position.y));
    ImGui::SetNextWindowSize(ImVec2(size.x, size.y));
    ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    if (viewportSize.x > 0 && viewportSize.y > 0)
    {
        m_viewportManager.resize(type, static_cast<ui32>(viewportSize.x), static_cast<ui32>(viewportSize.y));
        auto& viewport = m_viewportManager.getViewport(type);

        ImVec2 imagePos = ImGui::GetCursorScreenPos();
        ImGui::Image((void*)viewport.renderTexture->getShaderResourceView(), viewportSize);

        bool isImageHovered = ImGui::IsItemHovered();
        ImVec2 mousePos = ImGui::GetMousePos();
        float localX = mousePos.x - imagePos.x;
        float localY = mousePos.y - imagePos.y;

        bool isMouseInImageBounds = (localX >= 0 && localY >= 0 &&
            localX < viewportSize.x && localY < viewportSize.y);

        m_viewportManager.updateViewportStates(type,
            isImageHovered && isMouseInImageBounds,
            ImGui::IsWindowFocused(),
            localX, localY);
    }
    ImGui::End();
}