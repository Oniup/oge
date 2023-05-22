#include "gui/editor.hpp"

#include <imgui/imgui.h>

namespace oge {

static float VectorSliderMoveSpeed = 0.5f;

#define PROPERTY_EXISTS(NAME, TYPE)                    \
    bool property_exists_##NAME(ogl::Entity& entity) { \
        if (entity.get_component<TYPE>() != nullptr) { \
            return true;                               \
        }                                              \
        return false;                                  \
    }

PROPERTY_EXISTS(transform, ogl::TransformComponent)
PROPERTY_EXISTS(camera, ogl::CameraComponent)
PROPERTY_EXISTS(light, ogl::LightComponent)
PROPERTY_EXISTS(sprite_renderer, ogl::SpriteRendererComponent)
PROPERTY_EXISTS(sprite_animator, ogl::SpriteAtlasAnimatorComponent)
PROPERTY_EXISTS(mesh_renderer, ogl::MeshRendererComponent)

void property_draw_imgui_transform(ogl::Entity& entity) {
    ogl::TransformComponent* transform = entity.get_component<ogl::TransformComponent>();

    ImGui::DragFloat3("Position", &transform->position[0], VectorSliderMoveSpeed);
    ImGui::DragFloat3("Scale", &transform->scale[0], VectorSliderMoveSpeed);
    ImGui::DragFloat4("Rotation", &transform->rotation[0], VectorSliderMoveSpeed);

    if (ImGui::TreeNode("Options")) {
        ImGui::InputFloat("Slider Speed", &VectorSliderMoveSpeed);
        ImGui::Spacing();
        ImGui::TextWrapped("Node: soon the rotation will be changed to use quaternions");
        ImGui::TreePop();
    }
}

void property_draw_imgui_camera(ogl::Entity& entity) {
    ogl::CameraComponent* camera = entity.get_component<ogl::CameraComponent>();

    ImGui::Checkbox("Main", &camera->is_main);

    ImGui::Spacing();

    ImGui::DragFloat3("Position", &camera->position[0], VectorSliderMoveSpeed);
    ImGui::DragFloat3("Up Vector", &camera->up[0], VectorSliderMoveSpeed);
    ImGui::DragFloat3("Forward Vector", &camera->forward[0], VectorSliderMoveSpeed);

    ImGui::Spacing();

    ImGui::ColorEdit3("Clear Color", &camera->clear_color[0]);

    ImGui::Spacing();

    std::string project_type_selectable = "Projection Type: ";
    switch (camera->projection_type) {
    case ogl::CameraProjection_None:
        project_type_selectable += "ogl::CameraProjection_None";
        break;
    case ogl::CameraProjection_Perspective:
        project_type_selectable += "ogl::CameraProjection_Perspective";
        break;
    case ogl::CameraProjection_Orthographic:
        project_type_selectable += "ogl::CameraProjection_Orthographic";
        break;
    }

    if (ImGui::Button(project_type_selectable.c_str())) {
        ImGui::OpenPopup("Select camera projection");
    }

    if (ImGui::BeginPopup("Select camera projection")) {
        if (ImGui::Selectable("Perspective")) {
            camera->projection_type = ogl::CameraProjection_Perspective;
        }
        if (ImGui::Selectable("Orthographic")) {
            camera->projection_type = ogl::CameraProjection_Orthographic;
        }
        ImGui::EndPopup();
    }
}

void property_draw_imgui_light(ogl::Entity& entity) {
    ogl::LightComponent* light = entity.get_component<ogl::LightComponent>();

    char type_name[32]{};
    switch (light->type) {
    case ogl::LightType_Point:
        strcpy(type_name, "ogl::LightType_Point\0");
        break;
    case ogl::LightType_Directional:
        strcpy(type_name, "ogl::LightType_Directional\0");
        break;
    case ogl::LightType_Spot:
        strcpy(type_name, "ogl::LightType_Spot\0");
        break;
    }

    if (ImGui::Button(type_name)) {
        ImGui::OpenPopup("Select Light Type");
    }
    if (ImGui::BeginPopup("Select Light Type")) {
        if (ImGui::Selectable("ogl::LightType_Point")) {
            light->type = ogl::LightType_Point;
        }
        if (ImGui::Selectable("ogl::LightType_Directional")) {
            light->type = ogl::LightType_Directional;
        }
        if (ImGui::Selectable("ogl::LightType_Spot")) {
            light->type = ogl::LightType_Spot;
        }
        ImGui::EndPopup();
    }

    ImGui::Spacing();
    if (light->type != ogl::LightType_Directional) {
        ImGui::DragFloat3("Position", &light->position[0], VectorSliderMoveSpeed);
    }
    if (light->type != ogl::LightType_Point) {
        ImGui::DragFloat3("Direction", &light->direction[0], VectorSliderMoveSpeed);
    }

    ImGui::Spacing();
    ImGui::ColorEdit3("Color", &light->color[0]);
    ImGui::ColorEdit3("Ambient Color", &light->ambient_color[0]);
    ImGui::DragFloat("Intensity", &light->intensity, VectorSliderMoveSpeed);
}

