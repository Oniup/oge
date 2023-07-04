#include "gui/console.hpp"

#include <imgui/imgui.h>
#include <portable-file-dialogs/portable-file-dialogs.h>

namespace workspace {

KConsole::KConsole(KLDebug* debug) : KIWorkspace("Console")
{
    std::string names[] = {"Messages", "Warnings", "Errors", "Fatal Errors", "Inits", "Terminate"};
    for (std::size_t i = 0; i < debug_type_count; i++)
    {
        std::get<bool>(m_filters[i]) = true;
        std::get<std::string>(m_filters[i]) = std::move(names[i]);
    }

    ImGuiIO& io = ImGui::GetIO();
    (void)io;

#if defined(_MSC_VER)
    // TODO: ...
#else
    std::string preferences_path = pfd::path::home() + "/.config/kryos/preferences.yaml";
#endif

    yaml::Node preferences = yaml::open(preferences_path);
    m_font = io.Fonts->AddFontFromFileTTF(
        preferences["EditorUI"].get_child("FontMono").as<std::string>().c_str(), 18.0f
    );

    m_debug = debug;
}

void KConsole::on_imgui_update()
{
    ImGui::Begin(get_name().c_str(), &get_enabled(), ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Options"))
        {
            if (ImGui::BeginMenu("Filter"))
            {
                for (std::tuple<bool, std::string>& filter : m_filters)
                    ImGui::MenuItem(
                        std::get<std::string>(filter).c_str(), nullptr, &std::get<bool>(filter)
                    );
                ImGui::EndMenu();
            }
            ImGui::MenuItem("Auto-Scrolling", nullptr, &m_auto_scrolling);
            if (ImGui::MenuItem("Log Tests"))
            {
                KLDebug::log("Test Message");
                KLDebug::log("Test Warning Message", KEDebugType_Warning);
                KLDebug::log("Test Error Message", KEDebugType_Error);
            }
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Clear"))
            m_debug->clear_logs();
        ImGui::EndMenuBar();
    }

    ImGui::PushFont(m_font);

    for (const std::tuple<KEDebugType, std::string, float>& log : m_debug->get_logs())
    {
        if (std::get<bool>(m_filters[static_cast<std::size_t>(std::get<KEDebugType>(log))]))
        {
            std::string prefix{};
            switch (std::get<KEDebugType>(log))
            {
            case KEDebugType_Warning:
                prefix = "[Warning (" + std::to_string(std::get<float>(log)) + ")]: ";
                ImGui::PushStyleColor(
                    ImGuiCol_Text, ImVec4(
                                       m_debug_colors[0].r, m_debug_colors[0].g,
                                       m_debug_colors[0].b, m_debug_colors[0].a
                                   )
                );
                break;
                prefix = "[Warning (" + std::to_string(std::get<float>(log)) + ")]: ";
            case KEDebugType_Error:
                prefix = "[Error (" + std::to_string(std::get<float>(log)) + ")]: ";
                ImGui::PushStyleColor(
                    ImGuiCol_Text, ImVec4(
                                       m_debug_colors[1].r, m_debug_colors[1].g,
                                       m_debug_colors[1].b, m_debug_colors[1].a
                                   )
                );
                break;
            case KEDebugType_Message:
                prefix = "[Message (" + std::to_string(std::get<float>(log)) + ")]: ";
                break;
            }

            ImGui::TextWrapped("%s", std::string(prefix + std::get<std::string>(log)).c_str());
            if (std::get<KEDebugType>(log) != KEDebugType_Message)
                ImGui::PopStyleColor();
        }
    }

    if (m_auto_scrolling && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::PopFont();

    ImGui::End();
}

} // namespace workspace
