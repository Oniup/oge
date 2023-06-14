#ifndef __OGE_GUI_PROPERTY_TYPES_HPP__
#define __OGE_GUI_PROPERTY_TYPES_HPP__

#include "gui/editor.hpp"

#include <imgui/imgui.h>
#include <ogl/ogl.hpp>

namespace oge {

class PropertyDefinition {
  public:
    static float VectorSliderMoveSpeed;

  public:
    PropertyDefinition(const std::string& type_name, std::size_t type_hash)
        : m_name(type_name), m_hash(type_hash) {}
    ~PropertyDefinition() = default;

    inline const std::string& get_name() const { return m_name; }
    inline std::size_t get_hash() const { return m_hash; }

    virtual void draw_imgui(ogl::Entity entity){};
    virtual void add_component(ogl::Entity entity){};
    virtual bool exists(ogl::Entity entity) { return false; };

  private:
    std::string m_name = {};
    std::size_t m_hash = 0;
};

template<typename _Component>
class PropertyTypeDefinition : public PropertyDefinition {
  public:
    PropertyTypeDefinition()
        : PropertyDefinition(
              ogl::TypeDescriptor<_Component>::get_name(), ogl::TypeDescriptor<_Component>::get_id()
          ) {}

    virtual void add_component(ogl::Entity entity) override { entity.add_component<_Component>(); }

    virtual bool exists(ogl::Entity entity) override {
        if (entity.get_component<_Component>() != nullptr) {
            return true;
        }
        return false;
    }
};

class PropertiesEditorWorkspace : public PanelEditorWorkspaceBase {
  public:
    PropertiesEditorWorkspace(HierarchyEditorWorkspace* hierarchy);
    virtual ~PropertiesEditorWorkspace() override;

    void push_properties(const std::initializer_list<PropertyDefinition*> list);

    virtual void on_imgui_update() override;

  private:
    HierarchyEditorWorkspace* m_hierarchy = nullptr;
    std::vector<PropertyDefinition*> m_properties = {};
};

} // namespace oge

#endif
