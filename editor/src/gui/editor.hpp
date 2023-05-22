#ifndef __OGE_CORE_EDITOR_HPP__
#define __OGE_CORE_EDITOR_HPP__

#include <ogl/ogl.hpp>
#include <ogl/utils/yaml_serialization.hpp>

#define HIERARCHY_FILTER_NAME "@oge_editor"

#define PREF_NAME_SIZE 32

#define PREF_FIELD_EDITOR_UI "editor_ui"
#define PREF_FIELD_EDITOR_UI_COLOR "color_theme"

#define PREF_EDITOR_UI_FONT_REGULAR "font_regular"
#define PREF_EDITOR_UI_FONT_ITALIC "font_italics"
#define PREF_EDITOR_UI_FONT_BOLD "font_bold"
#define PREF_EDITOR_UI_FONT_BOLD_ITALIC "font_bold_italics"
#define PREF_EDITOR_UI_FONT_MONO "font_mono"

#define PREF_UI_COLOR_WINDOW_BG 0
#define PREF_UI_COLOR_HEADER 1
#define PREF_UI_COLOR_HEADER_HOVERED 2
#define PREF_UI_COLOR_HEADER_ACTIVE 3
#define PREF_UI_COLOR_BUTTON 4
#define PREF_UI_COLOR_BUTTON_HOVERED 5
#define PREF_UI_COLOR_BUTTON_ACTIVE 6
#define PREF_UI_COLOR_FRAME_BG 7
#define PREF_UI_COLOR_FRAME_BG_HOVERED 8
#define PREF_UI_COLOR_FRAME_BG_ACTIVE 9
#define PREF_UI_COLOR_TAB 10
#define PREF_UI_COLOR_TAB_HOVERED 11
#define PREF_UI_COLOR_TAB_ACTIVE 12
#define PREF_UI_COLOR_TAB_UNFOCUSED 13
#define PREF_UI_COLOR_TAB_UNFOCUSED_ACTIVE 14
#define PREF_UI_COLOR_TITLE_BG 15
#define PREF_UI_COLOR_TITLE_BG_ACTIVE 16
#define PREF_UI_COLOR_TITLE_BG_COLLAPSED 17

struct ImGuiIO;
struct ImFont;

namespace oge {

class PanelEditorWorkspaceBase {
  public:
    PanelEditorWorkspaceBase(std::string_view name);
    virtual ~PanelEditorWorkspaceBase() = default;

    inline const std::string& get_name() const { return m_name; }
    inline bool get_enabled() const { return m_enabled; }
    inline bool get_remove_when_disabled() const { return m_remove_when_disabled; }
    inline bool& get_enabled() { return m_enabled; }
    inline bool& get_remove_when_disabled() { return m_remove_when_disabled; }

    bool remove_modal_popup(std::string_view name);

    virtual void on_imgui_update() {}

  protected:
    inline const ImGuiIO* get_io() const { return m_io; }
    inline ImGuiIO* get_io() { return m_io; }

  private:
    std::string m_name{};
    ImGuiIO* m_io{nullptr};
    bool m_enabled{true};
    bool m_remove_when_disabled{false};
};

class EditorWorkspace : public ogl::ApplicationLayer {
  public:
    EditorWorkspace();
    virtual ~EditorWorkspace();

    template<typename _Panel, typename... _Args>
    _Panel* push_panel(_Args&&... args) {
        m_panels.push_back(new _Panel{std::forward<_Args>(args)...});
        return static_cast<_Panel*>(m_panels.back());
    }

    PanelEditorWorkspaceBase* get_panel(std::string_view name);
    const std::vector<PanelEditorWorkspaceBase*>& get_all_panels() { return m_panels; }
    void remove_panel(std::string_view name);

    void push_panels(std::initializer_list<PanelEditorWorkspaceBase*> panels);

    virtual void on_update() override;

  private:
    void _load_color_theme(ogl::YamlSerializationOption* ui_color);
    std::vector<PanelEditorWorkspaceBase*> m_panels{};
};

/******************************************************************************/
/******************************** Base Windows ********************************/
/******************************************************************************/

class DockingEditorWorkspace : public PanelEditorWorkspaceBase {
  public:
    DockingEditorWorkspace(EditorWorkspace* workspace);
    virtual ~DockingEditorWorkspace() override = default;

    virtual void on_imgui_update() override;

  private:
    void _menu_open_window(std::string_view panel_name);

    class EditorWorkspace* m_workspace{nullptr};
    int m_dock_node_flags{};
    int m_window_flags{};
};

class HierarchyEditorWorkspace : public PanelEditorWorkspaceBase {
  public:
    HierarchyEditorWorkspace();
    virtual ~HierarchyEditorWorkspace() override = default;

    inline ecs::Entity get_selected_entity() const { return m_selected_entity; }

    virtual void on_imgui_update() override;

