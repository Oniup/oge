#include "core/project.hpp"
#include "gui/editor.hpp"
#include "gui/preferences.hpp"

#include <imgui/imgui.h>
#include <portable-file-dialogs/portable-file-dialogs.h>

namespace oge {

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
    bool scene_loaded = ogl::SceneManager::get()->get_active_scene() != nullptr;

    bool open_project_popup = false;

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::BeginMenu("New")) {
                if (ImGui::MenuItem("Project")) {
                    open_project_popup = true;
                }

                if (ImGui::MenuItem("Scene")) {
                }

                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                std::vector<std::string> files =
                    pfd::open_file(
                        "Open Project", pfd::path::home(),
                        {"Oniups Game Engine Project File (.oproject)", "*.oproject"}
                    )
                        .result();
                if (files.size() > 0) {
                    Project::get()->load(files[0]);
                }
            }

            if (ImGui::BeginMenu("Open Recents")) {
                yaml::Node preferences = Preferences::get_preferences();
                yaml::Node& project_node = preferences["Project"];
                std::vector<std::string> recently_opened =
                    project_node["RecentlyOpened"].as<std::vector<std::string>>();

                if (recently_opened.size() > 0) {
                    std::size_t last_size = recently_opened.size();

                    // Printing project select and remove button
                    for (std::size_t i = 0; i < recently_opened.size(); i++) {
                        yaml::Node project_config = yaml::open(recently_opened[i]);
                        if (project_config.empty()) {
                            recently_opened.erase(recently_opened.begin() + i);
                            continue;
                        }

                        if (ImGui::MenuItem(project_config["ProjectName"].as<std::string>().c_str()
                            )) {
                            if (!Project::get()->load(recently_opened[i])) {
                                recently_opened.erase(recently_opened.begin() + i);
                                project_node["RecentlyOpened"] = recently_opened;
                                preferences.write_file(Preferences::get_preferences_path());
                            }
                        }
                    }
                }

                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Save", "Ctrl+S", nullptr, scene_loaded)) {
            }

            if (ImGui::MenuItem("Save As", "Ctrl+Shift+S", nullptr, scene_loaded)) {
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
                }
            }

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (open_project_popup) {
        ImGui::OpenPopup("Create Project");
    }
    Project::create_new_popup();

    ImGui::End();
}

void DockingEditorWorkspace::_menu_open_window(const std::string& panel_name) {}

} // namespace oge
