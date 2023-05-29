#include "core/project.hpp"
#include "utils/utils.hpp"

#include <imgui/imgui.h>
#include <portable-file-dialogs/portable-file-dialogs.h>
#include <yaml/yaml.hpp>

namespace oge {

Project* Project::m_Instance = nullptr;

void Project::create_new_popup() {
    if (ImGui::BeginPopupModal("Create Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImVec2 window_size = ImVec2(
            static_cast<float>(ogl::Pipeline::get()->get_window()->get_width()) * 0.2f,
            static_cast<float>(ogl::Pipeline::get()->get_window()->get_height()) * 0.2f
        );

        ImGui::BeginChild("Create Project Correct Size", window_size);
        {
            static char project_name[yaml::Node::max_name_size()];
            static char folder_location[yaml::Node::max_line_size()];
            static bool is_3d_based = true;

            ImGui::BeginChild("Setting Names", ImVec2(window_size.x * 0.3f, 0.0f));
            {
                ImGui::Text("Name");
                ImGui::Text("Directory");
                ImGui::Text("Is 3D Based");
            }
            ImGui::EndChild();
            ImGui::SameLine();

            ImGui::BeginChild("Setting");
            {
                ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::InputText("###Name", project_name, yaml::Node::max_line_size());
                ImGui::PopItemWidth();

                ImGui::PushItemWidth(
                    ImGui::GetContentRegionAvail().x -
                    (ImGui::CalcTextSize("...").x + ImGui::GetStyle().FramePadding.x +
                     ImGui::GetStyle().WindowPadding.x + ImGui::GetStyle().ItemInnerSpacing.x)
                );
                ImGui::InputText("###Directory", folder_location, yaml::Node::max_line_size());
                ImGui::PopItemWidth();
                ImGui::SameLine();

                if (ImGui::Button("...")) {
                    const std::string& directory =
                        pfd::select_folder("New Project Location", pfd::path::home()).result();
                    strncpy(folder_location, directory.c_str(), directory.size());
                }

                ImGui::Checkbox("###3DBased", &is_3d_based);

                ImGui::SetCursorPosY(window_size.y - ImGuiHelper::calc_button_size().y);
                if (ImGui::Button(
                        "Create", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0.0f)
                    )) {
                    ImGui::CloseCurrentPopup();
                }

                ImGui::SameLine();

                if (ImGui::Button("Cancel", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();

        ImGui::EndPopup();
    }
}

Project::Project() {
    assert(
        m_Instance == nullptr && "Project::Project() -> cannot created multiple project "
                                 "application layers"
    );

    m_Instance = this;
}

bool Project::create(const std::string& name, const std::string& project_root_path) {
    m_name = name;
    m_root_path = project_root_path;
    if (project_root_path[project_root_path.size() - 1] == '/') {
        m_project_filename = project_root_path + m_name + ".oproject";
    } else {
        m_project_filename = project_root_path + "/" + m_name + ".oproject";
    }

    yaml::Node root = {};
    root << yaml::node("ProjectName", m_name) << yaml::node("SceneCount", 0);
    if (!root.write_file(m_project_filename)) {
        m_name.clear();
        m_root_path.clear();
        m_project_filename.clear();

        return false;
    }

    return true;
}

bool Project::load(const std::string& project_filename) {
    yaml::Node root = yaml::open(project_filename);
    if (!root.empty()) {
        m_name = yaml::Convert<std::string>().value(root["ProjectName"]);
        m_project_filename = project_filename;
        m_root_path = std::string(project_filename, project_filename.find_last_of('/') - 1);

        std::vector<std::string> scene_names =
            yaml::Convert<std::vector<std::string>>().value(root["SceneNames"]);

        return true;
    }

    return false;
}

void Project::serialize(const std::string& path) {}

void Project::_deserialize_scene(ogl::Scene* scene) {}
void Project::_serialize_scene(ogl::Scene* scene) {}

} // namespace oge
