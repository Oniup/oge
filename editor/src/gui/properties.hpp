#ifndef __OGE_GUI_PROPERTY_TYPES_HPP__
#define __OGE_GUI_PROPERTY_TYPES_HPP__

#include "gui/editor.hpp"

#include <imgui/imgui.h>
#include <ogl/ogl.hpp>

namespace oge {

typedef void (*fnptr_imgui_draw_property)(const std::string& fieldname, void* ptr, float step_size);

class PropertiesEditorWorkspace : public PanelEditorWorkspaceBase
{
  public:
    PropertiesEditorWorkspace(HierarchyEditorWorkspace* hierarchy);
    virtual ~PropertiesEditorWorkspace() override = default;

    virtual void on_imgui_update() override;

  private:
    struct StructDrawData
    {
        std::byte* object;
        std::set<ogl::MemberInfo>::iterator it;
        std::set<ogl::MemberInfo>::iterator end;
        ogl::ReflectionRegistry* reflection;
    };

    void _initialize_draw_fnptrs(
        std::initializer_list<std::pair<std::uint64_t, fnptr_imgui_draw_property>> list
    );
    void _imgui_draw(StructDrawData& data);
    void _imgui_draw_vector(StructDrawData& data, const ogl::TypeInfo& vector_inner_type_info);
    void _imgui_draw_non_primitive(StructDrawData& data);

    HierarchyEditorWorkspace* m_hierarchy = nullptr;
    std::unordered_map<std::uint64_t, fnptr_imgui_draw_property> m_draw_fnptrs = {};
    float m_step_size = 0.5f;
};

} // namespace oge

#endif
