#include "gui/editor.hpp"

#include <ogl/utils/filesystem.hpp>

#include <GLFW/glfw3.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>

namespace oge {

ImVec4 convert_into_color(const glm::vec4& vec) { return ImVec4{vec.x, vec.y, vec.z, vec.w}; }

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
        settings.create_dir("layouts");

        settings.set_directory(settings.get_current_path() + "/layouts");
        settings.copy_file_into_this(
            "editor/assets/default_settings/default_layout.ini", "default.ini"
        );
        settings.set_directory(settings_path);
    }

    std::string path = settings_path + "/preferences.yaml";
    ogl::YamlSerialization preferences = ogl::YamlSerialization(path);
    ogl::YamlSerializationOption* ui = preferences.get(PREF_FIELD_EDITOR_UI);

    if (ui->get("reset_layout_on_open")->convert_value<bool>()) {
        std::remove("imgui.ini");

        std::string default_layout = ui->get("layout")->convert_value<std::string>();
        if (!ogl::FileSystem::copy_file(
                settings_path + "/layouts/" + default_layout + ".ini", "imgui.ini"
            )) {
            ogl::Debug::log("Failed to load editor layout", ogl::DebugType_Error);
        }
    }

    io.FontDefault = io.Fonts->AddFontFromFileTTF(
        ui->get(PREF_EDITOR_UI_FONT_REGULAR)->convert_value<std::string>().c_str(), 18.0f
    );

    ogl::YamlSerializationOption* color_theme = ui->get(PREF_FIELD_EDITOR_UI_COLOR);
    _load_color_theme(color_theme);
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
    for (size_t i = 0; i < m_panels.size(); i++) {
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

void EditorWorkspace::_load_color_theme(ogl::YamlSerializationOption* ui_color) {
    auto& colors = ImGui::GetStyle().Colors;

    colors[ImGuiCol_WindowBg] =
        convert_into_color(ui_color->scope[PREF_UI_COLOR_WINDOW_BG].convert_value<glm::vec4>());

    colors[ImGuiCol_Header] =
        convert_into_color(ui_color->scope[PREF_UI_COLOR_HEADER].convert_value<glm::vec4>());
    colors[ImGuiCol_HeaderHovered] =
        convert_into_color(ui_color->scope[PREF_UI_COLOR_HEADER_HOVERED].convert_value<glm::vec4>()
        );
    colors[ImGuiCol_HeaderActive] =
        convert_into_color(ui_color->scope[PREF_UI_COLOR_HEADER_ACTIVE].convert_value<glm::vec4>());

    colors[ImGuiCol_Button] =
        convert_into_color(ui_color->scope[PREF_UI_COLOR_BUTTON].convert_value<glm::vec4>());
    colors[ImGuiCol_ButtonHovered] =
        convert_into_color(ui_color->scope[PREF_UI_COLOR_BUTTON_HOVERED].convert_value<glm::vec4>()
        );
    colors[ImGuiCol_ButtonActive] =
        convert_into_color(ui_color->scope[PREF_UI_COLOR_BUTTON_ACTIVE].convert_value<glm::vec4>());

    colors[ImGuiCol_FrameBg] =
        convert_into_color(ui_color->scope[PREF_UI_COLOR_FRAME_BG].convert_value<glm::vec4>());
    colors[ImGuiCol_FrameBgHovered] = convert_into_color(
        ui_color->scope[PREF_UI_COLOR_FRAME_BG_HOVERED].convert_value<glm::vec4>()
    );
    colors[ImGuiCol_FrameBgActive] =
        convert_into_color(ui_color->scope[PREF_UI_COLOR_FRAME_BG_ACTIVE].convert_value<glm::vec4>()
        );

    colors[ImGuiCol_Tab] =
        convert_into_color(ui_color->scope[PREF_UI_COLOR_TAB].convert_value<glm::vec4>());
    colors[ImGuiCol_TabHovered] =
        convert_into_color(ui_color->scope[PREF_UI_COLOR_TAB_HOVERED].convert_value<glm::vec4>());
    colors[ImGuiCol_TabActive] =
        convert_into_color(ui_color->scope[PREF_UI_COLOR_TAB_ACTIVE].convert_value<glm::vec4>());
    colors[ImGuiCol_TabUnfocused] =
        convert_into_color(ui_color->scope[PREF_UI_COLOR_TAB_UNFOCUSED].convert_value<glm::vec4>());
    colors[ImGuiCol_TabUnfocusedActive] = convert_into_color(
        ui_color->scope[PREF_UI_COLOR_TAB_UNFOCUSED_ACTIVE].convert_value<glm::vec4>()
    );

    colors[ImGuiCol_TitleBg] =
        convert_into_color(ui_color->scope[PREF_UI_COLOR_TITLE_BG].convert_value<glm::vec4>());
    colors[ImGuiCol_TitleBgActive] =
        convert_into_color(ui_color->scope[PREF_UI_COLOR_TITLE_BG_ACTIVE].convert_value<glm::vec4>()
        );
    colors[ImGuiCol_TitleBgCollapsed] = convert_into_color(
        ui_color->scope[PREF_UI_COLOR_TITLE_BG_COLLAPSED].convert_value<glm::vec4>()
    );
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
                    m_workspace->push_panel<PreferencesEditorPopup>();
                }
            }

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::End();
}

