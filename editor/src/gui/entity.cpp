#include "ogl/core/entity.hpp"

namespace ogl {

Entity::Entity(entt::entity id) : m_id(id) {}

Entity::Entity(bool no_transform_component) {
    Scene* scene = SceneManager::get()->get_active_scene();
    if (scene == nullptr) {
        Debug::log("Entity::Entity() -> cannot create entity as there is no "
                   "active scene");
        return;
    }

    m_id = scene->get_registry().create();
    if (!no_transform_component) {
        scene->get_registry().emplace<TransformComponent>(m_id);
    }
}

void Entity::destroy() {
    Scene* scene = SceneManager::get()->get_active_scene();
    scene->push_deleted_entity(*this);

    // TODO: Implement entity parenting
}

} // namespace ogl
