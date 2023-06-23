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
    // PERFORMANCE: probably be better if this constexpr
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

            std::byte* object = reinterpret_cast<std::byte*>(pool->get_entitys_object(entity));
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
                            ImGui::TableSetupColumn(
                                "Names", ImGuiTableColumnFlags_WidthFixed,
                                ImGui::GetContentRegionAvail().x * 0.25f
                            );
                            ImGui::TableSetupColumn(
                                "settings", ImGuiTableColumnFlags_WidthFixed,
                                ImGui::GetContentRegionAvail().x * 0.75f
                            );
                            const std::set<ogl::MemberInfo>& members =
                                reflection->get_members(ogl::TypeId(pool->get_type_hash()));

                            StructDrawData data = {};
                            data.object = object;
                            data.it = members.begin();
                            data.end = members.end();
                            data.reflection = reflection;
                            _imgui_draw(data);
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
                    if (info.flags & ogl::TypeInfoFlags_Component)
                    {
                        if (entity.get_component(type) != nullptr)
                            continue;

                        if (ImGui::Button(
                                info.name.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0)
                            ))
                        {
                            entity.add_component(reflection, type);
                            break;
                        }
                    }
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

void PropertiesEditorWorkspace::_imgui_draw(StructDrawData& data)
{
    // NOTE: Not going to deal with pointers higher than 2, dealing with single pointer is enough
    if (data.it == data.end)
        return;

    if (data.it->variable.get_pointer_count() > 2 ||
        data.it->variable.get_flags() & ogl::VariableFlags_Const ||
        data.it->flags & ogl::MemberInfoEditorFlag_Hide)
    {
        data.it++;
        _imgui_draw(data);
        return;
    }

    const ogl::TypeInfo& info = data.reflection->get_type_info(data.it->type);
    if (info.flags & ogl::TypeInfoFlags_StdVector)
        _imgui_draw_vector(data, info);
    if (info.flags & ogl::TypeInfoFlags_StdArray)
    {
    }
    else if (m_draw_fnptrs.contains(data.it->type.get_id()))
    {
        ImGui::TableNextColumn();
        ImGui::Text("%s", data.it->fieldname.c_str());
        ImGui::TableNextColumn();

        // Primitive type that can easly be printed
        fnptr_imgui_draw_property fnptr = m_draw_fnptrs[data.it->type.get_id()];
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        fnptr(data.it->fieldname, static_cast<void*>(data.object + data.it->offset), m_step_size);
        ImGui::PopItemWidth();

        ImGui::TableNextRow();
        data.it++;
        _imgui_draw(data);
    }
    else
        _imgui_draw_non_primitive(data);
}

void PropertiesEditorWorkspace::_imgui_draw_vector(
    StructDrawData& data, const ogl::TypeInfo& vector_inner_type_info
)
{
    ImGui::TableNextColumn();
    ImGui::Text("%s", data.it->fieldname.c_str());
    ImGui::TableNextColumn();

    assert(
        data.reflection->is_templated_type(data.it->type) &&
        "Attempted to create vector property in editor, however type is not in the "
        "templated reflection registry"
    );

    // There should only be one type for vector
    ogl::TypeId internal_type =
        ogl::TypeId(data.reflection->get_templated_internal_types(data.it->type).front());

    if (m_draw_fnptrs.contains(internal_type.get_id()))
    {
        const ogl::TypeInfo& internal_type_info =
            data.reflection->get_type_info(ogl::TypeId(internal_type));

        ogl::VectorInternalStructor* internal_structure =
            reinterpret_cast<ogl::VectorInternalStructor*>(data.object + data.it->offset);
        std::size_t vector_size =
            (internal_structure->end - internal_structure->begin) / internal_type_info.size;

        // Printing the vector
        if (vector_size > 0)
        {
            ImGui::TableNextRow();

            fnptr_imgui_draw_property fnptr = m_draw_fnptrs[internal_type.get_id()];
            for (std::size_t i = 0; i < vector_size; i++)
            {
                ImGui::TableNextColumn();
                std::string number_str = "(" + std::to_string(i) + ")";
                float number_str_size = ImGui::CalcTextSize(number_str.c_str()).x;
                ImGui::SetCursorPosX(
                    ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - number_str_size
                );

                ImGui::Text("%s", number_str.c_str());
                ImGui::TableNextColumn();

                ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
                fnptr(
                    data.it->fieldname + std::to_string(i),
                    internal_structure->begin + (internal_type_info.size * i), m_step_size
                );
                ImGui::PopItemWidth();
            }
        }

        ImGui::TableNextRow();
        data.it++;
        _imgui_draw(data);
    }
    else
        ImGui::Text(
            "%s not supported editable type, structures will come soon",
            vector_inner_type_info.name.c_str()
        );
}

void PropertiesEditorWorkspace::_imgui_draw_non_primitive(StructDrawData& data)
{
    if (data.reflection->type_contains_members(data.it->variable.get_type().get_id()))
    {
        ogl::MemberInfoEditorFlags flags = data.it->flags;
        ImGui::TableNextColumn();
        ImGui::Text(
            "%s (%s)", data.it->fieldname.c_str(),
            data.reflection->get_variable_type_name(data.it->variable).c_str()
        );
        ImGui::TableNextRow();

        if (data.it->variable.is_pointer())
        {
            if (flags & ogl::MemberInfoEditorFlag_NeverOwnsPtrData)
            {
                // TODO: ...
            }
            else if (flags & ogl::MemberInfoEditorFlag_OwnsPtrData)
            {
                // TODO: ...
            }
        }
        else if (data.it->variable.is_array())
        {
            // TODO: ...
        }
        else
        {
            const std::set<ogl::MemberInfo>& members = data.reflection->get_members(data.it->type);

            StructDrawData new_object = {};
            new_object.object = data.object + data.it->offset;
            new_object.it = members.begin();
            new_object.end = members.end();
            new_object.reflection = data.reflection;

            _imgui_draw(new_object);
        }
    }
}

} // namespace oge
