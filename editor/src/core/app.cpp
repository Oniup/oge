#include "core/app.hpp"
#include "core/editor.hpp"

#include <ogl/renderer/renderer.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace oge {

	App::App() {
		ogl::Pipeline::get()->get_window()->set_title("Oniup's Game Editor - empty*");
		ogl::Pipeline::get()->get_window()->set_size(ogl::WindowResolution_Maximize);

		ogl::Debug* debug = get_layer<ogl::Debug>(OGL_CORE_DEBUG_LAYER_NAME);
		debug->set_serialize(true);
		ogl::Debug::log("This is a Plain Message");
		ogl::Debug::log("And this is a Warning Message", ogl::DebugType_Warning);
		ogl::Debug::log("Finally, this is a Error Message", ogl::DebugType_Error);

		EditorWorkspace* workspace = push_layer<EditorWorkspace>("editor workspace");
		workspace->push_panels({
			new Docking(workspace), new Console()
		});

		ogl::Pipeline::get()->push_renderer(new ogl::BasicRenderer());

		ogl::Scene* scene = ogl::SceneManager::get()->push("empty");
		ogl::SceneManager::get()->set_active(scene);

		ogl::Window* window = ogl::Pipeline::get()->get_window();

		ogl::Entity camera{ true };
		ogl::CameraComponent* camera_comp = camera.add_component<ogl::CameraComponent>();
		camera.add_component<ogl::TagComponent>("oge_editor");
		camera_comp->clear_color = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
		camera_comp->is_main = true;
		camera_comp->projection_type = ogl::CameraProjection_Perspective;
		camera_comp->projection_matrix = glm::perspective(
			glm::radians(45.0f), static_cast<float>(window->get_width()) / static_cast<float>(window->get_height()), 0.1f, 100.0f
		);

		// ogl::Entity ground{};
		// ogl::TransformComponent* ground_transform = ground.get_component<ogl::TransformComponent>();
		// ogl::MeshRendererComponent* ground_mesh_renderer = ground.add_component<ogl::MeshRendererComponent>();
		// ground_mesh_renderer->model = ogl::AssetHandler::get()->load_model_into_memory("ogl/assets/models/plane", ogl::ModelFileType_Obj);
		// ground_transform->position = glm::vec3(0.0f, -1.0f, -2.0f);
		// ground_transform->scale = glm::vec3(5.0f, 1.0f, 5.0f);

		// ogl::Entity light{};
		// ogl::LightComponent* light_comp = light.add_component<ogl::LightComponent>();
		// ogl::MeshRendererComponent* light_mesh_renderer = light.add_component<ogl::MeshRendererComponent>();
		// ogl::TransformComponent* light_transform = light.get_component<ogl::TransformComponent>();
		// light_mesh_renderer->model = ogl::AssetHandler::get()->load_model_into_memory("ogl/assets/models/cube", ogl::ModelFileType_Obj);
		// light_mesh_renderer->model->meshes[0].material->overlay_color = glm::vec4(0.7f, 1.0f, 1.0f, 1.0f);
		// light_transform->position = glm::vec3(1.0f, 1.0f, -1.0f);
		// light_comp->color = glm::vec3(0.7f, 1.0f, 1.0f);
		// light_comp->position = light_transform->position;
	}

}