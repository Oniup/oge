#include "gui/properties.hpp"
#include "gui/editor.hpp"

#include <imgui/imgui.h>

namespace oge {

void int_draw(const std::string& fieldname, void* ptr) {
    int* value = reinterpret_cast<int*>(ptr);
    ImGui::InputInt(std::string("##" + fieldname).c_str(), value);
}

void float_draw(const std::string& fieldname, void* ptr) {
    float* value = reinterpret_cast<float*>(ptr);
    ImGui::InputFloat(std::string("##" + fieldname).c_str(), value);
}

void bool_draw(const std::string& fieldname, void* ptr) {
    bool* value = reinterpret_cast<bool*>(ptr);
    ImGui::Checkbox(std::string("##" + fieldname).c_str(), value);
}

void str_draw(const std::string& fieldname, void* ptr) {
    constexpr std::size_t max_line_length = 10000;

    std::string* value = reinterpret_cast<std::string*>(ptr);
    static char str[max_line_length] = {};

    strncpy(str, value->c_str(), value->size());
    str[value->size()] = '\0';
    ImGui::InputText(std::string("##" + fieldname).c_str(), str, max_line_length);
    *value = str;
}

void vec2_draw(const std::string& fieldname, void* ptr) {
    glm::vec2& vec = *reinterpret_cast<glm::vec2*>(ptr);
    ImGui::InputFloat2(std::string("##" + fieldname).c_str(), &vec[0]);
}

void vec3_draw(const std::string& fieldname, void* ptr) {
    glm::vec3& vec = *reinterpret_cast<glm::vec3*>(ptr);
    ImGui::InputFloat3(std::string("##" + fieldname).c_str(), &vec[0]);
}

void vec4_draw(const std::string& fieldname, void* ptr) {
    glm::vec4& vec = *reinterpret_cast<glm::vec4*>(ptr);
    ImGui::InputFloat4(std::string("##" + fieldname).c_str(), &vec[0]);
}

void ivec2_draw(const std::string& fieldname, void* ptr) {
    glm::ivec2& vec = *reinterpret_cast<glm::ivec2*>(ptr);
    ImGui::InputInt2(std::string("##" + fieldname).c_str(), &vec[0]);
}

void ivec3_draw(const std::string& fieldname, void* ptr) {
    glm::ivec3& vec = *reinterpret_cast<glm::ivec3*>(ptr);
    ImGui::InputInt3(std::string("##" + fieldname).c_str(), &vec[0]);
}

void ivec4_draw(const std::string& fieldname, void* ptr) {
    glm::ivec4& vec = *reinterpret_cast<glm::ivec4*>(ptr);
    ImGui::InputInt4(std::string("##" + fieldname).c_str(), &vec[0]);
}

PropertiesEditorWorkspace::PropertiesEditorWorkspace(HierarchyEditorWorkspace* hierarchy)
    : PanelEditorWorkspaceBase("Properties"), m_hierarchy(hierarchy) {
    _initialize_draw_fnptrs({
        {ogl::TypeId::create<std::int32_t>().get_id(), int_draw},
        {ogl::TypeId::create<std::int64_t>().get_id(), int_draw},
        {ogl::TypeId::create<std::uint32_t>().get_id(), int_draw},
        {ogl::TypeId::create<std::uint64_t>().get_id(), int_draw},

        {ogl::TypeId::create<float>().get_id(), float_draw},
        {ogl::TypeId::create<bool>().get_id(), bool_draw},

        {ogl::TypeId::create<std::string>().get_id(), str_draw},

        {ogl::TypeId::create<glm::vec2>().get_id(), vec2_draw},
        {ogl::TypeId::create<glm::vec3>().get_id(), vec3_draw},
        {ogl::TypeId::create<glm::vec4>().get_id(), vec4_draw},

        {ogl::TypeId::create<glm::ivec2>().get_id(), ivec2_draw},
        {ogl::TypeId::create<glm::ivec3>().get_id(), ivec3_draw},
        {ogl::TypeId::create<glm::ivec4>().get_id(), ivec4_draw},
    });
}

void PropertiesEditorWorkspace::on_imgui_update() {
    ImGui::Begin(get_name().c_str(), &get_enabled(), ImGuiWindowFlags_MenuBar);

    if (m_hierarchy->get_selected_entity() != ECS_ENTITY_DESTROYED) {
        float width = ImGui::GetContentRegionAvail().x;
        ogl::Entity entity = m_hierarchy->get_selected_entity();
        ogl::Scene* scene = ogl::Application::get_layer<ogl::SceneManager>()->get_active_scene();
        ogl::ReflectionRegistry* reflection =
            ogl::Application::get_layer<ogl::ReflectionRegistry>();

        for (ecs::ObjectPool* pool : scene->get_registry().get_pools()) {
            ecs::byte* object = reinterpret_cast<ecs::byte*>(pool->get_entitys_object(entity));

            if (reflection->get_all_type_infos().contains(pool->get_type_hash())) {
                if (object != nullptr) {
                    const std::set<ogl::MemberInfo>& members =
                        reflection->get_members(ogl::TypeId(pool->get_type_hash()));

                    _imgui_draw(object, members.begin(), members.end(), reflection, width);
                }
            }
        }
    }

    ImGui::End();
}

void PropertiesEditorWorkspace::_initialize_draw_fnptrs(
    std::initializer_list<std::pair<std::uint64_t, fnptr_imgui_draw_comp>> list
) {
    for (const auto& [hash, fnptr] : list) {
        m_draw_fnptrs.emplace(hash, fnptr);
    }
}

void PropertiesEditorWorkspace::_imgui_draw(
    ecs::byte* object, std::set<ogl::MemberInfo>::iterator it,
    const std::set<ogl::MemberInfo>::iterator end, ogl::ReflectionRegistry* reflection,
    const float width
) {
    if (it != end) {
        if (m_draw_fnptrs.contains(it->type.get_id())) {
            ImGui::Text(
                "%s (%s)", it->fieldname.c_str(),
                reflection->get_variable_type_name(it->variable).c_str()
            );

            it++;
            _imgui_draw(object, it, end, reflection, width);
        } else {
            // TODO: Check for pointers types as it fucks with it
            if (reflection->type_contains_members(it->type)) {
                const std::set<ogl::MemberInfo>& members = reflection->get_members(it->type);
                ecs::byte* new_object = object + it->offset;
                ImGui::BeginChild(
                    std::string("Right_" + reflection->get_type_info(it->type).name).c_str(),
                    ImVec2(), true
                );
                { _imgui_draw(new_object, members.begin(), members.end(), reflection, width); }
                ImGui::EndChild();
            }
        }
    }
}

} // namespace oge
