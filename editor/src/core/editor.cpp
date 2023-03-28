#include "core/editor.hpp"

#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>

#include <GLFW/glfw3.h>

namespace oge {

	enum ConsoleFileReadingStage {
		ConsoleFileReadingStage_Type,
		ConsoleFileReadingStage_TimeRecorded,
		ConsoleFileReadingStage_Message
	};

	Panel::Panel(std::string_view name) : m_name(name) {
		m_io = &ImGui::GetIO(); static_cast<void>(*m_io);
	}

	Docking::Docking(EditorWorkspace* workspace) : Panel("Docking") {
		m_dock_node_flags = ImGuiDockNodeFlags_None;

		m_window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | 
						 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		m_workspace = workspace;
		get_enabled() = true;
	}

	void Docking::on_imgui_update() {
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		if (ImGui::Begin(get_name().c_str(), &get_enabled(), m_window_flags)) {
			ImGui::PopStyleVar(3);

			ImGuiID dock_space_id = ImGui::GetID("DockSpace");
			ImGui::DockSpace(dock_space_id, ImVec2(0.0f, 0.0f), m_dock_node_flags);

			if (ImGui::BeginMenuBar()) {
				if (ImGui::BeginMenu("File")) {

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Preferences")) {
					if (ImGui::BeginMenu("Open Window")) {
						const std::vector<Panel*>& panels = m_workspace->get_all_panels();

						for (Panel* panel : panels) {
							if (panel->get_name() != get_name()) {
								ImGui::MenuItem(panel->get_name().c_str(), nullptr, &panel->get_enabled());
							}
						}

						ImGui::EndMenu();
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}

			ImGui::End();
		}
	}

	Hierarchy::Hierarchy() : Panel("Hierarchy") {
	}

	void Hierarchy::on_imgui_update() {
		ImGui::Begin(get_name().c_str(), &get_enabled());
		ImGui::Text("This is a test");
		ImGui::Text("This is a test");
		ImGui::Text("This is a test");
		ImGui::Text("This is a test");
		ImGui::Text("This is a test");
		ImGui::Text("This is a test");

		ImGui::End();
	}

	Inspector::Inspector() : Panel("Inspector") {
	}

	void Inspector::on_imgui_update() {
	}

	Console::Console() : Panel("Console") {
		std::FILE* file = std::fopen(OGL_DEBUG_FILE_NAME, "r");
		if (file == nullptr) {
			m_log_file_size = 0;
			return;
		}

		char buffer[1024];
		while (std::fgets(buffer, 1024, file)) {
			m_log_file_size++;
		}
		std::fclose(file);

		m_filters = {
			std::make_tuple(true, "Messages"), 
			std::make_tuple(true, "Warnings"), 
			std::make_tuple(true, "Errors"), 
            std::make_tuple(true, "Inits"), 
			std::make_tuple(true, "Terminates"), 
		};

		get_enabled() = true;
	}

	void Console::on_imgui_update() {
		_load_logs_from_file();

		ImGui::Begin(get_name().c_str(), &get_enabled(), ImGuiWindowFlags_MenuBar);

		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("Options")) {
				if (ImGui::BeginMenu("Filter")) {
					for (std::tuple<bool, std::string>& filter : m_filters) {
						ImGui::MenuItem(std::get<std::string>(filter).c_str(), nullptr, &std::get<bool>(filter));
					}
					ImGui::EndMenu();
				}
				ImGui::MenuItem("Auto-Scrolling", nullptr, &m_auto_scrolling);
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Clear")) {
				m_logs.clear();
			}
			if (ImGui::MenuItem("Add Test Log")) {
				m_logs.push_back(ConsoleLog { ogl::DebugType_Message, ogl::Time::get_elapsed(), "Test Message" });
			}
			ImGui::EndMenuBar();
		}

		for (ConsoleLog& log : m_logs) {
			if (std::get<bool>(m_filters[static_cast<size_t>(log.type)])) {
				switch (log.type) {
				case ogl::DebugType_Warning:
					ImGui::TextColored(ImVec4(m_debug_colors[0].r, m_debug_colors[0].g, m_debug_colors[0].b, m_debug_colors[0].a), "[Warning (%f)]: %s", log.time_recorded, log.message.c_str());
					break;
				case ogl::DebugType_Error:
					ImGui::TextColored(ImVec4(m_debug_colors[1].r, m_debug_colors[1].g, m_debug_colors[1].b, m_debug_colors[1].a), "[Error (%f)]: %s", log.time_recorded, log.message.c_str());
					break;
				case ogl::DebugType_Message:
					ImGui::Text("[Message (%f)]: %s", log.time_recorded, log.message.c_str());
					break;
				}
			}
		}

		if (m_auto_scrolling && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
			ImGui::SetScrollHereY(1.0f);
		}

		ImGui::End();
	}

