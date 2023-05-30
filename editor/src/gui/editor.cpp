#include "gui/editor.hpp"
#include "utils/utils.hpp"
#include "utils/yaml_types.hpp"

#include <filesystem>

#include <GLFW/glfw3.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>
#include <portable-file-dialogs/portable-file-dialogs.h>

namespace oge {

enum ConsoleFileReadingStage {
    ConsoleFileReadingStage_Type,
    ConsoleFileReadingStage_TimeRecorded,
    ConsoleFileReadingStage_Message
};

PanelEditorWorkspaceBase::PanelEditorWorkspaceBase(std::string_view name) : m_name(name) {
    m_io = &ImGui::GetIO();
    static_cast<void>(*m_io);
}

EditorWorkspace::EditorWorkspace() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable |
                      ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForOpenGL(ogl::Pipeline::get()->get_window()->get_internal(), true);
    ImGui_ImplOpenGL3_Init("#version 450");

    // Settings Preferences

    std::string settings_path = pfd::path::home() + "/.config/oge";
    if (!std::filesystem::exists(settings_path)) {
        std::filesystem::create_directory(settings_path);

        // Create preference config file and its default for backup
        std::filesystem::copy_file(
            "editor/assets/default_settings/preferences.yaml", settings_path + "/preferences.yaml"
        );
        std::filesystem::copy_file(
            "editor/assets/default_settings/preferences.yaml",
            settings_path + "/default_preferences.yaml"
        );

        // Pushing the 2 default layouts
        std::string layouts_path = settings_path + "/layouts";
        std::filesystem::create_directory(layouts_path);
        std::filesystem::copy_file(
            "editor/assets/default_settings/layout1.ini", layouts_path + "/layout1.ini"
        );
        std::filesystem::copy_file(
            "editor/assets/default_settings/layout2.ini", layouts_path + "/layout2.ini"
        );
    }

    std::string path = settings_path + "/preferences.yaml";
    yaml::Node preferences = yaml::open(path);
    yaml::Node& ui = preferences["EditorUI"];

    if (ui["ResetLayoutOnLoad"].as<bool>()) {
        std::remove("imgui.ini");

        std::string default_layout = ui["Layout"].as<std::string>();
        if (!std::filesystem::copy_file(
                settings_path + "/layouts/" + default_layout + ".ini", "imgui.ini"
            )) {
            ogl::Debug::log("Failed to load editor layout", ogl::DebugType_Error);
        }
    }

    io.FontDefault =
        io.Fonts->AddFontFromFileTTF(ui["FontRegular"].as<std::string>().c_str(), 18.0f);

    _load_color_theme(ui["ColorTheme"]);
}

