#include "gui/editor.hpp"

#include <imgui/imgui.h>
#include <ogl/utils/filesystem.hpp>

namespace oge {

struct LayoutData {
    ogl::YamlSerializationOption* layout_names{nullptr};
    size_t selected_index{std::string::npos};
    size_t last_selected_index{std::string::npos};
    char change_name[PREF_NAME_SIZE]{};
    bool delete_current{false};

    char create_new_layout_name[PREF_NAME_SIZE]{};
    std::vector<std::string> copy_files_into_preferences{};
};

class PreferencesLayoutAndGui : public PreferencesMenuBase {
  public:
    PreferencesLayoutAndGui(
        ogl::YamlSerializationOption* target_field, const std::string* path,
        PreferencesEditorPopup* preferences
    )
        : PreferencesMenuBase("GUI and Layout", path, target_field, preferences) {
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
        std::string start_layout = get_field()->get("layout")->convert_value<std::string>();

        ImGui::InputText(" ", m_layout_data.create_new_layout_name, PREF_NAME_SIZE);
        ImGui::SameLine();
        if (ImGui::Button("Create Layout")) {
            if (strlen(m_layout_data.create_new_layout_name) > 0) {
                m_layout_data.copy_files_into_preferences.push_back(
                    m_layout_data.create_new_layout_name
                );

                saved_names.push_back(m_layout_data.create_new_layout_name);
                m_layout_data.layout_names->value =
                    ogl::YamlSerialization::format_vector_into_valid_value(saved_names);

                ogl::FileSystem::copy_file(
                    "imgui.ini", std::string(m_layout_data.create_new_layout_name) + ".ini"
                );

                m_layout_data.create_new_layout_name[0] = '\0';
                is_unsaved = true;
            }
        }

        ImGui::BeginChild(
            "List of saved layouts", ImVec2(ImGui::GetContentRegionAvail().x * 0.30f, height), true,
            ImGuiWindowFlags_NoTitleBar
        );
        {
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
        }
        ImGui::EndChild();

        if (m_layout_data.last_selected_index != m_layout_data.selected_index) {
            m_layout_data.last_selected_index = m_layout_data.selected_index;
            // TODO: reset data
        }

        ImGui::SameLine();
        ImGui::BeginChild("Layout settings", ImVec2(0, height), true, ImGuiWindowFlags_NoTitleBar);
        {
            if (m_layout_data.selected_index != std::string::npos) {
                if (saved_names[m_layout_data.selected_index] != "default") {
                    ImGui::InputText("Change Name", m_layout_data.change_name, PREF_NAME_SIZE);

                    if (ImGui::Button("Delete")) {
                        m_layout_data.delete_current = true;
                        is_unsaved = true;
                    }
                }

                if (start_layout != saved_names[m_layout_data.selected_index]) {
                    if (ImGui::Button("Load when startup")) {
                        get_field()->get("layout")->value =
                            "'" + saved_names[m_layout_data.selected_index] + "'";
                        is_unsaved = true;
                    }
                }
            }
        }
        ImGui::EndChild();

        if (strlen(m_layout_data.change_name) > 0) {
            is_unsaved = true;
        }
    }

    virtual void on_save() override {
        // Copy new layouts to where preferences is saved
        size_t offset = get_preferences()->get_config()->get_filename().find_last_of("/");
        const std::string path =
            std::string(get_preferences()->get_config()->get_filename(), offset);

        for (const std::string& filename : m_layout_data.copy_files_into_preferences) {
            ogl::FileSystem::copy_file(filename, path + filename);
            std::remove(filename.c_str());
        }

        m_layout_data.copy_files_into_preferences.clear();
    }

    virtual void on_no_save() override {
        for (const std::string& filename : m_layout_data.copy_files_into_preferences) {
            std::remove(filename.c_str());
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
    PreferencesKeyBindings(
        ogl::YamlSerializationOption* target_field, const std::string* path,
        PreferencesEditorPopup* preferences
    )
        : PreferencesMenuBase("Key Bindings", path, target_field, preferences) {}

    virtual void on_imgui_draw(bool& is_unsaved) override { ImGui::Text("Coming Soon ..."); }
};

PreferencesEditorPopup::PreferencesEditorPopup() : PanelEditorWorkspaceBase("Preferences") {
#ifndef WIN32
    m_path = ogl::FileSystem::get_env_var("HOME") + "/.config/oge/";
#else
    m_path = ogl::FileSystem::get_env_var("APPDATA") + "\\oge\\";
#endif

    m_config = ogl::YamlSerialization(std::string(m_path + "preferences.yaml").c_str());
    if (!m_config.is_null()) {
        m_settings = {
            new PreferencesLayoutAndGui(m_config.get(PREF_FIELD_EDITOR_UI), &m_path, this),
            // TODO: Need to change the field when finally gonna implement this functionality
            new PreferencesKeyBindings(m_config.get(PREF_FIELD_EDITOR_UI), &m_path, this),
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
    {
        if (m_unsaved) {
            if (ImGui::Button("Save Changes")) {
                for (PreferencesMenuBase* menu : m_settings) {
                    menu->on_save();
                }
                m_unsaved = false;
                m_config.write_changes();
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
            { m_selected_menu->on_imgui_draw(m_unsaved); }
            ImGui::EndChild();
        } else {
            // Draw empty settings
            ImGui::BeginChild(
                "Selected Menu", ImVec2(ImGui::GetContentRegionAvail().x, 0), true,
                ImGuiWindowFlags_NoTitleBar
            );
            ImGui::EndChild();
        }
    }
    ImGui::End();
}
} // namespace oge
