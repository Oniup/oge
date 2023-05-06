#ifndef __OGE_CORE_EDITOR_HPP__
#define __OGE_CORE_EDITOR_HPP__

#include <ogl/ogl.hpp>
#include <ogl/utils/yaml_serialization.hpp>

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

    template<typename _Panel, typename... _Args> _Panel* push_panel(_Args&&... args) {
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

    inline constexpr entt::entity get_non_selected_entity_value() const {
        return static_cast<entt::entity>(std::numeric_limits<uint32_t>().max());
    }
    inline entt::entity get_selected_entity() const { return m_selected_entity; }

    virtual void on_imgui_update() override;

  private:
    entt::entity m_selected_entity{
        static_cast<entt::entity>(std::numeric_limits<uint32_t>().max())};
};

class PropertiesEditorWorkspace : public PanelEditorWorkspaceBase {
  public:
    PropertiesEditorWorkspace(HierarchyEditorWorkspace* hierarchy);
    virtual ~PropertiesEditorWorkspace() override = default;

    virtual void on_imgui_update() override;

  private:
    HierarchyEditorWorkspace* m_hierarchy{nullptr};
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
    glm::ivec2 m_last_required_framebuffer_size{};
    ogl::Framebuffer* m_framebuffer{nullptr};
};

/******************************************************************************/
/******************************* Pop Up Windows *******************************/
/******************************************************************************/

typedef void (*fnptr_preferences_settings_menu)(const std::string&, bool&);

class PreferencesEditorPopup : public PanelEditorWorkspaceBase {
  public:
    PreferencesEditorPopup();
    virtual ~PreferencesEditorPopup() override = default;

    virtual void on_imgui_update() override;

  private:
    ogl::YamlSerialization conf{};
    std::string m_path{};
    bool m_unsaved = false;

    std::vector<std::tuple<const std::string, fnptr_preferences_settings_menu>> m_settings;
    size_t m_selected_index{std::string::npos};
    std::tuple<const std::string, fnptr_preferences_settings_menu>* m_selected_menu{nullptr};
};

} // namespace oge

#endif
