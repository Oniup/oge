#include "gui/properties.hpp"
#include "gui/editor.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace oge {

void int_draw(const std::string& fieldname, void* ptr, float step_size)
{
    int* value = reinterpret_cast<int*>(ptr);
    ImGui::DragInt(std::string("##" + fieldname).c_str(), value, step_size);
}

void float_draw(const std::string& fieldname, void* ptr, float step_size)
{
    float* value = reinterpret_cast<float*>(ptr);
    ImGui::DragFloat(std::string("##" + fieldname).c_str(), value, step_size);
}

void bool_draw(const std::string& fieldname, void* ptr, float step_size)
{
    bool* value = reinterpret_cast<bool*>(ptr);
    ImGui::Checkbox(std::string("##" + fieldname).c_str(), value);
}

void str_draw(const std::string& fieldname, void* ptr, float step_size)
{
    constexpr std::size_t max_line_length = 10000;

    std::string* value = reinterpret_cast<std::string*>(ptr);
    static char str[max_line_length] = {};

    strncpy(str, value->c_str(), value->size());
    str[value->size()] = '\0';
    ImGui::InputText(std::string("##" + fieldname).c_str(), str, max_line_length);
    *value = str;
}

void vec2_draw(const std::string& fieldname, void* ptr, float step_size)
{
    glm::vec2& vec = *reinterpret_cast<glm::vec2*>(ptr);
    ImGui::DragFloat2(std::string("##" + fieldname).c_str(), &vec[0], step_size);
}

void vec3_draw(const std::string& fieldname, void* ptr, float step_size)
{
    float* vec = reinterpret_cast<float*>(ptr);
    ImGui::DragFloat3(std::string("##" + fieldname).c_str(), vec, step_size);
}

void vec4_draw(const std::string& fieldname, void* ptr, float step_size)
{
    float* vec = reinterpret_cast<float*>(ptr);
    ImGui::DragFloat4(std::string("##" + fieldname).c_str(), vec, step_size);
}

void ivec2_draw(const std::string& fieldname, void* ptr, float step_size)
{
    int* vec = reinterpret_cast<int*>(ptr);
    ImGui::DragInt2(std::string("##" + fieldname).c_str(), vec, step_size);
}

void ivec3_draw(const std::string& fieldname, void* ptr, float step_size)
{
    int* vec = reinterpret_cast<int*>(ptr);
    ImGui::DragInt3(std::string("##" + fieldname).c_str(), vec, step_size);
}

void ivec4_draw(const std::string& fieldname, void* ptr, float step_size)
{
    int* vec = reinterpret_cast<int*>(ptr);
    ImGui::DragInt4(std::string("##" + fieldname).c_str(), vec, step_size);
}

