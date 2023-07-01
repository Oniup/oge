#ifndef __KRYOS_EDITOR_GUI_VIEWPORT_HPP__
#define __KRYOS_EDITOR_GUI_VIEWPORT_HPP__

#include "gui/editor.hpp"

#include <kryos/renderer/pipeline.hpp>
#include <kryos/scene/components.hpp>

namespace workspace {

class KViewport final : public KIWorkspace
{
  public:
    KViewport(KFramebuffer* framebuffer);
    virtual ~KViewport() override = default;

    virtual void on_imgui_update() override;

  private:
    void _camera_controller(KCCamera* camera);
    void _no_scene(KCCamera* camera, float window_width, float window_height);
    void _no_project();

    glm::ivec2 m_last_required_framebuffer_size = {};
    KFramebuffer* m_framebuffer = nullptr;

    float m_camera_move_speed = 5.0f;
    glm::vec2 m_camera_sensitivity = {0.05f, 0.05f};
    float m_yaw = 0.0f;
    float m_pitch = 0.0f;
};

} // namespace workspace

#endif