void property_draw_imgui_mesh_renderer(ogl::Entity& entity) {
    ogl::MeshRendererComponent* mesh_renderer = entity.get_component<ogl::MeshRendererComponent>();

    ImGui::Checkbox("Is Static", &mesh_renderer->is_static);
    ImGui::Checkbox("Uses Lights", &mesh_renderer->uses_lights);
    ImGui::Checkbox("Cast Shadows", &mesh_renderer->cast_shadows);

    if (mesh_renderer->model != nullptr) {
        ogl::Model* model = mesh_renderer->model;
        ImGui::Spacing();
        ImGui::TextWrapped("Name: %s, Full Path: %s", model->name.c_str(), model->path.c_str());
    } else {
        ImGui::Spacing();
        ImGui::Text("No Mesh Selected ...");
    }
}

PropertyImGuiDraw::PropertyImGuiDraw(
    const std::string& name, fnptr_property_comp_exists exists, fnptr_property_imgui_draw draw
)
    : name(name), exists(exists), draw(draw) {}

PropertyImGuiDraw::PropertyImGuiDraw(const PropertyImGuiDraw& other)
    : name(other.name), exists(other.exists), draw(other.draw) {}

PropertyImGuiDraw::PropertyImGuiDraw(PropertyImGuiDraw&& other)
    : name(std::move(other.name)), exists(other.exists), draw(other.draw) {
    other.exists = nullptr;
    other.draw = nullptr;
}

PropertyImGuiDraw& PropertyImGuiDraw::operator=(const PropertyImGuiDraw& other) {
    name = other.name;
    exists = other.exists;
    draw = other.draw;

    return *this;
}

PropertyImGuiDraw& PropertyImGuiDraw::operator=(PropertyImGuiDraw&& other) {
    name = std::move(other.name);
    exists = other.exists;
    draw = other.draw;

    other.exists = nullptr;
    other.draw = nullptr;

    return *this;
}

PropertiesEditorWorkspace::PropertiesEditorWorkspace(HierarchyEditorWorkspace* hierarchy)
    : PanelEditorWorkspaceBase("Properties"), m_hierarchy(hierarchy) {
    // Engine components
    push_properties({
        PropertyImGuiDraw("Transform", property_exists_transform, property_draw_imgui_transform),
        PropertyImGuiDraw("Camera", property_exists_camera, property_draw_imgui_camera),
        PropertyImGuiDraw("Light", property_exists_light, property_draw_imgui_light),
        PropertyImGuiDraw(
            "Mesh Renderer", property_exists_mesh_renderer, property_draw_imgui_mesh_renderer
        ),
    });

    // User defined components
    // TODO: access project dll and load imgui properties defined
}

void PropertiesEditorWorkspace::push_properties(const std::initializer_list<PropertyImGuiDraw> list
) {
    size_t i = m_properties.size();
    m_properties.resize(m_properties.size() + list.size());

    size_t invalid_count = 0;
    for (const PropertyImGuiDraw& property : list) {
        if (property.draw != nullptr && property.exists != nullptr && property.name.size() > 0) {
            m_properties[i] = property;
            i++;
        } else {
            invalid_count++;
        }
    }

    // Resize the array to the correct size
    if (invalid_count > 0) {
        m_properties.resize(m_properties.size() - invalid_count);
    }
}

void PropertiesEditorWorkspace::on_imgui_update() {
    ImGui::Begin(get_name().c_str(), &get_enabled());

    if (m_hierarchy->get_selected_entity() != ECS_ENTITY_DESTROYED) {
        ogl::Entity entity = ogl::Entity(m_hierarchy->get_selected_entity());

        ogl::NameComponent* name = entity.get_component<ogl::NameComponent>();
        ogl::TagComponent* tag = entity.get_component<ogl::TagComponent>();

        if (ImGui::Button("Add Component", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
            // TODO: add popup window with the list of components accessable ...
        }

        ImGui::Separator();

        if (name != nullptr) {
            ImGui::InputText("Name", name->get(), OGL_NAME_COMPONENT_MAX_SIZE);
        } else {
            if (ImGui::Button("Add Name Component")) {
                entity.add_component<ogl::NameComponent>("Entity");
            }
        }

        if (tag != nullptr) {
            ImGui::InputText("Tag", tag->get(), OGL_NAME_COMPONENT_MAX_SIZE);
        } else {
            if (ImGui::Button("Add Tag Component")) {
                entity.add_component<ogl::TagComponent>("");
            }
        }

        for (PropertyImGuiDraw& property : m_properties) {
            if (property.exists(entity)) {
                if (ImGui::CollapsingHeader(
                        property.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen
                    )) {
                    property.draw(entity);
                    ImGui::Separator();
                }
            }
        }
    }

    ImGui::End();
}

} // namespace oge
