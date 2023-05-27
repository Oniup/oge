#include "gui/editor.hpp"
#include "utils/yaml_types.hpp"

#include <ogl/utils/filesystem.hpp>

#include <GLFW/glfw3.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>

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

    // FIX: Make sure this works on windows
#ifndef WIN32
    std::string settings_path = ogl::FileSystem::get_env_var("HOME") + "/.config/oge";
#else
    std::string settings_path = ogl::FileSystem::get_env_var("APPDATA") + "/oge";
#endif

    ogl::FileSystemAt settings = ogl::FileSystemAt(settings_path);
    if (!settings.dir_exists()) {
        settings = ogl::FileSystemAt(settings_path, true);
        settings.copy_file_into_this("editor/assets/default_settings/preferences.yaml");
        settings.copy_file_into_this(
            "editor/assets/default_settings/preferences.yaml", "default_preferences.yaml"
        );
        settings.create_dir("layouts");

        settings.set_directory(settings.get_current_path() + "/layouts");
        settings.copy_file_into_this("editor/assets/default_settings/layout1.ini", "layout1.ini");
        settings.copy_file_into_this("editor/assets/default_settings/layout2.ini", "layout2.ini");
        settings.set_directory(settings_path);
    }

    std::string path = settings_path + "/preferences.yaml";
    yaml::Node preferences = yaml::Node::open(path);
    yaml::Node& ui = preferences["EditorUI"];

    if (yaml::Convert<bool>().value(ui["ResetLayoutOnLoad"])) {
        std::remove("imgui.ini");

        std::string default_layout = yaml::Convert<std::string>().value(ui["Layout"]);
        if (!ogl::FileSystem::copy_file(
                settings_path + "/layouts/" + default_layout + ".ini", "imgui.ini"
            )) {
            ogl::Debug::log("Failed to load editor layout", ogl::DebugType_Error);
        }
    }

    io.FontDefault = io.Fonts->AddFontFromFileTTF(
        yaml::Convert<std::string>().value(ui["FontRegular"]).c_str(), 18.0f
    );

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

    ImVec4 col = yaml::Convert<ImVec4>().value(ui_color["WindowBg"]);
    colors[ImGuiCol_WindowBg] = col;

    colors[ImGuiCol_WindowBg] = yaml::Convert<ImVec4>().value(ui_color["WindowBg"]);
    colors[ImGuiCol_Header] = yaml::Convert<ImVec4>().value(ui_color["Header"]);
    colors[ImGuiCol_HeaderHovered] = yaml::Convert<ImVec4>().value(ui_color["HeaderHovered"]);
    colors[ImGuiCol_HeaderActive] = yaml::Convert<ImVec4>().value(ui_color["HeaderActive"]);
    colors[ImGuiCol_Button] = yaml::Convert<ImVec4>().value(ui_color["Button"]);
    colors[ImGuiCol_ButtonHovered] = yaml::Convert<ImVec4>().value(ui_color["ButtonHovered"]);
    colors[ImGuiCol_ButtonActive] = yaml::Convert<ImVec4>().value(ui_color["ButtonActive"]);
    colors[ImGuiCol_FrameBg] = yaml::Convert<ImVec4>().value(ui_color["FrameBg"]);
    colors[ImGuiCol_FrameBgHovered] = yaml::Convert<ImVec4>().value(ui_color["FrameBgHovered"]);
    colors[ImGuiCol_FrameBgActive] = yaml::Convert<ImVec4>().value(ui_color["FrameBgActive"]);
    colors[ImGuiCol_Tab] = yaml::Convert<ImVec4>().value(ui_color["Tab"]);
    colors[ImGuiCol_TabHovered] = yaml::Convert<ImVec4>().value(ui_color["TabHovered"]);
    colors[ImGuiCol_TabActive] = yaml::Convert<ImVec4>().value(ui_color["TabActive"]);
    colors[ImGuiCol_TabUnfocused] = yaml::Convert<ImVec4>().value(ui_color["TabUnfocused"]);
    colors[ImGuiCol_TabUnfocusedActive] =
        yaml::Convert<ImVec4>().value(ui_color["TabUnfocusedActive"]);
    colors[ImGuiCol_TitleBg] = yaml::Convert<ImVec4>().value(ui_color["TitleBg"]);
    colors[ImGuiCol_TitleBgActive] = yaml::Convert<ImVec4>().value(ui_color["TitleBgActive"]);
    colors[ImGuiCol_TitleBgCollapsed] = yaml::Convert<ImVec4>().value(ui_color["TitleBgCollapsed"]);
}

/******************************************************************************/
/******************************** Base Windows ********************************/
/******************************************************************************/

DockingEditorWorkspace::DockingEditorWorkspace(EditorWorkspace* workspace)
    : PanelEditorWorkspaceBase("Docking") {
    m_dock_node_flags = ImGuiDockNodeFlags_None;

    m_window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    m_workspace = workspace;
    get_enabled() = true;
}

void DockingEditorWorkspace::on_imgui_update() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin(get_name().c_str(), &get_enabled(), m_window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dock_space_id = ImGui::GetID("DockSpace");
    ImGui::DockSpace(dock_space_id, ImVec2(0.0f, 0.0f), m_dock_node_flags);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::BeginMenu("Open")) {
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Save", "CTR+S")) {
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (ImGui::BeginMenu("Workspace")) {
                const std::vector<PanelEditorWorkspaceBase*>& panels =
                    m_workspace->get_all_panels();

                for (PanelEditorWorkspaceBase* panel : panels) {
                    if (panel->get_name() != get_name()) {
                        ImGui::MenuItem(panel->get_name().c_str(), nullptr, &panel->get_enabled());
                    }
                }

                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Preferences")) {
                if (m_workspace->get_panel("Preferences") == nullptr) {
                    ogl::Debug::log(
                        "Preference settings are coming soon ...", ogl::DebugType_Warning
                    );
                    // m_workspace->push_panel<PreferencesEditorPopup>();
                }
            }

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::End();
}

void DockingEditorWorkspace::_menu_open_window(std::string_view panel_name) {}

ConsoleEditorWorkspace::ConsoleEditorWorkspace(ogl::Debug* debug)
    : PanelEditorWorkspaceBase("Console") {
    std::string names[] = {"Messages", "Warnings", "Errors", "Fatal Errors", "Inits", "Terminate"};
    for (std::size_t i = 0; i < ogl::debug_type_count; i++) {
        std::get<bool>(m_filters[i]) = true;
        std::get<std::string>(m_filters[i]) = std::move(names[i]);
    }

#ifndef WIN32
    std::string settings_path =
        ogl::FileSystem::get_env_var("HOME") + "/.config/oge/preferences.yaml";
#else
    std::string settings_path = ogl::FileSystem::get_env_var("APPDATA") + "/oge/preferences.yaml";
#endif

    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    yaml::Node preferences = yaml::Node::open(settings_path);
    m_font = io.Fonts->AddFontFromFileTTF(
        yaml::Convert<std::string>().value(preferences["EditorUI"].get_child("FontMono")).c_str(),
        18.0f
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
