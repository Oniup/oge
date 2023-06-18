#ifndef __OGE_GUI_PROPERTY_TYPES_HPP__
#define __OGE_GUI_PROPERTY_TYPES_HPP__

#include "gui/editor.hpp"

#include <imgui/imgui.h>
#include <ogl/ogl.hpp>

namespace oge {

typedef void (*fnptr_imgui_draw_comp)(const std::string& fieldname, void* ptr);

class PropertiesEditorWorkspace : public PanelEditorWorkspaceBase
{
  public:
    PropertiesEditorWorkspace(HierarchyEditorWorkspace* hierarchy);
    virtual ~PropertiesEditorWorkspace() override = default;

    virtual void on_imgui_update() override;

  private:
    void _initialize_draw_fnptrs(
        std::initializer_list<std::pair<std::uint64_t, fnptr_imgui_draw_comp>> list
    );
    void _imgui_draw(
        ecs::byte* object, std::set<ogl::MemberInfo>::iterator it,
        const std::set<ogl::MemberInfo>::iterator end, ogl::ReflectionRegistry* reflection,
        const float width
    );

    HierarchyEditorWorkspace* m_hierarchy = nullptr;
    std::unordered_map<std::uint64_t, fnptr_imgui_draw_comp> m_draw_fnptrs = {};
};

} // namespace oge

#endif