void DockingEditorWorkspace::_menu_open_window(std::string_view panel_name) {}

HierarchyEditorWorkspace::HierarchyEditorWorkspace() : PanelEditorWorkspaceBase("Hierarchy") {}

void HierarchyEditorWorkspace::on_imgui_update() {
    ImGui::Begin(get_name().c_str(), &get_enabled());

    entt::registry& registry = ogl::SceneManager::get()->get_active_scene()->get_registry();
    const entt::entity* entities = registry.data();

    entt::entity entity_clicked = get_non_selected_entity_value();
    for (size_t i = 0; i < registry.size(); i++) {
        ogl::Entity entity = ogl::Entity(entities[i]);
        ogl::NameComponent* name = entity.get_component<ogl::NameComponent>();

        int flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen |
                    ImGuiTreeNodeFlags_SpanFullWidth;
        if (m_selected_entity == entities[i]) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        if (name != nullptr) {
            ImGui::TreeNodeEx(
                reinterpret_cast<void*>(static_cast<intptr_t>(entities[i])), flags, "%s (%u)",
                name->name.c_str(), entities[i]
            );
        } else {
            ImGui::TreeNodeEx(
                reinterpret_cast<void*>(static_cast<intptr_t>(entities[i])), flags, "No Name: (%u)",
                entities[i]
            );
        }

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            entity_clicked = entities[i];
        }
    }

    if (entity_clicked != get_non_selected_entity_value()) {
        m_selected_entity = entity_clicked;
    }

    ImGui::End();
}

PropertiesEditorWorkspace::PropertiesEditorWorkspace(HierarchyEditorWorkspace* hierarchy)
    : PanelEditorWorkspaceBase("Properties"), m_hierarchy(hierarchy) {}

void PropertiesEditorWorkspace::on_imgui_update() {
    ImGui::Begin(get_name().c_str(), &get_enabled());

    if (m_hierarchy->get_selected_entity() != m_hierarchy->get_non_selected_entity_value()) {
        ogl::Entity entity = ogl::Entity(m_hierarchy->get_selected_entity());
        ogl::NameComponent* name = entity.get_component<ogl::NameComponent>();

        if (name != nullptr) {
            ImGui::Text(
                "selected entity '%s' with the id of %u", name->name.c_str(),
                m_hierarchy->get_selected_entity()
            );
        } else {
            ImGui::Text(
                "selected entity 'No Name' with the id of %u", m_hierarchy->get_selected_entity()
            );
        }
    }

    ImGui::End();
}

