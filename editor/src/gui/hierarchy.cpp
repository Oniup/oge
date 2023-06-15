#include "gui/editor.hpp"

#include <imgui/imgui.h>

namespace oge {

HierarchyEditorWorkspace::HierarchyEditorWorkspace() : PanelEditorWorkspaceBase("Hierarchy") {}

void HierarchyEditorWorkspace::on_imgui_update() {
    ImGui::Begin(get_name().c_str(), &get_enabled());

    ogl::Scene* active_scene = ogl::Application::get_layer<ogl::SceneManager>()->get_active_scene();
    if (active_scene != nullptr) {
        ecs::Registry& registry = active_scene->get_registry();
        const std::vector<ecs::Entity>& entities = registry.get_entities();

        ecs::Entity entity_clicked = ECS_ENTITY_DESTROYED;
        bool opened_targeted_entity_popup = false;
        for (std::size_t i = 0; i < entities.size(); i++) {
            ogl::Entity entity = ogl::Entity(entities[i]);
            if (entity) {
                ogl::TagComponent* tag = entity.get_component<ogl::TagComponent>();
                bool include_ent = true;
                if (tag != nullptr) {
                    if (tag->tag.size() > 0) {
                        // PERFORMANCE: try to avoid string comparison
                        if (std::string(HIERARCHY_FILTER_NAME) == tag->tag) {
                            include_ent = false;
                        }
                    }
                }

                if (include_ent) {
                    _draw_entity(entity, entity_clicked, opened_targeted_entity_popup);
                }
            }
        }

        if (entity_clicked != ECS_ENTITY_DESTROYED) {
            m_selected_entity = entity_clicked;
        }

        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
            m_selected_entity = ECS_ENTITY_DESTROYED;
        }

        if (!opened_targeted_entity_popup) {
            if (ImGui::BeginPopupContextWindow()) {
                _popup_menu();
                ImGui::EndPopup();
            }
        }
    }

    ImGui::End();
}

void HierarchyEditorWorkspace::_draw_entity(
    ogl::Entity entity, ecs::Entity& entity_clicked, bool& opened_popup
) {
    int flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen |
                ImGuiTreeNodeFlags_SpanFullWidth;
    if (m_selected_entity == entity) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    ogl::NameComponent* name = entity.get_component<ogl::NameComponent>();
    std::string str = "Entity";
    if (name != nullptr) {
        str = name->name;
    }

    ecs::Entity id = entity;
    ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(id)), flags, "%s", str.c_str());

    if (ImGui::BeginPopupContextItem()) {
        opened_popup = true;
        _popup_menu(&entity);
        ImGui::EndPopup();
    }

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        entity_clicked = id;
    }
}

void HierarchyEditorWorkspace::_create_shape(
    const std::string& new_entity_name, const std::string& mesh_file_path, ogl::Entity* parent
) {
    ogl::Entity entity{};
    ogl::NameComponent* name = entity.add_component<ogl::NameComponent>(new_entity_name);

    ogl::MeshRendererComponent* mesh_renderer = entity.add_component<ogl::MeshRendererComponent>();
    mesh_renderer->model =
        ogl::Application::get_layer<ogl::AssetHandler>()->load_model_into_memory(mesh_file_path, ogl::ModelFileType_Obj);

    if (parent != nullptr) {
        // TODO: Setup parent component
    }
}

void HierarchyEditorWorkspace::_popup_menu(ogl::Entity* entity) {
    ogl::Scene* scene = ogl::Application::get_layer<ogl::SceneManager>()->get_active_scene();

    if (scene != nullptr) {
        if (ImGui::BeginMenu("New")) {
            if (ImGui::MenuItem("Entity")) {
                ogl::Entity creating{};
            }

            if (ImGui::BeginMenu("Shape")) {
                if (ImGui::MenuItem("Cube")) {
                    _create_shape("Cube", "ogl/assets/models/cube", entity);
                }
                if (ImGui::MenuItem("Sphere")) {
                    _create_shape("Sphere", "ogl/assets/models/sphere", entity);
                }
                if (ImGui::MenuItem("Plane")) {
                    _create_shape("Plane", "ogl/assets/models/plane", entity);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (entity != nullptr) {
            if (ImGui::MenuItem("Delete")) {
                entity->destroy();
            }
        }
    } else {
        if (ImGui::MenuItem("Create Scene")) {
            // TODO: ...
        }
    }
}

} // namespace oge
