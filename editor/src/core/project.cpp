#include "core/project.hpp"
#include "gui/preferences.hpp"
#include "utils/utils.hpp"

#include <filesystem>
#include <imgui/imgui.h>
#include <portable-file-dialogs/portable-file-dialogs.h>
#include <yaml/yaml.hpp>

Project* Project::m_Instance = nullptr;

void Project::create_new_popup()
{
    if (ImGui::BeginPopupModal("Create Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        kryos::Window* window = kryos::Application::get_layer<kryos::Window>();
        ImVec2 window_size = ImVec2(
            static_cast<float>(window->get_width()) * 0.2f,
            static_cast<float>(window->get_height()) * 0.2f
        );

        ImGui::BeginChild("Create Project Correct Size", window_size);
        {
            static char ProjectName[yaml::Node::max_name_size()];
            static char FolderLocation[yaml::Node::max_line_size()];
            static bool Is3DBased = true;
            static std::string error_message = "";
            ImVec2 error_message_size = ImVec2();

            if (error_message.size() > 0)
            {
                error_message_size = ImGui::CalcTextSize(error_message.c_str());
                ImGui::SetCursorPosX(
                    (ImGui::GetContentRegionAvail().x - error_message_size.x) * 0.5f
                );

                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                ImGui::Text("%s", error_message.c_str());
                ImGui::PopStyleColor();
            }

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
                ImGui::InputText("###Name", ProjectName, yaml::Node::max_line_size());
                ImGui::PopItemWidth();

                ImGui::PushItemWidth(
                    ImGui::GetContentRegionAvail().x -
                    (ImGui::CalcTextSize("...").x + ImGui::GetStyle().FramePadding.x +
                     ImGui::GetStyle().WindowPadding.x + ImGui::GetStyle().ItemInnerSpacing.x)
                );
                ImGui::InputText("###Directory", FolderLocation, yaml::Node::max_line_size());
                ImGui::PopItemWidth();
                ImGui::SameLine();

                if (ImGui::Button("..."))
                {
                    const std::string& directory =
                        pfd::select_folder("New Project Location", pfd::path::home()).result();
                    strncpy(FolderLocation, directory.c_str(), directory.size());
                }

                ImGui::Checkbox("###3DBased", &Is3DBased);

                if (error_message.size() > 0)
                {
                    ImGui::SetCursorPosY(
                        window_size.y - (ImGuiHelper::calc_button_size().y + error_message_size.y +
                                         ImGui::GetStyle().FramePadding.y * 2)
                    );
                }
                else
                    ImGui::SetCursorPosY(window_size.y - ImGuiHelper::calc_button_size().y);

                if (ImGui::Button("Create", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0.0f)))
                {
                    if (std::string(ProjectName).size() > 0 &&
                        std::string(FolderLocation).size() > 0)
                    {

                        if (Project::get()->create(ProjectName, FolderLocation, Is3DBased))
                            ImGui::CloseCurrentPopup();
                        else
                        {
                            ProjectName[0] = '\0';
                            FolderLocation[0] = '\0';
                            Is3DBased = true;

                            error_message = std::string("Error. Could not create project ") +
                                            ProjectName + " at " + FolderLocation;
                        }
                    }
                    else
                        error_message = "Invalid project details, must fill every field in";
                }

                ImGui::SameLine();

                if (ImGui::Button("Cancel", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
                {
                    ProjectName[0] = '\0';
                    FolderLocation[0] = '\0';
                    Is3DBased = true;
                    error_message = "";
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();

        ImGui::EndPopup();
    }
}

Project::Project()
{
    assert(
        m_Instance == nullptr && "Project::Project() -> cannot created multiple project "
                                 "application layers"
    );

    m_Instance = this;
}

bool Project::create(
    const std::string& name, const std::string& project_root_path, bool is_3d_based
)
{
    m_name = name;
    m_root_path = project_root_path;
    m_3d_based = is_3d_based;

    if (project_root_path[project_root_path.size() - 1] == '/')
    {
        m_root_path += m_name + "/";
        m_project_filename = m_root_path + m_name + ".kryosproject";
    }
    else
    {
        m_root_path += "/" + m_name + "/";
        m_project_filename = m_root_path + m_name + ".kryosproject";
    }

    if (std::filesystem::create_directory(m_root_path))
    {
        yaml::Node root = {};
        root << yaml::node("ProjectName", m_name) << yaml::node("SceneCount", 0)
             << yaml::node("Scenes");

        if (yaml::write(root, m_project_filename))
        {
            kryos::Application::get_layer<kryos::Window>()->set_title("Kryos - " + m_name);

            yaml::Node preferences = Preferences::get_preferences();
            yaml::Node& project_node = preferences["Project"];
            std::vector<std::string> recently_opened =
                project_node["RecentlyOpened"].as<std::vector<std::string>>();
            recently_opened.push_back(m_project_filename);
            project_node["RecentlyOpened"] = recently_opened;
            preferences.write_file(Preferences::get_preferences_path());

            return true;
        }
        else
            kryos::Debug::log(
                "Failed to create project: cannot write project yaml file to '" +
                    m_project_filename = "'",
                kryos::DebugType_Error
            );
    }
    else
        kryos::Debug::log(
            "Failed to create project: cannot create directory '" + m_root_path + "'",
            kryos::DebugType_Error
        );

    m_name.clear();
    m_root_path.clear();
    m_project_filename.clear();
    m_unsaved = false;
    return false;
}

bool Project::load(const std::string& project_filename)
{
    yaml::Node root = yaml::open(project_filename);
    if (!root.empty())
    {
        m_name = root["ProjectName"].as<std::string>();
        m_project_filename = project_filename;
        m_root_path = std::string(project_filename.c_str(), project_filename.find_last_of('/'));
        m_unsaved = false;

        kryos::Application::get_layer<kryos::Window>()->set_title("Kryos - " + m_name);

        if (root["SceneCount"].as<std::size_t>() > 0)
            // TODO: load scenes

            return true;
    }
    return false;
}

void Project::serialize(const std::string& filename, bool use_scene_name)
{
    yaml::Node project_node = yaml::open(m_project_filename);
    if (!project_node.empty())
    {
        kryos::SerializableTypeHandler* handler =
            kryos::Application::get_layer<kryos::SerializableTypeHandler>();
        kryos::SceneManager* scene_manager = kryos::Application::get_layer<kryos::SceneManager>();

        kryos::Scene* scene = scene_manager->get_active_scene();
        if (!use_scene_name)
        {
            std::size_t name_offset = filename.find_last_of('/') + 1;
            std::size_t scene_name_size = filename.find_last_of('.') - name_offset;
            std::string scene_name = std::string(filename.c_str() + name_offset, scene_name_size);
            scene->set_name(scene_name);
        }

        handler->serialize_text(scene, filename);
        m_unsaved = false;

        // Updating project file so it knows about the knewly created scene
        std::size_t scenes_count = project_node["SceneCount"].as<std::size_t>();

        yaml::Node& scenes_node = project_node["Scenes"];
        scenes_node << yaml::node(scene->get_name(), filename);
        project_node["SceneCount"] = scenes_count + 1;

        project_node.write_file(m_project_filename);
    }
    else
        kryos::Debug::log(
            "Cannot serialize scene, cannot open Project file", kryos::DebugType_Error
        );
}

void Project::deserialize(kryos::Scene* scene, const std::string& filename) {}
