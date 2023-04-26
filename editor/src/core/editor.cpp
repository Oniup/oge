#include "core/editor.hpp"

#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>

#include <GLFW/glfw3.h>

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

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Preferences")) {
            if (ImGui::BeginMenu("Open Window")) {
                const std::vector<PanelEditorWorkspaceBase*>& panels =
                    m_workspace->get_all_panels();

                for (PanelEditorWorkspaceBase* panel : panels) {
                    if (panel->get_name() != get_name()) {
                        ImGui::MenuItem(panel->get_name().c_str(), nullptr, &panel->get_enabled());
                    }
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::End();
}

HierarchyEditorWorkspace::HierarchyEditorWorkspace() : PanelEditorWorkspaceBase("Hierarchy") {}

void HierarchyEditorWorkspace::on_imgui_update() {
    ImGui::Begin(get_name().c_str(), &get_enabled());

    entt::registry& registry = ogl::SceneManager::get()->get_active_scene()->get_registry();
    const entt::entity* entities = registry.data();

    entt::entity entity_clicked = get_non_selected_entity_value();
    for (size_t i = 0; i < registry.size(); i++) {
        ogl::Entity entity = ogl::Entity(entities[i]);
        ogl::NameComponent* name = entity.get_component<ogl::NameComponent>();

        int flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
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

InspectorEditorWorkspace::InspectorEditorWorkspace(HierarchyEditorWorkspace* hierarchy)
    : PanelEditorWorkspaceBase("Inspector"), m_hierarchy(hierarchy) {}

void InspectorEditorWorkspace::on_imgui_update() {
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

    for (const std::tuple<ogl::DebugType, std::string, float>& log : m_debug->get_logs()) {
        if (std::get<bool>(m_filters[static_cast<size_t>(std::get<ogl::DebugType>(log))])) {
            switch (std::get<ogl::DebugType>(log)) {
            case ogl::DebugType_Warning:
                ImGui::TextColored(
                    ImVec4(
                        m_debug_colors[0].r, m_debug_colors[0].g, m_debug_colors[0].b,
                        m_debug_colors[0].a
                    ),
                    "[Warning (%f)]: %s", std::get<float>(log), std::get<std::string>(log).c_str()
                );
                break;
            case ogl::DebugType_Error:
                ImGui::TextColored(
                    ImVec4(
                        m_debug_colors[1].r, m_debug_colors[1].g, m_debug_colors[1].b,
                        m_debug_colors[1].a
                    ),
                    "[Error (%f)]: %s", std::get<float>(log), std::get<std::string>(log).c_str()
                );
                break;
            case ogl::DebugType_Message:
                ImGui::Text(
                    "[Message (%f)]: %s", std::get<float>(log), std::get<std::string>(log).c_str()
                );
                break;
            }
        }
    }

    if (m_auto_scrolling && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }

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
                ogl::Debug::log("Viewport::on_imgui_update() -> failed to resize framebuffer size");
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
        if (panel->get_enabled()) {
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

} // namespace oge
