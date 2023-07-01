#include "gui/viewport.hpp"
#include "core/project.hpp"
#include "gui/editor.hpp"
#include "gui/preferences.hpp"
#include "utils/utils.hpp"

#include <kryos/core/input.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <portable-file-dialogs/portable-file-dialogs.h>

namespace workspace {

KViewport::KViewport(KFramebuffer* framebuffer) : KIWorkspace("Viewport")
{
    if (framebuffer != nullptr)
        m_framebuffer = framebuffer;
    else
        KLDebug::log("Viewport::Viewport(Framebuffer*) -> failed to create "
                     "viewport as framebuffer is nullptr");
}

void KViewport::on_imgui_update()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    int window_flags = ImGuiWindowFlags_NoScrollbar;
    if (KLProject::get()->unsaved())
        window_flags |= ImGuiWindowFlags_UnsavedDocument;

    ImGui::Begin(get_name().c_str(), &get_enabled(), window_flags);
    {
        ImGui::PopStyleVar();

        KScene* scene = KIApplication::get_layer<KLSceneManager>()->get_active_scene();
        KCCamera* editor_camera = nullptr;

        if (scene != nullptr)
        {
            auto view = ecs::View<KCCamera, KCTag>(&scene->get_registry());
            for (ecs::Entity entity : view)
            {
                if (view.has_required(entity))
                {
                    auto [camera, tag] = view.get();
                    if (std::string(tag->tag) == HIERARCHY_FILTER_NAME)
                    {
                        editor_camera = camera;
                        break;
                    }
                }
            }
        }
        else
        {
            ImVec2 window_size = ImGui::GetWindowSize();

            if (KLProject::get()->opened())
                _no_scene(editor_camera, window_size.x, window_size.y);
            else
                _no_project();
        }

        if (editor_camera != nullptr)
        {
            _camera_controller(editor_camera);
            ImGui::BeginChild(1);
            {
                if (m_framebuffer != nullptr)
                {
                    ImVec2 window_size = ImGui::GetWindowSize();

                    if (window_size.x != m_framebuffer->size.x ||
                        window_size.y != m_framebuffer->size.y)
                    {
                        m_framebuffer =
                            KIApplication::get_layer<KLPipeline>()->recreate_framebuffer(
                                m_framebuffer, static_cast<int>(window_size.x),
                                static_cast<int>(window_size.y)
                            );
                        if (m_framebuffer == nullptr)
                        {
                            KLDebug::log("Viewport::on_imgui_update() -> failed to resize "
                                         "framebuffer size");
                            return;
                        }
                    }

                    uint64_t viewport_texture_id = static_cast<uint64_t>(m_framebuffer->texture);
                    ImGui::Image(
                        reinterpret_cast<void*>(viewport_texture_id),
                        ImVec2(
                            static_cast<float>(m_framebuffer->size.x),
                            static_cast<float>(m_framebuffer->size.y)
                        ),
                        ImVec2(1, 1), ImVec2(0, 0)
                    );
                }
                else
                    ImGui::Text("No Framebuffer Allocated");
            }
            ImGui::EndChild();
        }
    }

    ImGui::End();
}

void KViewport::_camera_controller(KCCamera* camera)
{
    glm::vec2 mouse_position = KInput::get_mouse_position();
    static glm::vec2 last_mouse_position = mouse_position;

    if (camera != nullptr)
    {
        if (KInput::pressed_key(InputKeyCode_LeftShift))
        {
            if (KInput::pressed_mousebutton(InputMouseButton_Right))
            {
                constexpr float increase_speed = 0.3f;
                m_camera_move_speed =
                    m_camera_move_speed + (increase_speed * KInput::get_mouse_scroll_direction());
                if (m_camera_move_speed < 0.0f)
                    m_camera_move_speed = 0.0f;

                if (KInput::pressed_key(InputKeyCode_S))
                    camera->position -= m_camera_move_speed * camera->forward * KTime::get_delta();

                if (KInput::pressed_key(InputKeyCode_W))
                    camera->position += m_camera_move_speed * camera->forward * KTime::get_delta();

                if (KInput::pressed_key(InputKeyCode_D))
                    camera->position -= m_camera_move_speed *
                                        glm::cross(camera->forward, camera->up) *
                                        KTime::get_delta();

                if (KInput::pressed_key(InputKeyCode_A))
                    camera->position += m_camera_move_speed *
                                        glm::cross(camera->forward, camera->up) *
                                        KTime::get_delta();

                if (KInput::pressed_key(InputKeyCode_Q))
                    camera->position.y -= m_camera_move_speed * KTime::get_delta();

                if (KInput::pressed_key(InputKeyCode_E))
                    camera->position.y += m_camera_move_speed * KTime::get_delta();
                glm::vec2 mouse_movement = last_mouse_position - mouse_position;

                mouse_movement *= 0.05f;

                m_yaw += mouse_movement.x;
                m_pitch += mouse_movement.y;
                m_pitch = std::clamp(m_pitch, -89.0f, 90.0f);

                camera->forward = glm::vec3(
                    glm::cos(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch)),
                    glm::sin(glm::radians(m_pitch)),
                    glm::sin(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch))
                );
            }
        }
    }

    last_mouse_position = mouse_position;
}

