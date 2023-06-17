#ifndef __OGE_GUI_PROPERTY_TYPES_HPP__
#define __OGE_GUI_PROPERTY_TYPES_HPP__

#include "gui/editor.hpp"

#include <imgui/imgui.h>
#include <ogl/ogl.hpp>

namespace oge {

class PropertiesEditorWorkspace : public PanelEditorWorkspaceBase {
  public:
    PropertiesEditorWorkspace(HierarchyEditorWorkspace* hierarchy);
    virtual ~PropertiesEditorWorkspace() override = default;

    virtual void on_imgui_update() override;

  private:
    HierarchyEditorWorkspace* m_hierarchy = nullptr;
};

} // namespace oge

#endif
