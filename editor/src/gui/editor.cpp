#include "gui/editor.hpp"
#include "utils/utils.hpp"
#include "utils/yaml_types.hpp"

#include <kryos/core/application.hpp>
#include <kryos/core/debug.hpp>
#include <kryos/renderer/window.hpp>
#include <kryos/utils/utils.hpp>

#include <GLFW/glfw3.h>
#include <filesystem>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>
#include <portable-file-dialogs/portable-file-dialogs.h>

enum ConsoleFileReadingStage
{
    ConsoleFileReadingStage_Type,
    ConsoleFileReadingStage_TimeRecorded,
    ConsoleFileReadingStage_Message
};

workspace::KIWorkspace::KIWorkspace(const std::string& name) : m_name(name)
{
    m_io = &ImGui::GetIO();
    static_cast<void>(*m_io);
}

KLEditorWorkspace::KLEditorWorkspace()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable |
                      ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForOpenGL(KIApplication::get_layer<KLWindow>()->get_internal(), true);
    ImGui_ImplOpenGL3_Init("#version 450");

    // Settings Preferences

#if defined(_MSC_VER)
    // TODO: ...
#else
    std::string config_path = pfd::path::home() + "/.config/kryos";
#endif

    if (!std::filesystem::exists(config_path))
    {
        std::filesystem::create_directory(config_path);

        // Create preference config file and its default for backup
        std::filesystem::copy_file(
            "editor/assets/default_settings/preferences.yaml", config_path + "/preferences.yaml"
        );
        std::filesystem::copy_file(
            "editor/assets/default_settings/preferences.yaml",
            config_path + "/default_preferences.yaml"
        );

        // Pushing the 2 default layouts
        std::string layouts_path = config_path + "/layouts";
        std::filesystem::create_directory(layouts_path);
        std::filesystem::copy_file(
            "editor/assets/default_settings/layout1.ini", layouts_path + "/layout1.ini"
        );
        std::filesystem::copy_file(
            "editor/assets/default_settings/layout2.ini", layouts_path + "/layout2.ini"
        );
    }

    std::string path = config_path + "/preferences.yaml";
    yaml::Node preferences = yaml::open(path);
    yaml::Node& ui = preferences["EditorUI"];

    if (ui["ResetLayoutOnLoad"].as<bool>())
    {
        std::remove("imgui.ini");

        std::string default_layout = ui["Layout"].as<std::string>();
        if (!std::filesystem::copy_file(
                config_path + "/layouts/" + default_layout + ".ini", "imgui.ini"
            ))
            KLDebug::log("Failed to load editor layout", KEDebugType_Error);
    }

    io.FontDefault =
        io.Fonts->AddFontFromFileTTF(ui["FontRegular"].as<std::string>().c_str(), 18.0f);

    _load_colors(ui["Theme"]);
    _load_styles(ui["Style"]);
}

KLEditorWorkspace::~KLEditorWorkspace()
{
    for (workspace::KIWorkspace* panel : m_panels)
    {
        delete panel;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

workspace::KIWorkspace* KLEditorWorkspace::get_panel(const std::string& name)
{
    for (workspace::KIWorkspace* panel : m_panels)
    {
        if (panel->get_name() == name)
        {
            return panel;
        }
    }

    return nullptr;
}

void KLEditorWorkspace::remove_panel(const std::string& name)
{
    for (std::size_t i = 0; i < m_panels.size(); i++)
    {
        if (m_panels[i]->get_name() == name)
        {
            delete m_panels[i];
            m_panels.erase(m_panels.begin() + i);
            return;
        }
    }
}

void KLEditorWorkspace::push_panels(std::initializer_list<workspace::KIWorkspace*> panels)
{
    for (workspace::KIWorkspace* panel : panels)
        m_panels.push_back(panel);
}

void KLEditorWorkspace::on_update()
{
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    for (workspace::KIWorkspace* panel : m_panels)
    {
        if (!panel->get_enabled())
        {
            if (panel->get_remove_when_disabled())
                remove_panel(panel->get_name());
        }
        else
            panel->on_imgui_update();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    static ImGuiIO& io = ImGui::GetIO();
    (void)io;
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_context);
    }
}

void KLEditorWorkspace::_load_colors(yaml::Node& colors)
{
    auto& coloring = ImGui::GetStyle().Colors;

    std::size_t i = 0;
    for (yaml::Node& color : colors)
    {
        coloring[i] = color.as<ImVec4>();
        i++;
    }
}

void KLEditorWorkspace::_load_styles(yaml::Node& styles)
{
    auto& styling = ImGui::GetStyle();

    styling.WindowPadding = styles["WindowPadding"].as<ImVec2>();
    styling.CellPadding = styles["CellPadding"].as<ImVec2>();
    styling.ItemSpacing = styles["ItemSpacing"].as<ImVec2>();
    styling.ScrollbarSize = styles["ScrollbarSize"].as<float>();
    styling.GrabMinSize = styles["GrabMinSize"].as<float>();
    styling.ItemSpacing = styles["ItemSpacing"].as<ImVec2>();
    styling.WindowRounding = styles["WindowRounding"].as<float>();
    styling.ChildRounding = styles["ChildRounding"].as<float>();
    styling.FrameRounding = styles["FrameRounding"].as<float>();
    styling.PopupRounding = styles["PopupRounding"].as<float>();
    styling.ScrollbarRounding = styles["ScrollbarRounding"].as<float>();
    styling.TabRounding = styles["TabRounding"].as<float>();

    int window_menu_button_position = styles["WindowMenuButtonPosition"].as<int>();
    switch (window_menu_button_position)
    {
    case 0:
        styling.WindowMenuButtonPosition = ImGuiDir_None;
        break;
    case 1:
        styling.WindowMenuButtonPosition = ImGuiDir_Left;
        break;
    case 2:
        styling.WindowMenuButtonPosition = ImGuiDir_Right;
        break;
    }
}
