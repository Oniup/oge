#ifndef __OGE_CORE_EDITOR_HPP__
#define __OGE_CORE_EDITOR_HPP__

#include "ogl/utils/utils.hpp"
#include "ogl/core/application_layer.hpp"

namespace oge {

	class Panel {
	public:
		Panel(std::string_view name) : m_name(name) {}
		virtual ~Panel() = default;

		//void begin();
		//void end();

		virtual void on_update() {};
		virtual void on_imgui_render() {}

	private:
		int imgui_window_config{};
		std::string m_name{};
	};

	class EditorWorkspace : public ogl::ApplicationLayer {
	public:
		//EditorWorkspace();
		virtual ~EditorWorkspace() = default;

	private:
		std::vector<Panel> m_panels{};
	};

}

#endif