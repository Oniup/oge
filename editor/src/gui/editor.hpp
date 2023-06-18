#ifndef __OGE_CORE_EDITOR_HPP__
#define __OGE_CORE_EDITOR_HPP__

#include <ogl/ogl.hpp>
#include <yaml/yaml.hpp>

#define HIERARCHY_FILTER_NAME "@oge_editor"

#define PREF_NAME_SIZE 32

struct ImGuiIO;
struct ImFont;

namespace oge {

class PanelEditorWorkspaceBase
{
  public:
    PanelEditorWorkspaceBase(const std::string& name);
    virtual ~PanelEditorWorkspaceBase() = default;

    inline const std::string& get_name() const { return m_name; }
    inline bool get_enabled() const { return m_enabled; }
    inline bool get_remove_when_disabled() const { return m_remove_when_disabled; }
    inline bool& get_enabled() { return m_enabled; }
    inline bool& get_remove_when_disabled() { return m_remove_when_disabled; }

    bool remove_modal_popup(const std::string& name);

    virtual void on_imgui_update() {}

  protected:
    inline const ImGuiIO* get_io() const { return m_io; }
    inline ImGuiIO* get_io() { return m_io; }

  private:
    std::string m_name = {};
    ImGuiIO* m_io = nullptr;
    bool m_enabled = true;
    bool m_remove_when_disabled = false;
};

class EditorWorkspace : public ogl::ApplicationLayer
{
  public:
    EditorWorkspace();
    virtual ~EditorWorkspace();

    template<typename _Panel, typename... _Args>
    _Panel* push_panel(_Args&&... args)
    {
        m_panels.push_back(new _Panel{std::forward<_Args>(args)...});
        return static_cast<_Panel*>(m_panels.back());
    }

    PanelEditorWorkspaceBase* get_panel(const std::string& name);
    const std::vector<PanelEditorWorkspaceBase*>& get_all_panels() { return m_panels; }
    void remove_panel(const std::string& name);

    void push_panels(std::initializer_list<PanelEditorWorkspaceBase*> panels);

    virtual void on_update() override;

  private:
    void _load_colors(yaml::Node& color);
    void _load_styles(yaml::Node& styles);

  private:
    std::vector<PanelEditorWorkspaceBase*> m_panels{};
};

/******************************************************************************/
/******************************** Base Windows ********************************/
/******************************************************************************/

class DockingEditorWorkspace : public PanelEditorWorkspaceBase
{
  public:
    DockingEditorWorkspace(EditorWorkspace* workspace);
    virtual ~DockingEditorWorkspace() override = default;

    virtual void on_imgui_update() override;

  private:
    void _menu_open_window(const std::string& panel_name);

    class EditorWorkspace* m_workspace = nullptr;
    int m_dock_node_flags = 0;
    int m_window_flags = 0;
};

class HierarchyEditorWorkspace : public PanelEditorWorkspaceBase
{
  public:
    HierarchyEditorWorkspace();
    virtual ~HierarchyEditorWorkspace() override = default;

    inline ecs::Entity get_selected_entity() const { return m_selected_entity; }

    virtual void on_imgui_update() override;

  private:
    void _draw_entity(ogl::Entity entity, ecs::Entity& entity_clicked, bool& opened_popup);
    void _create_shape(
        const std::string& new_entity_name, const std::string& mesh_file_path, ogl::Entity* parent
    );
    void _popup_menu(ogl::Entity* entity = nullptr);

    ecs::Entity m_selected_entity = ECS_ENTITY_DESTROYED;
    std::vector<ogl::Entity> m_deleted_entity = {};
};

class ConsoleEditorWorkspace : public PanelEditorWorkspaceBase
{
  public:
    ConsoleEditorWorkspace(ogl::Debug* debug);
    virtual ~ConsoleEditorWorkspace() override = default;

    virtual void on_imgui_update() override;

  private:
    glm::vec4 m_debug_colors[2] = {
        glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)};
    std::tuple<bool, std::string> m_filters[ogl::debug_type_count] = {};
    bool m_auto_scrolling = true;
    ogl::Debug* m_debug = nullptr;
    ImFont* m_font = nullptr;
};

class AssetsEditorWorkspace : public PanelEditorWorkspaceBase
{
  public:
    AssetsEditorWorkspace();
    virtual ~AssetsEditorWorkspace() override = default;

    virtual void on_imgui_update() override;
};

class ViewportEditorWorkspace : public PanelEditorWorkspaceBase
{
  public:
    ViewportEditorWorkspace(ogl::Framebuffer* framebuffer);
    virtual ~ViewportEditorWorkspace() override = default;

    virtual void on_imgui_update() override;

  private:
    void _camera_controller(ogl::CameraComponent* camera);
    void _no_scene(ogl::CameraComponent* camera, float window_width, float window_height);
    void _no_project();

    glm::ivec2 m_last_required_framebuffer_size = {};
    ogl::Framebuffer* m_framebuffer = nullptr;

    float m_camera_move_speed = 5.0f;
    glm::vec2 m_camera_sensitivity = {0.05f, 0.05f};
    float m_yaw = 0.0f;
    float m_pitch = 0.0f;
};

} // namespace oge

#endif
