#include "gui/properties.hpp"
#include "gui/editor.hpp"

#include <imgui/imgui.h>

namespace oge {

PropertiesEditorWorkspace::PropertiesEditorWorkspace(HierarchyEditorWorkspace* hierarchy)
    : PanelEditorWorkspaceBase("Properties"), m_hierarchy(hierarchy) {}

void PropertiesEditorWorkspace::on_imgui_update() {
    ImGui::Begin(get_name().c_str(), &get_enabled(), ImGuiWindowFlags_MenuBar);

    ImGui::End();
}

} // namespace oge
