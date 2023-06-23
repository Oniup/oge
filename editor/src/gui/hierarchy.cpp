#include "gui/editor.hpp"

#include <imgui/imgui.h>

HierarchyEditorWorkspace::HierarchyEditorWorkspace() : PanelEditorWorkspaceBase("Hierarchy") {}

void HierarchyEditorWorkspace::on_imgui_update()
{
    ImGui::Begin(get_name().c_str(), &get_enabled());

    kryos::Scene* active_scene = kryos::Application::get_layer<kryos::SceneManager>()->get_active_scene();
    if (active_scene != nullptr)
    {
        ecs::Registry& registry = active_scene->get_registry();
        const std::vector<ecs::Entity>& entities = registry.get_entities();

        ecs::Entity entity_clicked = ECS_ENTITY_DESTROYED;
        bool opened_targeted_entity_popup = false;
        for (std::size_t i = 0; i < entities.size(); i++)
        {
            kryos::Entity entity = kryos::Entity(entities[i]);
            if (entity)
            {
                kryos::TagComponent* tag = entity.get_component<kryos::TagComponent>();
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

void HierarchyEditorWorkspace::_draw_entity(
    kryos::Entity entity, ecs::Entity& entity_clicked, bool& opened_popup
)
{
    int flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen |
                ImGuiTreeNodeFlags_SpanFullWidth;
    if (m_selected_entity == entity)
        flags |= ImGuiTreeNodeFlags_Selected;

    kryos::NameComponent* name = entity.get_component<kryos::NameComponent>();
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

void HierarchyEditorWorkspace::_create_shape(
    const std::string& new_entity_name, const std::string& mesh_file_path, kryos::Entity* parent
)
{
    kryos::Entity entity{};
    kryos::NameComponent* name = entity.add_component<kryos::NameComponent>(new_entity_name);

    kryos::MeshRendererComponent* mesh_renderer = entity.add_component<kryos::MeshRendererComponent>();
    mesh_renderer->model = kryos::Application::get_layer<kryos::AssetHandler>()->load_model_into_memory(
        mesh_file_path, kryos::ModelFileType_Obj
    );

    if (parent != nullptr)
    {
        // TODO: Setup parent component
    }
}

void HierarchyEditorWorkspace::_popup_menu(kryos::Entity* entity)
{
    kryos::Scene* scene = kryos::Application::get_layer<kryos::SceneManager>()->get_active_scene();

    if (scene != nullptr)
    {
        if (ImGui::BeginMenu("New"))
        {
            if (ImGui::MenuItem("Entity"))
                kryos::Entity creating{};

            if (ImGui::BeginMenu("Shape"))
            {
                if (ImGui::MenuItem("Cube"))
                    _create_shape("Cube", "kryos-lib/assets/models/cube", entity);
                if (ImGui::MenuItem("Sphere"))
                    _create_shape("Sphere", "kryos-lib/assets/models/sphere", entity);
                if (ImGui::MenuItem("Plane"))
                    _create_shape("Plane", "kryos-lib/assets/models/plane", entity);
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
