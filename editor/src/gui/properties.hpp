#ifndef __KRYOS_ENGINE_GUI_PROPERTY_TYPES_HPP__
#define __KRYOS_ENGINE_GUI_PROPERTY_TYPES_HPP__

#include "gui/editor.hpp"

#include <imgui/imgui.h>
#include <kryos/kryos.hpp>

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
        std::set<kryos::MemberInfo>::iterator it;
        std::set<kryos::MemberInfo>::iterator end;
        kryos::ReflectionRegistry* reflection;
    };

    void _initialize_draw_fnptrs(
        std::initializer_list<std::pair<std::uint64_t, fnptr_imgui_draw_property>> list
    );
    void _imgui_draw(StructDrawData& data);
    void _imgui_draw_std_vector(StructDrawData& data, const kryos::TypeInfo& vector_inner_type_info);
    void _imgui_draw_array(StructDrawData& data, const kryos::TypeInfo& type, fnptr_imgui_draw_property);
    void _imgui_draw_std_array(StructDrawData& data);
    void _imgui_draw_non_primitive(StructDrawData& data);

    HierarchyEditorWorkspace* m_hierarchy = nullptr;
    std::unordered_map<std::uint64_t, fnptr_imgui_draw_property> m_draw_fnptrs = {};
    float m_step_size = 0.5f;
};

#endif
