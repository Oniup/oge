#ifndef __OGE_CORE_EDITOR_HPP__
#define __OGE_CORE_EDITOR_HPP__

#include <ogl/ogl.hpp>

namespace oge {

	class Panel {
	public:
		Panel(std::string_view name) : m_name(name) {}
		virtual ~Panel() = default;

		inline const std::string& get_name() const { return m_name; }

		virtual void on_imgui_update() {}

	private:
		std::string m_name{};
	};

	class Docking : public Panel {
	public:
		Docking();
		virtual ~Docking() override = default;

		virtual void on_imgui_update() override;
	};

	class EditorWorkspace : public ogl::ApplicationLayer {
	public:
		EditorWorkspace();
		virtual ~EditorWorkspace();

		template <typename _Panel, typename ... _Args>
		_Panel* push_panel(_Args&& ... args) {
			m_panels.push_back(new _Panel{ std::forward<_Args>(args)... });
			return static_cast<_Panel*>(m_panels.back());
		}

		void remove_panel(std::string_view name);

		virtual void on_update() override;

	private:
		std::vector<Panel*> m_panels{};
	};

}

#endif