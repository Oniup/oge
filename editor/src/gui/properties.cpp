#include "gui/properties.hpp"
#include "gui/editor.hpp"

#include <kryos/core/application.hpp>
#include <kryos/scene/entity.hpp>
#include <kryos/scene/scene_manager.hpp>
#include <kryos/scene/components.hpp>
#include <kryos/serialization/reflection.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace workspace {

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

KProperties::KProperties(KHierarchy* hierarchy)
    : KIWorkspace("Properties"), m_hierarchy(hierarchy)
{
    // PERFORMANCE: probably be better if this constexpr
    _initialize_draw_fnptrs({
        {KTypeId::create<std::int32_t>().get_id(), int_draw},
        {KTypeId::create<std::int64_t>().get_id(), int_draw},
        {KTypeId::create<std::uint32_t>().get_id(), int_draw},
        {KTypeId::create<std::uint64_t>().get_id(), int_draw},

        {KTypeId::create<float>().get_id(), float_draw},
        {KTypeId::create<bool>().get_id(), bool_draw},

        {KTypeId::create<std::string>().get_id(), str_draw},

        {KTypeId::create<glm::vec2>().get_id(), vec2_draw},
        {KTypeId::create<glm::vec3>().get_id(), vec3_draw},
        {KTypeId::create<glm::vec4>().get_id(), vec4_draw},

        {KTypeId::create<glm::ivec2>().get_id(), ivec2_draw},
        {KTypeId::create<glm::ivec3>().get_id(), ivec3_draw},
        {KTypeId::create<glm::ivec4>().get_id(), ivec4_draw},
    });
}

