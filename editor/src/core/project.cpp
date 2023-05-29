#include "core/project.hpp"

#include <yaml/yaml.hpp>

namespace oge {

Project* Project::m_Instance = nullptr;

Project::Project() {
    assert(
        m_Instance == nullptr && "Project::Project() -> cannot created multiple project "
                                 "application layers"
    );

    m_Instance = this;
}

bool Project::create(const std::string& name, const std::string& project_root_path) {
    m_name = name;
    m_root_path = project_root_path;
    if (project_root_path[project_root_path.size() - 1] == '/') {
        m_project_filename = project_root_path + m_name + ".oproject";
    } else {
        m_project_filename = project_root_path + "/" + m_name + ".oproject";
    }

    yaml::Node root = {};
    root << yaml::node("ProjectName", m_name) << yaml::node("SceneCount", 0);
    if (!root.write_file(m_project_filename)) {
        m_name.clear();
        m_root_path.clear();
        m_project_filename.clear();

        return false;
    }

    return true;
}

bool Project::load(const std::string& project_filename) {
    yaml::Node root = yaml::open(project_filename);
    if (!root.empty()) {
        m_name = yaml::Convert<std::string>().value(root["ProjectName"]);
        m_project_filename = project_filename;
        m_root_path = std::string(project_filename, project_filename.find_last_of('/') - 1);

        std::vector<std::string> scene_names =
            yaml::Convert<std::vector<std::string>>().value(root["SceneNames"]);

        return true;
    }

    return false;
}

void Project::serialize(const std::string& path) {}

void Project::_deserialize_scene(ogl::Scene* scene) {}
void Project::_serialize_scene(ogl::Scene* scene) {}

} // namespace oge
