#ifndef __OGE_CORE_EDITOR_HPP__
#define __OGE_CORE_EDITOR_HPP__

#include <ogl/ogl.hpp>

struct ImGuiIO;

namespace oge {

	class Panel {
	public:
		Panel(std::string_view name);
		virtual ~Panel() = default;

		inline const std::string& get_name() const { return m_name; }
		inline const bool& get_enabled() const  { return m_enabled; }
		inline bool& get_enabled() { return m_enabled; }

		virtual void on_imgui_update() {}

	protected:
		inline const ImGuiIO* get_io() const { return m_io; }
		inline ImGuiIO* get_io() { return m_io; }

	private:
		std::string m_name{};
		ImGuiIO* m_io{ nullptr };
		bool m_enabled{ false };
	};

	class Docking : public Panel {
	public:
		Docking(class EditorWorkspace* workspace);
		virtual ~Docking() override = default;

		virtual void on_imgui_update() override;

	private:
		class EditorWorkspace* m_workspace{ nullptr };
		int m_dock_node_flags{};
		int m_window_flags{};
	};

	class Hierarchy : public Panel {
	public:
		Hierarchy();
		virtual ~Hierarchy() override = default;

		virtual void on_imgui_update() override;
	};

	class Inspector : public Panel {
	public:
		Inspector();
		virtual ~Inspector() override = default;

		virtual void on_imgui_update() override;
	};

	struct ConsoleLog {
		ogl::DebugType type{};
		float time_recorded{};
		std::string message{};
	};

	class Console : public Panel {
	public:
		Console();
		virtual ~Console() override = default;

		virtual void on_imgui_update() override;

	private:
		void _load_logs_from_file();

		size_t m_log_file_size{};
		std::vector<ConsoleLog> m_logs{};
		glm::vec4 m_debug_colors[2]{ glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) };
		std::array<std::tuple<bool, std::string>, ogl::debug_type_count> m_filters{};
		bool m_auto_scrolling{ true };
	};

	class Assets : public Panel {
	public:
		Assets();
		virtual ~Assets() override = default;

		virtual void on_imgui_update() override;
	};

	class Viewport : public Panel {
	public:
		Viewport();
		virtual ~Viewport() override = default;

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

		Panel* get_panel(std::string_view name);
		const std::vector<Panel*>& get_all_panels() { return m_panels; }
		void remove_panel(std::string_view name);

		void push_panels(std::initializer_list<Panel*> panels);

		virtual void on_update() override;

	private:
		std::vector<Panel*> m_panels{};
	};

}

#endif