#include "gui/editor.hpp"

#include <ogl/utils/filesystem.hpp>

#include <GLFW/glfw3.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>

namespace oge {

struct LayoutData {
    ogl::YamlSerializationOption* layout_names{nullptr};
    size_t selected_index{std::string::npos};
    size_t last_selected_index{std::string::npos};
    char change_name[PREF_NAME_SIZE]{};
    bool delete_current{false};
};

class PreferencesLayoutAndGui : public PreferencesMenuBase {
  public:
    PreferencesLayoutAndGui(ogl::YamlSerializationOption* target_field, const std::string* path)
        : PreferencesMenuBase("GUI and Layout", path, target_field) {
        if (get_field() != nullptr) {
            m_layout_data.layout_names = get_field()->get("saved_layouts");

            if (m_layout_data.layout_names == nullptr) {
                ogl::Debug::log(
                    "Failed to retreave saved layout names in preferences.yaml",
                    ogl::DebugType_Error
                );
            }
        }
    }

    virtual void on_imgui_draw(bool& is_unsaved) override {
        _layouts(is_unsaved);
        _fonts(is_unsaved);
        _colors(is_unsaved);
    }

  private:
    void _layouts(bool& is_unsaved) {
        ImGui::SeparatorText("Layouts");

        const float height = ImGui::GetContentRegionAvail().y * 0.25f;
        std::vector<std::string> saved_names =
            m_layout_data.layout_names->convert_value_into_vector<std::string>();
        {
            ImGui::BeginChild(
                "List of saved layouts", ImVec2(ImGui::GetContentRegionAvail().x * 0.30f, height),
                true, ImGuiWindowFlags_NoTitleBar
            );
            for (size_t i = 0; i < saved_names.size(); i++) {
                int tree_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth |
                                 ImGuiTreeNodeFlags_NoTreePushOnOpen;

                if (m_layout_data.selected_index == i) {
                    tree_flags |= ImGuiTreeNodeFlags_Selected;

                    if (m_layout_data.delete_current) {
                        m_layout_data.delete_current = false;

                        saved_names.erase(saved_names.begin() + i);
                        m_layout_data.selected_index = std::string::npos;
                        m_layout_data.layout_names->value =
                            ogl::YamlSerialization::format_vector_into_valid_value(saved_names);

                        continue;
                    }
                }

                ImGui::TreeNodeEx(saved_names[i].c_str(), tree_flags);
                if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
                    m_layout_data.selected_index = i;
                }
            }
            ImGui::EndChild();
        }

        ImGui::SameLine();

        {
            if (m_layout_data.last_selected_index != m_layout_data.selected_index) {
                m_layout_data.last_selected_index = m_layout_data.selected_index;
                // TODO: reset data
            }

            ImGui::BeginChild(
                "Layout settings", ImVec2(0, height), true, ImGuiWindowFlags_NoTitleBar
            );
            if (m_layout_data.selected_index != std::string::npos) {
                if (saved_names[m_layout_data.selected_index] != "default") {
                    ImGui::InputText("Change Name", m_layout_data.change_name, PREF_NAME_SIZE);

                    if (ImGui::Button("Delete")) {
                        m_layout_data.delete_current = true;
                        is_unsaved = true;
                    }
                }

                if (ImGui::Button("Load when startup")) {
                }
            }
            ImGui::EndChild();
        }

        if (strlen(m_layout_data.change_name) > 0) {
            is_unsaved = true;
        }
    }

    void _fonts(bool& is_unsaved) {
        ImGui::SeparatorText("Fonts");
        ImGui::Text("Comfing Soon ...");
    }

    void _colors(bool& is_unsaved) {
        ImGui::SeparatorText("Gui Colors");
        ImGui::Text("Comfing Soon ...");
    }

  private:
    LayoutData m_layout_data{};
};

class PreferencesKeyBindings : public PreferencesMenuBase {
  public:
    PreferencesKeyBindings(ogl::YamlSerializationOption* target_field, const std::string* path)
        : PreferencesMenuBase("Key Bindings", path, target_field) {}

    virtual void on_imgui_draw(bool& is_unsaved) override { ImGui::Text("Coming Soon ..."); }
};

PreferencesEditorPopup::PreferencesEditorPopup() : PanelEditorWorkspaceBase("Preferences") {
#ifndef WIN32
    m_path = ogl::FileSystem::get_env_var("HOME") + "/.config/oge/";
#else
    m_path = ogl::FileSystem::get_env_var("APPDATA") + "\\oge\\";
#endif

    conf = ogl::YamlSerialization(std::string(m_path + "preferences.yaml").c_str());
    if (!conf.is_null()) {
        m_settings = {
            new PreferencesLayoutAndGui(conf.get(PREF_FIELD_EDITOR_UI), &m_path),
            // TODO: Need to change the field when finally gonna implement this functionality
            new PreferencesKeyBindings(conf.get(PREF_FIELD_EDITOR_UI), &m_path),
        };

        // Validation
        for (PreferencesMenuBase* menu : m_settings) {
            if (menu->failed_to_get_field()) {
                ogl::Debug::log(
                    "Failed to load Preference Menu: " + menu->get_name() +
                    ", as it could not find the options needed in preferences.yaml [" + m_path + "]"
                );
                get_enabled() = false;
                break;
            }
        }
    } else {
        ogl::Debug::log(
            "Failed to open Preferences as the preferences yaml file at [" + m_path +
                "] doesn't exist",
            ogl::DebugType_Error
        );
        get_enabled() = false;
    }

    get_remove_when_disabled() = true;
    m_unsaved = false;
}

void PreferencesEditorPopup::on_imgui_update() {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking;
    if (m_unsaved) {
        window_flags |= ImGuiWindowFlags_UnsavedDocument;
    }

    ImGui::Begin(get_name().c_str(), nullptr, window_flags);

    if (m_unsaved) {
        if (ImGui::Button("Save Changes")) {
            m_unsaved = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Close")) {
            ImGui::OpenPopup("Don't Save?");
        }

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Don't Save?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Sure you want to close? Will lose all unsaved changes!");

            if (ImGui::Button("Ok")) {
                get_enabled() = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    } else {
        if (ImGui::Button("Close")) {
            get_enabled() = false;
        }
    }

    // Selecting what settings to edit
    ImGui::BeginChild(
        "Select", ImVec2(ImGui::GetContentRegionAvail().x * 0.15, 0), false,
        ImGuiWindowFlags_NoTitleBar
    );
    for (size_t i = 0; i < m_settings.size(); i++) {
        int tree_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen |
                         ImGuiTreeNodeFlags_SpanFullWidth;
        if (i == m_selected_index) {
            tree_flags |= ImGuiTreeNodeFlags_Selected;
        }

        ImGui::TreeNodeEx(m_settings[i]->get_name().c_str(), tree_flags);
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            m_selected_index = i;
            m_selected_menu = m_settings[i];
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    if (m_selected_menu != nullptr) {
        // Draw menu settings
        ImGui::BeginChild(
            "Selected Menu", ImVec2(ImGui::GetContentRegionAvail().x, 0), true,
            ImGuiWindowFlags_NoTitleBar
        );
        m_selected_menu->on_imgui_draw(m_unsaved);
        ImGui::EndChild();
    } else {
        // Draw empty settings
        ImGui::BeginChild(
            "Selected Menu", ImVec2(ImGui::GetContentRegionAvail().x, 0), true,
            ImGuiWindowFlags_NoTitleBar
        );
        ImGui::EndChild();
    }

    ImGui::End();
}
} // namespace oge
