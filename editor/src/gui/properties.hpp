#ifndef __KRYOS_ENGINE_GUI_PROPERTIES_HPP__
#define __KRYOS_ENGINE_GUI_PROPERTIES_HPP__

#include "gui/editor.hpp"
#include "gui/hierarchy.hpp"

#include <imgui/imgui.h>
#include <kryos/serialization/reflection.hpp>
#include <kryos/serialization/serialization.hpp>

namespace workspace {

typedef void (*fnptr_imgui_draw_property)(const std::string& fieldname, void* ptr, float step_size);

class KProperties final : public KIWorkspace
{
  public:
    KProperties(KHierarchy* hierarchy);
    virtual ~KProperties() override = default;

    virtual void on_imgui_update() override;

  private:
    void _initialize_draw_fnptrs(
        std::initializer_list<std::pair<std::uint64_t, fnptr_imgui_draw_property>> list
    );
    void _imgui_draw(KSerializeData& data);
    void
        _imgui_draw_std_vector(KSerializeData& data, const KTypeInfo& vector_inner_type_info);
    void _imgui_draw_array(
        KSerializeData& data, const KTypeInfo& type, fnptr_imgui_draw_property
    );
    void _imgui_draw_std_array(KSerializeData& data);
    void _imgui_draw_non_primitive(KSerializeData& data);

  private:
    KHierarchy* m_hierarchy = nullptr;
    std::unordered_map<std::uint64_t, fnptr_imgui_draw_property> m_draw_fnptrs = {};
    float m_step_size = 0.5f;
};

}

#endif