	void Console::_load_logs_from_file() {
		std::FILE* file = std::fopen(OGL_DEBUG_FILE_NAME, "r");
		if (file == nullptr) {
			return;
		}

		char line[1024];
		size_t line_position = 0;
		while (std::fgets(line, 1024, file)) {
			if (line_position >= m_log_file_size) {
				if (std::strlen(line) > 1) {
					size_t line_length = std::strlen(line);
					ConsoleFileReadingStage stage = ConsoleFileReadingStage_Type;
					ConsoleLog log{};

					size_t j = 0;
					char current[1024];
					for (size_t i = 0; i < line_length; i++) {
						switch (stage) {
						case ConsoleFileReadingStage_Type:
							if (line[i] == ',') {
								current[j] = '\0';
								log.type = static_cast<ogl::DebugType>(std::stoi(current));

								j = 0;
								stage = ConsoleFileReadingStage_TimeRecorded;
								continue;
							}
							break;
						case ConsoleFileReadingStage_TimeRecorded:
							if (line[i] == ',') {
								current[j] = '\0';
								log.time_recorded = std::stof(current);

								j = 0;
								stage = ConsoleFileReadingStage_Message;
								continue;
							}
							break;
						}

						current[j] = line[i];
						j++;
					}

					current[j] = '\0';
					log.message = current;
					m_logs.push_back(log);
				}
			}
			line_position++;
		}
		
		m_log_file_size = line_position + 1;
		std::fclose(file);
	}

	Assets::Assets() : Panel("Assets") {

	}

	void Assets::on_imgui_update() {
		if (ImGui::Begin(get_name().c_str(), &get_enabled())) {
			ImGui::End();
		}
	}

	Viewport::Viewport() : Panel("Viewport") {

	}

	void Viewport::on_imgui_update() {
		if (ImGui::Begin(get_name().c_str(), &get_enabled())) {
			ImGui::End();
		}
	}

	EditorWorkspace::EditorWorkspace() {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;

		ImGui::StyleColorsDark();
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		    style.WindowRounding = 0.0f;
		    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		ImGui_ImplGlfw_InitForOpenGL(ogl::Pipeline::get()->get_window()->get_internal(), true);
		ImGui_ImplOpenGL3_Init("#version 450");
	}

	EditorWorkspace::~EditorWorkspace() {
		for (Panel* panel : m_panels) {
			delete panel;
		}

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	Panel* EditorWorkspace::get_panel(std::string_view name) {
		for (Panel* panel : m_panels) {
			if (panel->get_name() == name) {
				return panel;
			}
		}

		return nullptr;
	}

	void EditorWorkspace::remove_panel(std::string_view name) {
		for (size_t i = 0; i < m_panels.size(); i++) {
			if (m_panels[i]->get_name() == name) {
				delete m_panels[i];
				m_panels.erase(m_panels.begin() + i);
				return;
			}
		}
	}

	void EditorWorkspace::push_panels(std::initializer_list<Panel*> panels) {
		for (Panel* panel : panels) {
			m_panels.push_back(panel);
		}
	}

	void EditorWorkspace::on_update() {
		static ImGuiIO& io = ImGui::GetIO(); (void)io;

		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		for (Panel* panel : m_panels) {
			if (panel->get_enabled()) {
				panel->on_imgui_update();
			}
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			GLFWwindow* backup_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_context);
		}
	}

}