PropertiesEditorWorkspace::PropertiesEditorWorkspace(HierarchyEditorWorkspace* hierarchy)
    : PanelEditorWorkspaceBase("Properties"), m_hierarchy(hierarchy)
{
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

void PropertiesEditorWorkspace::on_imgui_update()
{
    ImGui::Begin(get_name().c_str(), &get_enabled(), ImGuiWindowFlags_MenuBar);
    static bool add_component_popup = false;

    if (ImGui::BeginMenuBar())
    {
        if (m_hierarchy->get_selected_entity() != ECS_ENTITY_DESTROYED)
        {
            if (ImGui::MenuItem("Add Component"))
                add_component_popup = true;

            ogl::Entity entity = m_hierarchy->get_selected_entity();
            if (entity.get_component<ogl::NameComponent>() == nullptr)
            {
                if (ImGui::MenuItem("Add Name"))
                    entity.add_component<ogl::NameComponent>();
            }

            if (entity.get_component<ogl::TagComponent>() == nullptr)
            {
                if (ImGui::MenuItem("Add Tag"))
                    entity.add_component<ogl::TagComponent>();
            }
        }

        ImGui::EndMenuBar();
    }

    if (m_hierarchy->get_selected_entity() != ECS_ENTITY_DESTROYED)
    {
        ogl::Entity entity = m_hierarchy->get_selected_entity();
        ogl::Scene* scene = ogl::Application::get_layer<ogl::SceneManager>()->get_active_scene();
        ogl::ReflectionRegistry* reflection =
            ogl::Application::get_layer<ogl::ReflectionRegistry>();

        ogl::NameComponent* name_comp = entity.get_component<ogl::NameComponent>();
        ogl::TagComponent* tag_comp = entity.get_component<ogl::TagComponent>();

        static char str[OGL_NAME_COMPONENT_MAX_SIZE] = {};
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        if (name_comp != nullptr)
        {
            strncpy(str, name_comp->name.c_str(), name_comp->name.size());
            str[name_comp->name.size()] = '\0';

            ImGui::InputText("##NameComponent", str, OGL_NAME_COMPONENT_MAX_SIZE);
            name_comp->name = str;
        }

        if (tag_comp != nullptr)
        {
            strncpy(str, tag_comp->tag.c_str(), tag_comp->tag.size());
            str[tag_comp->tag.size()] = '\0';

            ImGui::InputText("##NameComponent", str, OGL_NAME_COMPONENT_MAX_SIZE);
            tag_comp->tag = str;
        }
        ImGui::PopItemWidth();

        ImGui::NewLine();
        ImGui::Separator();

        for (ecs::ObjectPool* pool : scene->get_registry().get_pools())
        {
            if (pool->get_type_hash() == ogl::TypeId::create<ogl::NameComponent>().get_id() ||
                pool->get_type_hash() == ogl::TypeId::create<ogl::TagComponent>().get_id() ||
                pool->get_type_hash() == ogl::TypeId::create<ogl::ParentComponent>().get_id())
            {
                continue;
            }

            ecs::byte* object = reinterpret_cast<ecs::byte*>(pool->get_entitys_object(entity));
            if (object != nullptr)
            {
                if (reflection->get_all_type_infos().contains(pool->get_type_hash()))
                {
                    if (ImGui::CollapsingHeader(
                            pool->get_name().c_str(), ImGuiTreeNodeFlags_DefaultOpen
                        ))
                    {
                        ImGui::BeginTable("Component Table", 2, ImGuiTableFlags_BordersInnerH);
                        {
                            ImGui::TableSetupColumn("Names", ImGuiTableColumnFlags_WidthFixed, ImGui::GetContentRegionAvail().x * 0.25f);
                            ImGui::TableSetupColumn("settings", ImGuiTableColumnFlags_WidthFixed, ImGui::GetContentRegionAvail().x * 0.75f);
                            const std::set<ogl::MemberInfo>& members =
                                reflection->get_members(ogl::TypeId(pool->get_type_hash()));

                            _imgui_draw(object, members.begin(), members.end(), reflection);
                        }
                        ImGui::EndTable();
                    }
                }
            }
        }

        if (add_component_popup)
        {
            ImGui::OpenPopup("Add Component");
            add_component_popup = false;
        }

        float popup_width = ImGui::GetContentRegionAvail().x;
        if (ImGui::BeginPopup("Add Component"))
        {
            ogl::ReflectionRegistry* reflection =
                ogl::Application::get_layer<ogl::ReflectionRegistry>();
            ImGui::BeginChild("List", ImVec2(popup_width, 300));
            {
                for (const auto& [type, info] : reflection->get_all_type_infos())
                {
                    if (type == ogl::TypeId::create<ogl::NameComponent>().get_id() ||
                        type == ogl::TypeId::create<ogl::TagComponent>().get_id() ||
                        type == ogl::TypeId::create<ogl::ParentComponent>().get_id())
                    {
                        continue;
                    }
                    else if (entity.get_component(type) != nullptr)
                        continue;

                    if (ImGui::Button(
                            info.name.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0)
                        ))
                        entity.add_component(reflection, type);
                }
            }
            ImGui::EndChild();
            ImGui::EndPopup();
        }
    }

    ImGui::End();
}

void PropertiesEditorWorkspace::_initialize_draw_fnptrs(
    std::initializer_list<std::pair<std::uint64_t, fnptr_imgui_draw_property>> list
)
{
    for (const auto& [hash, fnptr] : list)
        m_draw_fnptrs.emplace(hash, fnptr);
}

void PropertiesEditorWorkspace::_imgui_draw(
    ecs::byte* object, std::set<ogl::MemberInfo>::iterator it,
    const std::set<ogl::MemberInfo>::iterator end, ogl::ReflectionRegistry* reflection
)
{
    if (it != end)
    {
        if (it->variable.get_flags() & ogl::VariableFlags_Const)
            return;

        if (m_draw_fnptrs.contains(it->type.get_id()))
        {
            ogl::TransformComponent* transform = reinterpret_cast<ogl::TransformComponent*>(object);

            ImGui::TableNextColumn();
            ImGui::Text("%s", it->fieldname.c_str());
            ImGui::TableNextColumn();

            fnptr_imgui_draw_property fnptr = m_draw_fnptrs[it->type.get_id()];
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
            fnptr(it->fieldname, static_cast<void*>(object + it->offset), m_step_size);
            ImGui::PopItemWidth();

            ImGui::TableNextRow();

            it++;
            _imgui_draw(object, it, end, reflection);
        }
        else
        {
            // TODO: Check for pointers types as it fucks with it
            if (reflection->type_contains_members(it->variable.get_type().get_id()))
            {
                // Not going to deal with pointers higher than 2
                if (it->variable.get_pointer_count() > 2)
                    return;

                ImGui::TableNextColumn();
                ImGui::Text(
                    "%s (%s)", it->fieldname.c_str(),
                    reflection->get_variable_type_name(it->variable).c_str()
                );
                ImGui::TableNextRow();

                if (it->variable.is_pointer())
                {
                    // TODO: ...
                }
                else if (it->variable.is_array())
                {
                    // TODO: ...
                }
                else
                {
                    const std::set<ogl::MemberInfo>& members = reflection->get_members(it->type);
                    ecs::byte* new_object = object + it->offset;
                    _imgui_draw(new_object, members.begin(), members.end(), reflection);
                }
            }
        }
    }
}

} // namespace oge
