#include "gui/properties.hpp"
#include "gui/editor.hpp"

#include <imgui/imgui.h>

namespace oge {

float PropertyDefinition::VectorSliderMoveSpeed = 0.5f;

class TransformPropertyDefinition : public PropertyTypeDefinition<ogl::TransformComponent> {
  public:
    TransformPropertyDefinition() : PropertyTypeDefinition() {}
    virtual void draw_imgui(ogl::Entity entity) override {
        ogl::TransformComponent* transform = entity.get_component<ogl::TransformComponent>();

        ImGui::DragFloat3("Position", &transform->translation[0], VectorSliderMoveSpeed);
        ImGui::DragFloat3("Scale", &transform->scale[0], VectorSliderMoveSpeed);
        ImGui::DragFloat4("Rotation", &transform->rotation[0], VectorSliderMoveSpeed);
    }
};

class CameraPropertyDefinition : public PropertyTypeDefinition<ogl::CameraComponent> {
  public:
    CameraPropertyDefinition() : PropertyTypeDefinition() {}
    virtual void draw_imgui(ogl::Entity entity) override {
        ogl::CameraComponent* camera = entity.get_component<ogl::CameraComponent>();

        ImGui::Checkbox("Main", &camera->is_main);

        ImGui::Spacing();

        ImGui::DragFloat3("Position", &camera->position[0], VectorSliderMoveSpeed);
        ImGui::DragFloat3("Up", &camera->up[0], VectorSliderMoveSpeed);
        ImGui::DragFloat3("Forward", &camera->forward[0], VectorSliderMoveSpeed);

        ImGui::Spacing();

        ImGui::ColorEdit3("Clear Color", &camera->clear_color[0]);

        ImGui::Spacing();

        std::string project_type_selectable = "Projection Type: ";
        switch (camera->projection_type) {
        case ogl::CameraProjection_None:
            project_type_selectable += "None";
            break;
        case ogl::CameraProjection_Perspective:
            project_type_selectable += "Perspective";
            break;
        case ogl::CameraProjection_Orthographic:
            project_type_selectable += "Orthographic";
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
};

class LightPropertyDefinition : public PropertyTypeDefinition<ogl::LightComponent> {
  public:
    LightPropertyDefinition() : PropertyTypeDefinition() {}

    virtual void draw_imgui(ogl::Entity entity) override {
        ogl::LightComponent* light = entity.get_component<ogl::LightComponent>();

        char type_name[32]{};
        switch (light->type) {
        case ogl::LightType_Point:
            strcpy(type_name, "Type: Point\0");
            break;
        case ogl::LightType_Directional:
            strcpy(type_name, "Type: Directional\0");
            break;
        case ogl::LightType_Spot:
            strcpy(type_name, "Type: Spot\0");
            break;
        }

        if (ImGui::Button(type_name)) {
            ImGui::OpenPopup("Select Light Type");
        }
        if (ImGui::BeginPopup("Select Light Type")) {
            if (ImGui::Selectable("Point")) {
                light->type = ogl::LightType_Point;
            }
            if (ImGui::Selectable("Direction")) {
                light->type = ogl::LightType_Directional;
            }
            if (ImGui::Selectable("Spot")) {
                light->type = ogl::LightType_Spot;
            }
            ImGui::EndPopup();
        }

        ImGui::Spacing();
        if (light->type != ogl::LightType_Point) {
            ImGui::DragFloat3("Direction", &light->direction[0], VectorSliderMoveSpeed);
        }

        ImGui::Spacing();
        ImGui::ColorEdit3("Color", &light->color[0]);
        ImGui::ColorEdit3("Ambient Color", &light->ambient_color[0]);
        ImGui::DragFloat("Intensity", &light->intensity, VectorSliderMoveSpeed);
    }
};

class MeshRendererPropertyDefinition : public PropertyTypeDefinition<ogl::MeshRendererComponent> {
  public:
    MeshRendererPropertyDefinition() : PropertyTypeDefinition() {}

    virtual void draw_imgui(ogl::Entity entity) override {
        ogl::MeshRendererComponent* mesh_renderer =
            entity.get_component<ogl::MeshRendererComponent>();
        constexpr std::size_t change_path_buffer_size = 1028;
        static char change_path_buffer[change_path_buffer_size] = {};

        bool is_static = mesh_renderer->is_static;
        ImGui::Checkbox("Is Static", &is_static);
        ImGui::Checkbox("Uses Lights", &mesh_renderer->uses_lights);
        ImGui::Checkbox("Cast Shadows", &mesh_renderer->cast_shadows);

        if (mesh_renderer->model != nullptr) {
            ogl::Model* model = mesh_renderer->model;
            ImGui::Spacing();
            ImGui::TextWrapped("Mesh: %s", mesh_renderer->model->name.c_str());

            if (ImGui::Button(mesh_renderer->model->path.c_str())) {
                ImGui::OpenPopup("Set Mesh Path");
                strncpy(
                    change_path_buffer, mesh_renderer->model->path.c_str(),
                    mesh_renderer->model->path.size()
                );
                change_path_buffer[mesh_renderer->model->path.size()] = '\0';
            }
        } else {
            ImGui::Spacing();
            if (ImGui::Button("Set Mesh")) {
                ImGui::OpenPopup("Set Mesh Path");
                change_path_buffer[0] = '\0';
            }
        }

        if (ImGui::BeginPopup("Set Mesh Path")) {
            ImGui::InputText("Path", change_path_buffer, change_path_buffer_size);
            if (ImGui::Button("Set")) {
                ogl::Model* model = ogl::AssetHandler::get()->load_model_into_memory(
                    change_path_buffer, ogl::ModelFileType_Obj
                );
                if (!mesh_renderer->is_static) {
                    delete mesh_renderer->model;
                }
                mesh_renderer->model = model;
            }
            ImGui::EndPopup();
        }

        if (is_static != mesh_renderer->is_static) {
            ogl::Debug::log("static meshes aren't implemented yet", ogl::DebugType_Warning);
        }
    }
};

PropertiesEditorWorkspace::PropertiesEditorWorkspace(HierarchyEditorWorkspace* hierarchy)
    : PanelEditorWorkspaceBase("Properties"), m_hierarchy(hierarchy) {
    // Engine components
    push_properties({
        new TransformPropertyDefinition(),
        new CameraPropertyDefinition(),
        new LightPropertyDefinition(),
        new MeshRendererPropertyDefinition(),
    });

    // User defined components
    // TODO: access project dll and load imgui properties defined
}

PropertiesEditorWorkspace::~PropertiesEditorWorkspace() {
    for (PropertyDefinition* property : m_properties) {
        delete property;
    }
}

void PropertiesEditorWorkspace::push_properties(
    const std::initializer_list<PropertyDefinition*> list
) {
    size_t i = m_properties.size();
    m_properties.resize(m_properties.size() + list.size());

    for (PropertyDefinition* property : list) {
        m_properties[i] = property;
        i++;
    }
}

void PropertiesEditorWorkspace::on_imgui_update() {
    ImGui::Begin(get_name().c_str(), &get_enabled(), ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Edit")) {
            ImGui::InputFloat("Drag Element Speed", &PropertyDefinition::VectorSliderMoveSpeed);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (m_hierarchy->get_selected_entity() != ECS_ENTITY_DESTROYED) {
        ogl::Entity entity = ogl::Entity(m_hierarchy->get_selected_entity());

        if (ImGui::Button("Add Component", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
            ImGui::OpenPopup("Add Component Popup");
        }

        float popup_width = ImGui::GetContentRegionAvail().x;
        if (ImGui::BeginPopup("Add Component Popup")) {
            ImGui::BeginChild("Component List", ImVec2(popup_width, 300.0f));

            for (PropertyDefinition* property : m_properties) {
                if (!property->exists(entity)) {
                    if (ImGui::Button(
                            property->get_name().c_str(),
                            ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)
                        )) {
                        property->add_component(m_hierarchy->get_selected_entity());
                    }
                }
            }

            ImGui::EndChild();
            ImGui::EndPopup();
        }

        ogl::NameComponent* name = entity.get_component<ogl::NameComponent>();
        ogl::TagComponent* tag = entity.get_component<ogl::TagComponent>();

        if (name != nullptr) {
            char str[OGL_NAME_COMPONENT_MAX_SIZE];
            strncpy(str, name->name.c_str(), name->name.size());
            str[name->name.size()] = '\0';
            ImGui::InputText("Name", str, OGL_NAME_COMPONENT_MAX_SIZE);
            name->name = str;
        } else {
            if (ImGui::Button("Add Name Component")) {
                entity.add_component<ogl::NameComponent>("Entity");
            }
        }

        if (tag != nullptr) {
            char str[OGL_NAME_COMPONENT_MAX_SIZE];
            strncpy(str, tag->tag.c_str(), tag->tag.size());
            str[tag->tag.size()] = '\0';
            ImGui::InputText("Tag", str, OGL_NAME_COMPONENT_MAX_SIZE);
            tag->tag = str;
        } else {
            if (ImGui::Button("Add Tag Component")) {
                entity.add_component<ogl::TagComponent>("");
            }
        }

        for (PropertyDefinition* property : m_properties) {
            if (property->exists(entity)) {
                if (ImGui::CollapsingHeader(
                        property->get_name().c_str(), ImGuiTreeNodeFlags_DefaultOpen
                    )) {
                    property->draw_imgui(entity);

                    ImGui::NewLine();
                }
            }
        }
    }
    ImGui::End();
}

} // namespace oge
