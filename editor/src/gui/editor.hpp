#ifndef __KRYOS_EDITOR_GUI_EDITOR_HPP__
#define __KRYOS_EDITOR_GUI_EDITOR_HPP__

#include <kryos/core/application_layer.hpp>
#include <yaml/yaml.hpp>

#define HIERARCHY_FILTER_NAME "@kryos_editor"

#define PREF_NAME_SIZE 32

struct ImGuiIO;
struct ImFont;

namespace workspace {

class KIWorkspace
{
  public:
    KIWorkspace(const std::string& name);
    virtual ~KIWorkspace() = default;

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

} // namespace workspace

class KLEditorWorkspace final : public KIApplicationLayer
{
  public:
    KLEditorWorkspace();
    virtual ~KLEditorWorkspace();

    template<typename _Panel, typename... _Args>
    _Panel* push_panel(_Args&&... args)
    {
        m_panels.push_back(new _Panel{std::forward<_Args>(args)...});
        return static_cast<_Panel*>(m_panels.back());
    }

    workspace::KIWorkspace* get_panel(const std::string& name);
    const std::vector<workspace::KIWorkspace*>& get_all_panels() { return m_panels; }
    void remove_panel(const std::string& name);

    void push_panels(std::initializer_list<workspace::KIWorkspace*> panels);

    virtual void on_update() override;

  private:
    void _load_colors(yaml::Node& color);
    void _load_styles(yaml::Node& styles);

  private:
    std::vector<workspace::KIWorkspace*> m_panels{};
};

#endif
