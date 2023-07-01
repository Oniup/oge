#ifndef __KRYOS_EDITOR_GUI_HIEARCHY_HPP__
#define __KRYOS_EDITOR_GUI_HIEARCHY_HPP__

#include "gui/editor.hpp"

#include <kryos/scene/entity.hpp>

namespace workspace {

class KHierarchy final : public KIWorkspace
{
  public:
    KHierarchy();
    virtual ~KHierarchy() override = default;

    inline ecs::Entity get_selected_entity() const { return m_selected_entity; }

    virtual void on_imgui_update() override;

  private:
    void _draw_entity(KEntity entity, ecs::Entity& entity_clicked, bool& opened_popup);
    void _create_shape(
        const std::string& new_entity_name, const std::string& mesh_file_path, KEntity* parent
    );
    void _popup_menu(KEntity* entity = nullptr);

    ecs::Entity m_selected_entity = ECS_ENTITY_DESTROYED;
    std::vector<KEntity> m_deleted_entity = {};
};

}

#endif