ConsoleEditorWorkspace::ConsoleEditorWorkspace(ogl::Debug* debug)
    : PanelEditorWorkspaceBase("Console") {
    std::string names[] = {"Messages", "Warnings", "Errors", "Fatal Errors", "Inits", "Terminate"};
    for (size_t i = 0; i < ogl::debug_type_count; i++) {
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

    ogl::YamlSerialization preferences = ogl::YamlSerialization(settings_path.c_str());
    m_font = io.Fonts->AddFontFromFileTTF(
        preferences.get(PREF_FIELD_EDITOR_UI)
            ->get(PREF_EDITOR_UI_FONT_MONO)
            ->convert_value<std::string>()
            .c_str(),
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
        if (std::get<bool>(m_filters[static_cast<size_t>(std::get<ogl::DebugType>(log))])) {
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

        // if (std::get<bool>(m_filters[static_cast<size_t>(std::get<ogl::DebugType>(log))])) {
        //     switch (std::get<ogl::DebugType>(log)) {
        //     case ogl::DebugType_Warning:
        //         ImGui::TextColored(
        //             ImVec4(
        //                 m_debug_colors[0].r, m_debug_colors[0].g, m_debug_colors[0].b,
        //                 m_debug_colors[0].a
        //             ),
        //             "[Warning (%f)]: %s", std::get<float>(log),
        //             std::get<std::string>(log).c_str()
        //         );
        //         break;
        //     case ogl::DebugType_Error:
        //         ImGui::TextColored(
        //             ImVec4(
        //                 m_debug_colors[1].r, m_debug_colors[1].g, m_debug_colors[1].b,
        //                 m_debug_colors[1].a
        //             ),
        //             "[Error (%f)]: %s", std::get<float>(log), std::get<std::string>(log).c_str()
        //         );
        //         break;
        //     case ogl::DebugType_Message:
        //         ImGui::Text(
        //             "[Message (%f)]: %s", std::get<float>(log),
        //             std::get<std::string>(log).c_str()
        //         );
        //         break;
        //     }
        // }
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

ViewportEditorWorkspace::ViewportEditorWorkspace(ogl::Framebuffer* framebuffer)
    : PanelEditorWorkspaceBase("Viewport") {
    if (framebuffer != nullptr) {
        m_framebuffer = framebuffer;
    } else {
        ogl::Debug::log("Viewport::Viewport(ogl::Framebuffer*) -> failed to create "
                        "viewport as framebuffer is nullptr");
    }
}

void ViewportEditorWorkspace::on_imgui_update() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin(get_name().c_str(), &get_enabled(), ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleVar();
    ImGui::BeginChild(1);

    if (m_framebuffer != nullptr) {
        ImVec2 window_size = ImGui::GetWindowSize();

        if (window_size.x != m_framebuffer->size.x || window_size.y != m_framebuffer->size.y) {
            m_framebuffer = ogl::Pipeline::get()->recreate_framebuffer(
                m_framebuffer, static_cast<int>(window_size.x), static_cast<int>(window_size.y)
            );
            if (m_framebuffer == nullptr) {
                ogl::Debug::log("Viewport::on_imgui_update() -> failed to resize "
                                "framebuffer size");
                return;
            }
        }

        uint64_t viewport_texture_id = static_cast<uint64_t>(m_framebuffer->texture);
        ImGui::Image(
            reinterpret_cast<void*>(viewport_texture_id),
            ImVec2(
                static_cast<float>(m_framebuffer->size.x), static_cast<float>(m_framebuffer->size.y)
            ),
            ImVec2(1, 1), ImVec2(0, 0)
        );
    } else {
        ImGui::Text("No Framebuffer Allocated");
    }
    ImGui::EndChild();

    ImGui::End();
}

} // namespace oge