void KViewport::_no_scene(KCCamera* editor_camera, float window_width, float window_height)
{
    ImVec2 text_size = ImGui::CalcTextSize("Create Empty Scene");
    ImGui::SetCursorPosX((window_width - text_size.x) * 0.5f);
    ImGui::SetCursorPosY((window_height - text_size.y) * 0.5f);

    if (ImGui::Button("Create Empty Scene"))
    {
        KIApplication::get_layer<KLSceneManager>()->set_active(
            KIApplication::get_layer<KLSceneManager>()->push("Empty Scene")
        );

        // Create Editor Camera for new scene
        KEntity entity = KEntity(true);
        entity.add_component<KCTag>(HIERARCHY_FILTER_NAME);

        editor_camera = entity.add_component<KCCamera>();
        editor_camera->clear_color = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
        editor_camera->is_main = true;
        // TODO: Set to Orthographic if in 2D mode
        editor_camera->projection_type = CameraProjection_Perspective;
        KLProject::get()->unsaved() = true;
    }
}

void KViewport::_no_project()
{
    ImVec2 available_space = ImGui::GetContentRegionAvail();
    ImVec2 child_window_size = ImVec2(available_space.x * 0.5f, available_space.y * 0.5f);
    ImGui::SetCursorPosX((available_space.x - child_window_size.x) * 0.5f);
    ImGui::SetCursorPosY((available_space.y - child_window_size.y) * 0.5f);

    ImGui::BeginChild("No Project Enabled", child_window_size, true);
    {
        ImGui::BeginChild("No Project Enabled -> Edit", ImVec2(child_window_size.x * 0.5f, 0.0f));
        {
            float button_width = ImGui::GetContentRegionAvail().x;
            float button_height = ImGuiHelper::calc_button_size().y;

            // Calc button offset so it is centered
            ImGui::SetCursorPosY((child_window_size.y - button_height * 3.0f) * 0.5f);

            if (ImGui::Button("Create New Project", ImVec2(button_width, 0.0f)))
                ImGui::OpenPopup("Create Project");

            KLProject::create_new_popup();

            if (ImGui::Button("Open Project", ImVec2(button_width, 0.0f)))
            {
                std::vector<std::string> files =
                    pfd::open_file(
                        "Open Project", pfd::path::home(),
                        {"Kryos Project File (.kryosproject)", "*.kryosproject"}
                    )
                        .result();
                if (files.size() > 0)
                {
                    if (KLProject::get()->load(files[0]))
                    {
                        // Update preferences config
                        yaml::Node project_config = yaml::open(files[0]);
                        yaml::Node preferences = Preferences::get_preferences();
                        yaml::Node& project = preferences["Project"];

                        std::vector<std::string> recently_opened =
                            project["RecentlyOpened"].as<std::vector<std::string>>();
                        bool found = false;
                        for (const std::string& name : recently_opened)
                        {
                            if (name == project_config["ProjectName"].as<std::string>())
                            {
                                found = true;
                                break;
                            }
                        }

                        if (!found)
                        {
                            recently_opened.push_back(files[0]);
                            project["RecentlyOpened"] = recently_opened;
                            preferences.write_file(Preferences::get_preferences_path());
                        }
                    }
                }
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // List all recently opened projects, allow removing projects from the list
        ImGui::BeginChild("No Project Enabled -> Recently Opened", ImVec2(0.0f, 0.0f));
        {
            yaml::Node preferences = Preferences::get_preferences();
            yaml::Node& project_node = preferences["Project"];
            std::vector<std::string> recently_opened =
                project_node["RecentlyOpened"].as<std::vector<std::string>>();

            float text_width = ImGui::CalcTextSize("Recent Projects").x;
            ImGui::SetCursorPosX((child_window_size.x * 0.5f - text_width) * 0.5f);
            ImGui::Text("Recent Projects");

            ImGui::BeginChild(
                "No Project Enabled -> Recently Opened List", ImVec2(0.0f, 0.0f), true
            );
            if (recently_opened.size() > 0)
            {
                std::size_t last_size = recently_opened.size();

                // Printing project select and remove button
                for (std::size_t i = 0; i < recently_opened.size(); i++)
                {
                    yaml::Node project_config = yaml::open(recently_opened[i]);
                    if (project_config.empty())
                    {
                        recently_opened.erase(recently_opened.begin() + i);
                        continue;
                    }

                    if (ImGui::Button(
                            project_config["ProjectName"].as<std::string>().c_str(),
                            ImVec2(
                                ImGui::GetContentRegionAvail().x -
                                    (ImGui::CalcTextSize("X").x + ImGui::GetStyle().FramePadding.x +
                                     ImGui::GetStyle().WindowPadding.x +
                                     ImGui::GetStyle().ItemInnerSpacing.x),
                                0.0f
                            )
                        ))
                    {
                        if (!KLProject::get()->load(recently_opened[i]))
                            recently_opened.erase(recently_opened.begin() + i);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("X"))
                    {
                        recently_opened.erase(recently_opened.begin() + i);
                    }
                }

                // Update preferences config
                if (recently_opened.size() != last_size)
                {
                    project_node["RecentlyOpened"] = recently_opened;
                    assert(
                        yaml::write(preferences, Preferences::get_preferences_path()) &&
                        "failed to write to preferences when editing recently opened projects"
                    );
                }
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();
    }
    ImGui::EndChild();
}

} // namespace workspace