EditorWorkspace::~EditorWorkspace() {
    for (PanelEditorWorkspaceBase* panel : m_panels) {
        delete panel;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

PanelEditorWorkspaceBase* EditorWorkspace::get_panel(std::string_view name) {
    for (PanelEditorWorkspaceBase* panel : m_panels) {
        if (panel->get_name() == name) {
            return panel;
        }
    }

    return nullptr;
}

void EditorWorkspace::remove_panel(std::string_view name) {
    for (std::size_t i = 0; i < m_panels.size(); i++) {
        if (m_panels[i]->get_name() == name) {
            delete m_panels[i];
            m_panels.erase(m_panels.begin() + i);
            return;
        }
    }
}

void EditorWorkspace::push_panels(std::initializer_list<PanelEditorWorkspaceBase*> panels) {
    for (PanelEditorWorkspaceBase* panel : panels) {
        m_panels.push_back(panel);
    }
}

void EditorWorkspace::on_update() {
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    for (PanelEditorWorkspaceBase* panel : m_panels) {
        if (!panel->get_enabled()) {
            if (panel->get_remove_when_disabled()) {
                remove_panel(panel->get_name());
            }
        } else {
            panel->on_imgui_update();
        }
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    static ImGuiIO& io = ImGui::GetIO();
    (void)io;
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_context);
    }
}

void EditorWorkspace::_load_color_theme(yaml::Node& ui_color) {
    auto& colors = ImGui::GetStyle().Colors;

    colors[ImGuiCol_WindowBg] = ui_color["WindowBg"].as<ImVec4>();
    colors[ImGuiCol_Header] = ui_color["Header"].as<ImVec4>();
    colors[ImGuiCol_HeaderHovered] = ui_color["HeaderHovered"].as<ImVec4>();
    colors[ImGuiCol_HeaderActive] = ui_color["HeaderActive"].as<ImVec4>();
    colors[ImGuiCol_Button] = ui_color["Button"].as<ImVec4>();
    colors[ImGuiCol_ButtonHovered] = ui_color["ButtonHovered"].as<ImVec4>();
    colors[ImGuiCol_ButtonActive] = ui_color["ButtonActive"].as<ImVec4>();
    colors[ImGuiCol_FrameBg] = ui_color["FrameBg"].as<ImVec4>();
    colors[ImGuiCol_FrameBgHovered] = ui_color["FrameBgHovered"].as<ImVec4>();
    colors[ImGuiCol_FrameBgActive] = ui_color["FrameBgActive"].as<ImVec4>();
    colors[ImGuiCol_Tab] = ui_color["Tab"].as<ImVec4>();
    colors[ImGuiCol_TabHovered] = ui_color["TabHovered"].as<ImVec4>();
    colors[ImGuiCol_TabActive] = ui_color["TabActive"].as<ImVec4>();
    colors[ImGuiCol_TabUnfocused] = ui_color["TabUnfocused"].as<ImVec4>();
    colors[ImGuiCol_TabUnfocusedActive] = ui_color["TabUnfocusedActive"].as<ImVec4>();
    colors[ImGuiCol_TitleBg] = ui_color["TitleBg"].as<ImVec4>();
    colors[ImGuiCol_TitleBgActive] = ui_color["TitleBgActive"].as<ImVec4>();
    colors[ImGuiCol_TitleBgCollapsed] = ui_color["TitleBgCollapsed"].as<ImVec4>();
}

/******************************************************************************/
/******************************** Base Windows ********************************/
/******************************************************************************/

ConsoleEditorWorkspace::ConsoleEditorWorkspace(ogl::Debug* debug)
    : PanelEditorWorkspaceBase("Console") {
    std::string names[] = {"Messages", "Warnings", "Errors", "Fatal Errors", "Inits", "Terminate"};
    for (std::size_t i = 0; i < ogl::debug_type_count; i++) {
        std::get<bool>(m_filters[i]) = true;
        std::get<std::string>(m_filters[i]) = std::move(names[i]);
    }

    std::string settings_path = pfd::path::home() + "/.config/oge/preferences.yaml";
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    yaml::Node preferences = yaml::open(settings_path);
    m_font = io.Fonts->AddFontFromFileTTF(
        preferences["EditorUI"].get_child("FontMono").as<std::string>().c_str(), 18.0f
    );

    m_debug = debug;
}

void ConsoleEditorWorkspace::on_imgui_update() {
    ImGui::Begin(get_name().c_str(), &get_enabled(), ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Options")) {
            if (ImGui::BeginMenu("Filter")) {
                for (std::tuple<bool, std::string>& filter : m_filters) {
                    ImGui::MenuItem(
                        std::get<std::string>(filter).c_str(), nullptr, &std::get<bool>(filter)
                    );
                }
                ImGui::EndMenu();
            }
            ImGui::MenuItem("Auto-Scrolling", nullptr, &m_auto_scrolling);
            if (ImGui::MenuItem("Log Tests")) {
                ogl::Debug::log("Test Message");
                ogl::Debug::log("Test Warning Message", ogl::DebugType_Warning);
                ogl::Debug::log("Test Error Message", ogl::DebugType_Error);
            }
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Clear")) {
            m_debug->clear_logs();
        }
        ImGui::EndMenuBar();
    }

    ImGui::PushFont(m_font);

    for (const std::tuple<ogl::DebugType, std::string, float>& log : m_debug->get_logs()) {
        if (std::get<bool>(m_filters[static_cast<std::size_t>(std::get<ogl::DebugType>(log))])) {
            std::string prefix{};
            switch (std::get<ogl::DebugType>(log)) {
            case ogl::DebugType_Warning:
                prefix = "[Warning (" + std::to_string(std::get<float>(log)) + ")]: ";
                ImGui::PushStyleColor(
                    ImGuiCol_Text, ImVec4(
                                       m_debug_colors[0].r, m_debug_colors[0].g,
                                       m_debug_colors[0].b, m_debug_colors[0].a
                                   )
                );
                break;
                prefix = "[Warning (" + std::to_string(std::get<float>(log)) + ")]: ";
            case ogl::DebugType_Error:
                prefix = "[Error (" + std::to_string(std::get<float>(log)) + ")]: ";
                ImGui::PushStyleColor(
                    ImGuiCol_Text, ImVec4(
                                       m_debug_colors[1].r, m_debug_colors[1].g,
                                       m_debug_colors[1].b, m_debug_colors[1].a
                                   )
                );
                break;
            case ogl::DebugType_Message:
                prefix = "[Message (" + std::to_string(std::get<float>(log)) + ")]: ";
                break;
            }

            ImGui::TextWrapped("%s", std::string(prefix + std::get<std::string>(log)).c_str());
            if (std::get<ogl::DebugType>(log) != ogl::DebugType_Message) {
                ImGui::PopStyleColor();
            }
        }
    }

    if (m_auto_scrolling && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::PopFont();

    ImGui::End();
}

AssetsEditorWorkspace::AssetsEditorWorkspace() : PanelEditorWorkspaceBase("Assets") {}

void AssetsEditorWorkspace::on_imgui_update() {
    ImGui::Begin(get_name().c_str(), &get_enabled());
    ImGui::End();
}

} // namespace oge