  private:
    void _draw_entity(ogl::Entity entity, ecs::Entity& entity_clicked, bool& opened_popup);
    void _create_shape(
        std::string_view new_entity_name, std::string_view mesh_file_path, ogl::Entity* parent
    );
    void _popup_menu(ogl::Entity* entity = nullptr);

    ecs::Entity m_selected_entity = {ECS_ENTITY_DESTROYED};
    std::vector<ogl::Entity> m_deleted_entity = {};
};

typedef void (*fnptr_property_imgui_draw)(ogl::Entity&);
typedef bool (*fnptr_property_comp_exists)(ogl::Entity&);

struct PropertyImGuiDraw {
    std::string name{};
    fnptr_property_comp_exists exists{nullptr};
    fnptr_property_imgui_draw draw{nullptr};

    PropertyImGuiDraw() = default;
    PropertyImGuiDraw(
        const std::string& name, fnptr_property_comp_exists exists, fnptr_property_imgui_draw draw
    );
    PropertyImGuiDraw(const PropertyImGuiDraw& other);
    PropertyImGuiDraw(PropertyImGuiDraw&& other);

    PropertyImGuiDraw& operator=(const PropertyImGuiDraw& other);
    PropertyImGuiDraw& operator=(PropertyImGuiDraw&& other);
};

class PropertiesEditorWorkspace : public PanelEditorWorkspaceBase {
  public:
    PropertiesEditorWorkspace(HierarchyEditorWorkspace* hierarchy);
    virtual ~PropertiesEditorWorkspace() override = default;

    void push_properties(const std::initializer_list<PropertyImGuiDraw> list);

    virtual void on_imgui_update() override;

  private:
    HierarchyEditorWorkspace* m_hierarchy{nullptr};
    std::vector<PropertyImGuiDraw> m_properties{};
};

class ConsoleEditorWorkspace : public PanelEditorWorkspaceBase {
  public:
    ConsoleEditorWorkspace(ogl::Debug* debug);
    virtual ~ConsoleEditorWorkspace() override = default;

    virtual void on_imgui_update() override;

  private:
    glm::vec4 m_debug_colors[2]{
        glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)};
    std::tuple<bool, std::string> m_filters[ogl::debug_type_count];
    bool m_auto_scrolling{true};
    ogl::Debug* m_debug{nullptr};
    ImFont* m_font{nullptr};
};

class AssetsEditorWorkspace : public PanelEditorWorkspaceBase {
  public:
    AssetsEditorWorkspace();
    virtual ~AssetsEditorWorkspace() override = default;

    virtual void on_imgui_update() override;
};

class ViewportEditorWorkspace : public PanelEditorWorkspaceBase {
  public:
    ViewportEditorWorkspace(ogl::Framebuffer* framebuffer);
    virtual ~ViewportEditorWorkspace() override = default;

    virtual void on_imgui_update() override;

  private:
    void _camera_controller();

    glm::ivec2 m_last_required_framebuffer_size{};
    ogl::Framebuffer* m_framebuffer{nullptr};

    ogl::CameraComponent* m_camera{nullptr};
    ogl::CameraComponent* m_scene_main_camera{nullptr};
    float m_camera_move_speed{5.0f};
    glm::vec2 m_camera_sensitivity{0.05f, 0.05f};
    float m_yaw{0.0f};
    float m_pitch{0.0f};
};

/******************************************************************************/
/******************************* Pop Up Windows *******************************/
/******************************************************************************/

class PreferencesMenuBase {
  public:
    PreferencesMenuBase(
        const std::string& name, const std::string* path,
        ogl::YamlSerializationOption* target_field, class PreferencesEditorPopup* preferences
    )
        : m_name(name), m_field(target_field), m_path(path), m_preferences(preferences) {}
    virtual ~PreferencesMenuBase() = default;
    const std::string& get_name() const { return m_name; }
    inline bool failed_to_get_field() const { return m_field == nullptr; }
    inline const std::string& get_path() const { return *m_path; }
    inline class PreferencesEditorPopup* get_preferences() { return m_preferences; }

    virtual void on_imgui_draw(bool& is_unsaved) = 0;
    virtual void on_save() {}
    virtual void on_no_save() {}

  protected:
    ogl::YamlSerializationOption* get_field() { return m_field; }

  private:
    ogl::YamlSerializationOption* m_field{nullptr};
    std::string m_name{};
    const std::string* m_path{nullptr};
    class PreferencesEditorPopup* m_preferences{nullptr};
};

class PreferencesEditorPopup : public PanelEditorWorkspaceBase {
  public:
    PreferencesEditorPopup();
    virtual ~PreferencesEditorPopup() override = default;

    inline ogl::YamlSerialization* get_config() { return &m_config; }

    virtual void on_imgui_update() override;

  private:
    ogl::YamlSerialization m_config{};
    std::string m_path{};
    bool m_unsaved = false;

    std::vector<PreferencesMenuBase*> m_settings;
    std::size_t m_selected_index{std::string::npos};
    PreferencesMenuBase* m_selected_menu{nullptr};
};

} // namespace oge

#endif
