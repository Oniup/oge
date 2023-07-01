#ifndef __KRYOS_EDITOR_GUI_CONSOLE_HPP__
#define __KRYOS_EDITOR_GUI_CONSOLE_HPP__

#include "gui/editor.hpp"

#include <kryos/core/debug.hpp>

namespace workspace {

class KConsole final : public KIWorkspace
{
  public:
    KConsole(KLDebug* debug);
    virtual ~KConsole() override = default;

    virtual void on_imgui_update() override;

  private:
    glm::vec4 m_debug_colors[2] = {
        glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)};
    std::tuple<bool, std::string> m_filters[debug_type_count] = {};
    bool m_auto_scrolling = true;
    KLDebug* m_debug = nullptr;
    ImFont* m_font = nullptr;
};

}

#endif
