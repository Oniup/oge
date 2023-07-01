#ifndef __KRYOS_EDITOR_GUI_DOCKING_HPP__
#define __KRYOS_EDITOR_GUI_DOCKING_HPP__

#include "gui/editor.hpp"

namespace workspace {

class KDocking final : public KIWorkspace
{
  public:
    KDocking(KLEditorWorkspace* workspace);
    virtual ~KDocking() override = default;

    virtual void on_imgui_update() override;

  private:
    void _menu_open_window(const std::string& panel_name);

    class KLEditorWorkspace* m_workspace = nullptr;
    int m_dock_node_flags = 0;
    int m_window_flags = 0;
};

}

#endif
