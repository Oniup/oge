#include "core/app.hpp"
#include "core/editor.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <ogl/renderer/renderer.hpp>
#include <ogl/utils/yaml_serialization.hpp>

namespace oge {

App::App() {
    ogl::Pipeline::get()->get_window()->set_title("Oniup's Game Editor - empty*");
    ogl::Pipeline::get()->get_window()->set_size(ogl::WindowResolution_Maximize);
    static_cast<ogl::BasicRenderer*>(ogl::Pipeline::get()->push_renderer(new ogl::BasicRenderer()))
        ->use_default_framebuffer(false);
    ogl::Framebuffer* editor_viewport_framebuffer =
        ogl::Pipeline::get()->create_framebuffer("editor viewport", 1280, 720);

    ogl::Debug* debug = get_layer<ogl::Debug>(OGL_CORE_DEBUG_LAYER_NAME);
    debug->set_automatically_clear_on_update(false);
    debug->set_serialize(true);

    // initialize editor workspace
    EditorWorkspace* workspace = push_layer<EditorWorkspace>("editor workspace");
    workspace->push_panels({
        new DockingEditorWorkspace(workspace),
        new ConsoleEditorWorkspace(debug),
        new ViewportEditorWorkspace(editor_viewport_framebuffer),
        new HierarchyEditorWorkspace(),
        new AssetsEditorWorkspace(),
    });
    workspace->push_panel<InspectorEditorWorkspace>(
        static_cast<HierarchyEditorWorkspace*>(workspace->get_panel("Hierarchy"))
    );

    // example scene
    // TODO: implement serialization
    ogl::Scene* scene = ogl::SceneManager::get()->push("empty");
    ogl::SceneManager::get()->set_active(scene);

    ogl::Entity camera{true};
    camera.add_component<ogl::NameComponent>("Main Camera");
    ogl::CameraComponent* camera_comp = camera.add_component<ogl::CameraComponent>();
    camera.add_component<ogl::TagComponent>("oge_editor");
    camera_comp->clear_color = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    camera_comp->is_main = true;
    camera_comp->projection_type = ogl::CameraProjection_Perspective;

    ogl::Entity ground{};
    ground.add_component<ogl::NameComponent>("Ground");
    ogl::TransformComponent* ground_transform = ground.get_component<ogl::TransformComponent>();
    ogl::MeshRendererComponent* ground_mesh_renderer =
        ground.add_component<ogl::MeshRendererComponent>();
    ground_mesh_renderer->model = ogl::AssetHandler::get()->load_model_into_memory(
        "ogl/assets/models/plane", ogl::ModelFileType_Obj
    );
    ground_transform->position = glm::vec3(0.0f, -1.0f, -7.0f);
    ground_transform->scale = glm::vec3(2.0f, 1.0f, 2.0f);

    ogl::Entity sphere{};
    sphere.add_component<ogl::NameComponent>("Sphere");
    sphere.get_component<ogl::TransformComponent>()->position = glm::vec3(0.0f, 0.0f, -7.0f);
    sphere.get_component<ogl::TransformComponent>()->scale = glm::vec3(1.0f, 1.0f, 1.0f);
    ogl::MeshRendererComponent* sphere_mesh_renderer =
        sphere.add_component<ogl::MeshRendererComponent>();
    sphere_mesh_renderer->model = ogl::AssetHandler::get()->load_model_into_memory(
        "ogl/assets/models/sphere", ogl::ModelFileType_Obj
    );
    sphere_mesh_renderer->model->meshes[0].material->overlay_color =
        glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);

    ogl::Entity light{};
    light.add_component<ogl::NameComponent>("Point Light");
    ogl::LightComponent* light_comp = light.add_component<ogl::LightComponent>();
    ogl::MeshRendererComponent* light_mesh_renderer =
        light.add_component<ogl::MeshRendererComponent>();
    ogl::TransformComponent* light_transform = light.get_component<ogl::TransformComponent>();
    light_mesh_renderer->model = ogl::AssetHandler::get()->load_model_into_memory(
        "ogl/assets/models/cube", ogl::ModelFileType_Obj
    );
    light_mesh_renderer->model->meshes[0].material->overlay_color =
        glm::vec4(0.7f, 1.0f, 1.0f, 1.0f);
    light_transform->position = glm::vec3(1.0f, 1.0f, -1.0f);
    light_comp->color = glm::vec3(0.7f, 1.0f, 1.0f);
    light_comp->position = light_transform->position;

    ogl::YamlSerialization conf = ogl::YamlSerialization("test_data.yaml");
    conf.debug_log();

    conf.clear(); // override file otherwise it'll append the following new settings
    for (size_t i = 0; i < 3; i++) {
        ogl::YamlSerializationOption* comp =
            conf.push_option(ogl::YamlSerializationOption("Ent:1"));
        comp->scope.push_back(ogl::YamlSerializationOption("ogl::TransformComponent"));
        comp->scope[0].scope = {
            {"position", "glm::vec3(1, 2, 3)"},
            {"scale", "glm::vec3(1, 1, 1)"},
            {"rotation", "glm::vec4(0, 0, 1, 0)"},
        };

        comp->scope.push_back(ogl::YamlSerializationOption("ogl::PlayerController"));
        comp->scope[1].scope = {
            {"move_speed", "5.0"},
            {"damage", "5.0"},
        };
    }

    conf.write_changes();
}

} // namespace oge
