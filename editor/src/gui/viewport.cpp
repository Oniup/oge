#include "gui/editor.hpp"

#include <imgui/imgui.h>

namespace oge {

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

    _camera_controller();

    ImGui::Begin(get_name().c_str(), &get_enabled(), ImGuiWindowFlags_NoScrollbar);
    {
        ImGui::PopStyleVar();

        if (m_camera != nullptr) {
            if (m_scene_main_camera == nullptr) {
                ogl::Scene* scene = ogl::SceneManager::get()->get_active_scene();
                // setting to the correct clear color
                auto camera_view = scene->get_registry().view<ogl::CameraComponent>();
                for (entt::entity entity : camera_view) {
                    ogl::CameraComponent& camera = camera_view.get<ogl::CameraComponent>(entity);

                    if (camera.is_main) {
                        m_scene_main_camera = &camera;
                        break;
                    }
                }
            }

            m_camera->clear_color = m_scene_main_camera->clear_color;

            ImGui::BeginChild(1);
            {
                if (m_framebuffer != nullptr) {
                    ImVec2 window_size = ImGui::GetWindowSize();

                    if (window_size.x != m_framebuffer->size.x ||
                        window_size.y != m_framebuffer->size.y) {
                        m_framebuffer = ogl::Pipeline::get()->recreate_framebuffer(
                            m_framebuffer, static_cast<int>(window_size.x),
                            static_cast<int>(window_size.y)
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
                            static_cast<float>(m_framebuffer->size.x),
                            static_cast<float>(m_framebuffer->size.y)
                        ),
                        ImVec2(1, 1), ImVec2(0, 0)
                    );
                } else {
                    ImGui::Text("No Framebuffer Allocated");
                }
            }
            ImGui::EndChild();

        } else {
            ogl::Scene* scene = ogl::SceneManager::get()->get_active_scene();

            if (scene != nullptr) {
                auto view = scene->get_registry().view<ogl::CameraComponent, ogl::TagComponent>();

                for (entt::entity entity : view) {
                    ogl::TagComponent& tag = view.get<ogl::TagComponent>(entity);
                    if (strncmp(tag.tag, HIERARCHY_FILTER_NAME, strlen(tag.tag)) == 0) {
                        m_camera = &view.get<ogl::CameraComponent>(entity);
                    }
                }

                if (m_camera == nullptr) {
                    ogl::Entity editor_camera{true};
                    ogl::CameraComponent* camera_comp =
                        editor_camera.add_component<ogl::CameraComponent>();
                    editor_camera.add_component<ogl::TagComponent>(HIERARCHY_FILTER_NAME);
                    camera_comp->clear_color = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
                    camera_comp->is_main = true;
                    // TODO: Set to Orthographic if in 2D mode
                    camera_comp->projection_type = ogl::CameraProjection_Perspective;
                }
            } else {
                ImVec2 spacing = ImGui::GetContentRegionAvail();
                spacing = ImVec2(spacing.x / 2, spacing.y / 2);
                if (ImGui::Button("Create Empty Scene")) {
                    ogl::Debug::log(
                        "Create New Empty Scene, [This is will implemented when working on "
                        "project functionality]"
                    );
                }
            }
        }
    }
    ImGui::End();
}

void ViewportEditorWorkspace::_camera_controller() {
    glm::vec2 mouse_position = ogl::Input::get_mouse_position();
    static glm::vec2 last_mouse_position = mouse_position;

    if (m_camera != nullptr) {
        if (ogl::Input::pressed_key(ogl::InputKeyCode_LeftShift)) {
            if (ogl::Input::pressed_mousebutton(ogl::InputMouseButton_Right)) {
                constexpr float increase_speed = 0.3f;
                m_camera_move_speed = m_camera_move_speed + (increase_speed * ogl::Input::get_mouse_scroll_direction());
                if (m_camera_move_speed < 0.0f) {
                    m_camera_move_speed = 0.0f;
                }

                if (ogl::Input::pressed_key(ogl::InputKeyCode_S)) {
                    m_camera->position -=
                        m_camera_move_speed * m_camera->forward * ogl::Time::get_delta();
                }

                if (ogl::Input::pressed_key(ogl::InputKeyCode_W)) {
                    m_camera->position +=
                        m_camera_move_speed * m_camera->forward * ogl::Time::get_delta();
                }

                if (ogl::Input::pressed_key(ogl::InputKeyCode_D)) {
                    m_camera->position -= m_camera_move_speed *
                                          glm::cross(m_camera->forward, m_camera->up) *
                                          ogl::Time::get_delta();
                }

                if (ogl::Input::pressed_key(ogl::InputKeyCode_A)) {
                    m_camera->position += m_camera_move_speed *
                                          glm::cross(m_camera->forward, m_camera->up) *
                                          ogl::Time::get_delta();
                }

                if (ogl::Input::pressed_key(ogl::InputKeyCode_Q)) {
                    m_camera->position.y -= m_camera_move_speed * ogl::Time::get_delta();
                }

                if (ogl::Input::pressed_key(ogl::InputKeyCode_E)) {
                    m_camera->position.y += m_camera_move_speed * ogl::Time::get_delta();
                }
                glm::vec2 mouse_movement = last_mouse_position - mouse_position;

                mouse_movement *= 0.05f;

                m_yaw += mouse_movement.x;
                m_pitch += mouse_movement.y;
                m_pitch = std::clamp(m_pitch, -89.0f, 90.0f);

                m_camera->forward = glm::vec3(
                    glm::cos(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch)),
                    glm::sin(glm::radians(m_pitch)),
                    glm::sin(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch))
                );
            }
        }
    }

    last_mouse_position = mouse_position;
}

} // namespace oge
