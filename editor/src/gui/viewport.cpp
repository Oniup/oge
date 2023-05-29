#include "core/project.hpp"
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

    ImGui::Begin(get_name().c_str(), &get_enabled(), ImGuiWindowFlags_NoScrollbar);
    {
        ImGui::PopStyleVar();

        ogl::Scene* scene = ogl::SceneManager::get()->get_active_scene();
        ogl::CameraComponent* editor_camera = nullptr;

        if (scene != nullptr) {
            auto view = ecs::View<ogl::CameraComponent, ogl::TagComponent>(&scene->get_registry());
            for (ecs::Entity entity : view) {
                if (view.has_required(entity)) {
                    auto [camera, tag] = view.get();
                    if (std::string(tag->tag) == HIERARCHY_FILTER_NAME) {
                        editor_camera = camera;
                        break;
                    }
                }
            }
        } else {
            ImVec2 window_size = ImGui::GetWindowSize();
            ImVec2 text_size = ImGui::CalcTextSize("Create Empty Scene");
            ImGui::SetCursorPosX((window_size.x - text_size.x) * 0.5f);
            ImGui::SetCursorPosY((window_size.y - text_size.y) * 0.5f);

            // if (Project::get()->opened()) {
            if (ImGui::Button("Create Empty Scene")) {
                ogl::SceneManager::get()->set_active(ogl::SceneManager::get()->push("Empty Scene"));

                // Create Editor Camera for new scene
                ogl::Entity entity = ogl::Entity(true);
                entity.add_component<ogl::TagComponent>(HIERARCHY_FILTER_NAME);

                editor_camera = entity.add_component<ogl::CameraComponent>();
                editor_camera->clear_color = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
                editor_camera->is_main = true;
                // TODO: Set to Orthographic if in 2D mode
                editor_camera->projection_type = ogl::CameraProjection_Perspective;
            }
            // } else {
            // }
        }

        if (editor_camera != nullptr) {
            _camera_controller(editor_camera);
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
        }
    }

    ImGui::End();
}

void ViewportEditorWorkspace::_camera_controller(ogl::CameraComponent* camera) {
    glm::vec2 mouse_position = ogl::Input::get_mouse_position();
    static glm::vec2 last_mouse_position = mouse_position;

    if (camera != nullptr) {
        if (ogl::Input::pressed_key(ogl::InputKeyCode_LeftShift)) {
            if (ogl::Input::pressed_mousebutton(ogl::InputMouseButton_Right)) {
                constexpr float increase_speed = 0.3f;
                m_camera_move_speed = m_camera_move_speed +
                                      (increase_speed * ogl::Input::get_mouse_scroll_direction());
                if (m_camera_move_speed < 0.0f) {
                    m_camera_move_speed = 0.0f;
                }

                if (ogl::Input::pressed_key(ogl::InputKeyCode_S)) {
                    camera->position -=
                        m_camera_move_speed * camera->forward * ogl::Time::get_delta();
                }

                if (ogl::Input::pressed_key(ogl::InputKeyCode_W)) {
                    camera->position +=
                        m_camera_move_speed * camera->forward * ogl::Time::get_delta();
                }

                if (ogl::Input::pressed_key(ogl::InputKeyCode_D)) {
                    camera->position -= m_camera_move_speed *
                                        glm::cross(camera->forward, camera->up) *
                                        ogl::Time::get_delta();
                }

                if (ogl::Input::pressed_key(ogl::InputKeyCode_A)) {
                    camera->position += m_camera_move_speed *
                                        glm::cross(camera->forward, camera->up) *
                                        ogl::Time::get_delta();
                }

                if (ogl::Input::pressed_key(ogl::InputKeyCode_Q)) {
                    camera->position.y -= m_camera_move_speed * ogl::Time::get_delta();
                }

                if (ogl::Input::pressed_key(ogl::InputKeyCode_E)) {
                    camera->position.y += m_camera_move_speed * ogl::Time::get_delta();
                }
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

} // namespace oge