void KProperties::on_imgui_update()
{
    ImGui::Begin(get_name().c_str(), &get_enabled(), ImGuiWindowFlags_MenuBar);
    static bool add_component_popup = false;

    if (ImGui::BeginMenuBar())
    {
        if (m_hierarchy->get_selected_entity() != ECS_ENTITY_DESTROYED)
        {
            if (ImGui::MenuItem("Add Component"))
                add_component_popup = true;

            KEntity entity = m_hierarchy->get_selected_entity();
            if (entity.get_component<KCName>() == nullptr)
            {
                if (ImGui::MenuItem("Add Name"))
                    entity.add_component<KCName>();
            }

            if (entity.get_component<KCTag>() == nullptr)
            {
                if (ImGui::MenuItem("Add Tag"))
                    entity.add_component<KCTag>();
            }
        }

        ImGui::EndMenuBar();
    }

    if (m_hierarchy->get_selected_entity() != ECS_ENTITY_DESTROYED)
    {
        KEntity entity = m_hierarchy->get_selected_entity();
        KScene* scene = KIApplication::get_layer<KLSceneManager>()->get_active_scene();
        KLReflectionRegistry* reflection = KIApplication::get_layer<KLReflectionRegistry>();

        KCName* name_comp = entity.get_component<KCName>();
        KCTag* tag_comp = entity.get_component<KCTag>();

        static char str[KRYOS_NAME_COMPONENT_MAX_SIZE] = {};
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        if (name_comp != nullptr)
        {
            strncpy(str, name_comp->name.c_str(), name_comp->name.size());
            str[name_comp->name.size()] = '\0';

            ImGui::InputText("##NameComponent", str, KRYOS_NAME_COMPONENT_MAX_SIZE);
            name_comp->name = str;
        }

        if (tag_comp != nullptr)
        {
            strncpy(str, tag_comp->tag.c_str(), tag_comp->tag.size());
            str[tag_comp->tag.size()] = '\0';

            ImGui::InputText("##NameComponent", str, KRYOS_NAME_COMPONENT_MAX_SIZE);
            tag_comp->tag = str;
        }
        ImGui::PopItemWidth();

        ImGui::NewLine();
        ImGui::Separator();

        for (ecs::ObjectPool* pool : scene->get_registry().get_pools())
        {
            if (pool->get_type_hash() == KTypeId::create<KCName>().get_id() ||
                pool->get_type_hash() == KTypeId::create<KCTag>().get_id() ||
                pool->get_type_hash() == KTypeId::create<KCParent>().get_id())
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
                            const std::set<KMemberInfo>& members =
                                reflection->get_members(KTypeId(pool->get_type_hash()));

                            KSerializeData data = {};
                            data.object = object;
                            data.iter = members.begin();
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
            KLReflectionRegistry* reflection = KIApplication::get_layer<KLReflectionRegistry>();
            ImGui::BeginChild("List", ImVec2(popup_width, 300));
            {
                for (const auto& [type, info] : reflection->get_all_type_infos())
                {
                    if (info.flags & KETypeInfoFlag_Component)
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

void KProperties::_initialize_draw_fnptrs(
    std::initializer_list<std::pair<std::uint64_t, fnptr_imgui_draw_property>> list
)
{
    for (const auto& [hash, fnptr] : list)
        m_draw_fnptrs.emplace(hash, fnptr);
}

void KProperties::_imgui_draw(KSerializeData& data)
{
    // NOTE: Not going to deal with pointers higher than 2, dealing with single pointer is enough
    if (data.iter == data.end)
        return;

    if (data.iter->variable.get_pointer_count() > 2 ||
        data.iter->variable.get_flags() & KEVariableFlag_Const ||
        data.iter->flags & KEMemberInfoEditorFlag_Hide)
    {
        data.iter++;
        _imgui_draw(data);
        return;
    }

    const KTypeInfo& stripped_info = data.reflection->get_type_info(data.iter->variable.get_type());

    if (stripped_info.flags & KETypeInfoFlag_StdVector)
        _imgui_draw_std_vector(data, stripped_info);
    if (stripped_info.flags & KETypeInfoFlag_StdArray)
        _imgui_draw_std_array(data);
    else if (m_draw_fnptrs.contains(data.iter->variable.get_type().get_id()))
    {
        ImGui::TableNextColumn();
        ImGui::Text("%s", data.iter->fieldname.c_str());
        ImGui::TableNextColumn();

        fnptr_imgui_draw_property fnptr = m_draw_fnptrs[data.iter->variable.get_type().get_id()];
        if (data.iter->variable.is_array())
            _imgui_draw_array(data, stripped_info, fnptr);
        else
        {
            // Primitive type that can easly be printed
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
            fnptr(
                data.iter->fieldname, static_cast<void*>(data.object + data.iter->offset),
                m_step_size
            );
            ImGui::PopItemWidth();

            ImGui::TableNextRow();
        }

        data.iter++;
        _imgui_draw(data);
    }
    else
        _imgui_draw_non_primitive(data);
}

void KProperties::_imgui_draw_std_vector(
    KSerializeData& data, const KTypeInfo& vector_inner_type_info
)
{
    ImGui::TableNextColumn();
    ImGui::Text("%s", data.iter->fieldname.c_str());
    ImGui::TableNextColumn();

    assert(
        data.reflection->is_templated_type(data.iter->type) &&
        "Attempted to create vector property in editor, however type is not in the "
        "templated reflection registry"
    );

    // There should only be one type for vector
    KTypeId internal_type =
        KTypeId(data.reflection->get_templated_internal_types(data.iter->type).front());

    if (m_draw_fnptrs.contains(internal_type.get_id()))
    {
        const KTypeInfo& internal_type_info =
            data.reflection->get_type_info(KTypeId(internal_type));

        KVectorInternalStructor* internal_structure =
            reinterpret_cast<KVectorInternalStructor*>(data.object + data.iter->offset);
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
                    data.iter->fieldname + std::to_string(i),
                    internal_structure->begin + (internal_type_info.size * i), m_step_size
                );
                ImGui::PopItemWidth();
            }
        }

        ImGui::TableNextRow();
        data.iter++;
        _imgui_draw(data);
    }
    else
        ImGui::Text(
            "%s not supported editable type, structures will come soon",
            vector_inner_type_info.name.c_str()
        );
}

void KProperties::_imgui_draw_std_array(KSerializeData& data)
{
    ImGui::TableNextColumn();
    ImGui::Text("%s", data.iter->fieldname.c_str());
    ImGui::TableNextColumn();
    ImGui::Text("std::array coming soon...");
    ImGui::TableNextRow();
}

void KProperties::_imgui_draw_array(
    KSerializeData& data, const KTypeInfo& info, fnptr_imgui_draw_property fnptr
)
{
    ImGui::TableNextRow();

    std::byte* array_begin = data.object + data.iter->offset;
    for (std::size_t i = 0; i < data.iter->variable.get_array_size(); i++)
    {
        ImGui::TableNextColumn();

        std::string number_str = "(" + std::to_string(i) + ")";
        float number_str_size = ImGui::CalcTextSize(number_str.c_str()).x;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - number_str_size);
        ImGui::Text("%s", number_str.c_str());

        ImGui::TableNextColumn();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        std::byte* element = array_begin + (i * info.size);
        fnptr(data.iter->fieldname + std::to_string(i), element, m_step_size);
        ImGui::PopItemWidth();

        ImGui::TableNextRow();
    }
}

void KProperties::_imgui_draw_non_primitive(KSerializeData& data)
{
    if (data.reflection->type_contains_members(data.iter->variable.get_type().get_id()))
    {
        KEMemberInfoEditorFlags flags = data.iter->flags;
        ImGui::TableNextColumn();
        ImGui::Text(
            "%s (%s)", data.iter->fieldname.c_str(),
            data.reflection->get_variable_type_name(data.iter->variable).c_str()
        );
        ImGui::TableNextRow();

        if (data.iter->variable.is_pointer())
        {
            if (flags & KEMemberInfoEditorFlag_NeverOwnsPtrData)
            {
                // TODO: ...
            }
            else if (flags & KEMemberInfoEditorFlag_OwnsPtrData)
            {
                // TODO: ...
            }
        }
        else if (data.iter->variable.is_array())
        {
            // TODO: ...
        }
        else
        {
            const std::set<KMemberInfo>& members = data.reflection->get_members(data.iter->type);

            KSerializeData new_object = {};
            new_object.object = data.object + data.iter->offset;
            new_object.iter = members.begin();
            new_object.end = members.end();
            new_object.reflection = data.reflection;

            _imgui_draw(new_object);
        }
    }
}

} // namespace workspace
