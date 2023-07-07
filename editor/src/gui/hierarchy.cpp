#include "gui/hierarchy.hpp"

#include <kryos/core/asset_handler.hpp>
#include <kryos/scene/components.hpp>
#include <kryos/scene/scene_manager.hpp>

#include <imgui/imgui.h>

namespace workspace {

KHierarchy::KHierarchy() : KIWorkspace("Hierarchy") {}

void KHierarchy::on_imgui_update()
{
    ImGui::Begin(get_name().c_str(), &get_enabled());

    KScene* active_scene = KIApplication::get_layer<KLSceneManager>()->get_active_scene();
    if (active_scene != nullptr)
    {
        ecs::Registry& registry = active_scene->get_registry();
        const std::vector<ecs::Entity>& entities = registry.get_entities();

        ecs::Entity entity_clicked = ECS_ENTITY_DESTROYED;
        bool opened_targeted_entity_popup = false;
        for (std::size_t i = 0; i < entities.size(); i++)
        {
            KEntity entity = KEntity(entities[i]);
            if (entity)
            {
                KCTag* tag = entity.get_component<KCTag>();
                bool include_ent = true;
                if (tag != nullptr)
                {
                    if (tag->tag.size() > 0)
                    {
                        // PERFORMANCE: try to avoid string comparison
                        if (std::string(HIERARCHY_FILTER_NAME) == tag->tag)
                            include_ent = false;
                    }
                }

                if (include_ent)
                    _draw_entity(entity, entity_clicked, opened_targeted_entity_popup);
            }
        }

        if (entity_clicked != ECS_ENTITY_DESTROYED)
            m_selected_entity = entity_clicked;

        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
            m_selected_entity = ECS_ENTITY_DESTROYED;

        if (!opened_targeted_entity_popup)
        {
            if (ImGui::BeginPopupContextWindow())
            {
                _popup_menu();
                ImGui::EndPopup();
            }
        }
    }

    ImGui::End();
}

void KHierarchy::_draw_entity(KEntity entity, ecs::Entity& entity_clicked, bool& opened_popup)
{
    int flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen |
                ImGuiTreeNodeFlags_SpanFullWidth;
    if (m_selected_entity == entity)
        flags |= ImGuiTreeNodeFlags_Selected;

    KCName* name = entity.get_component<KCName>();
    std::string str = "Entity";
    if (name != nullptr)
        str = name->name;

    ecs::Entity id = entity;
    ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(id)), flags, "%s", str.c_str());

    if (ImGui::BeginPopupContextItem())
    {
        opened_popup = true;
        _popup_menu(&entity);
        ImGui::EndPopup();
    }

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        entity_clicked = id;
}

void KHierarchy::_create_shape(
    const std::string& new_entity_name, const std::string& mesh_name, KEntity* parent
)
{
    KEntity entity{};
    KCName* name = entity.add_component<KCName>(new_entity_name);

    KCMeshRenderer* mesh_renderer = entity.add_component<KCMeshRenderer>();
    mesh_renderer->model = KIApplication::get_layer<KLAssetHandler>()->get_static_model(mesh_name);

    if (parent != nullptr)
    {
        // TODO: Setup parent component
    }
}

void KHierarchy::_popup_menu(KEntity* entity)
{
    KScene* scene = KIApplication::get_layer<KLSceneManager>()->get_active_scene();

    if (scene != nullptr)
    {
        if (ImGui::BeginMenu("New"))
        {
            if (ImGui::MenuItem("Entity"))
                KEntity creating{};

            if (ImGui::BeginMenu("Shape"))
            {
                if (ImGui::MenuItem("Cube"))
                    _create_shape("Cube", "cube", entity);
                if (ImGui::MenuItem("Sphere"))
                    _create_shape("Sphere", "sphere", entity);
                if (ImGui::MenuItem("Plane"))
                    _create_shape("Plane", "plane", entity);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (entity != nullptr)
        {
            if (ImGui::MenuItem("Delete"))
                entity->destroy();
        }
    }
    else
    {
        if (ImGui::MenuItem("Create Scene"))
        {
            // TODO: ...
        }
    }
}

} // namespace workspace
