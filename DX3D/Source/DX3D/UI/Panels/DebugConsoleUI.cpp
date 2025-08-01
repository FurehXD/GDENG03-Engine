#include <DX3D/UI/Panels/DebugConsoleUI.h>
#include <DX3D/Core/Logger.h>
#include <imgui.h>

using namespace dx3d;

DebugConsoleUI::DebugConsoleUI(Logger& logger)
    : m_logger(logger)
{
}

void DebugConsoleUI::render()
{
    ImGuiIO& io = ImGui::GetIO();
    float windowWidth = io.DisplaySize.x;
    float windowHeight = io.DisplaySize.y;
    float halfWidth = windowWidth * 0.5f;
    float halfHeight = windowHeight * 0.5f;
    float debugHeight = halfHeight * 0.4f;

    ImGui::SetNextWindowPos(ImVec2(halfWidth, windowHeight - debugHeight));
    ImGui::SetNextWindowSize(ImVec2(halfWidth, debugHeight));

    ImGui::Begin("Debug Console", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    auto logEntries = m_logger.getRecentLogs(100);

    if (ImGui::Button("Clear Logs"))
    {
        m_logger.clearLogs();
    }

    ImGui::SameLine();
    ImGui::Text("Log Entries: %zu", logEntries.size());

    ImGui::Separator();

    ImGui::BeginChild("LogScrollRegion", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

    for (const auto& entry : logEntries)
    {
        ImVec4 color;
        const char* levelText;

        switch (entry.level)
        {
        case LogEntry::Level::Error:
            color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
            levelText = "[ERROR]";
            break;
        case LogEntry::Level::Warning:
            color = ImVec4(1.0f, 1.0f, 0.4f, 1.0f);
            levelText = "[WARN]";
            break;
        case LogEntry::Level::Info:
            color = ImVec4(0.4f, 1.0f, 0.4f, 1.0f);
            levelText = "[INFO]";
            break;
        default:
            color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            levelText = "[LOG]";
            break;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::Text("%s %s %s", entry.timestamp.c_str(), levelText, entry.message.c_str());
        ImGui::PopStyleColor();
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();

    ImGui::End();
}