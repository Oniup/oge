#include "core/editor.hpp"

#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>

#include <GLFW/glfw3.h>

namespace oge {

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
		if (ImGui::Begin(get_name().c_str(), &get_enabled())) {
			ImGui::Text("This is a test");
			ImGui::Text("This is a test");
			ImGui::Text("This is a test");
			ImGui::Text("This is a test");
			ImGui::Text("This is a test");
			ImGui::Text("This is a test");

			ImGui::End();
		}
	}

	Inspector::Inspector() : Panel("Inspector") {
	}

	void Inspector::on_imgui_update() {
		if (ImGui::Begin(get_name().c_str(), &get_enabled())) {
			ImGui::End();
		}
	}

	Console::Console() : Panel("Console") {
	}

	void Console::on_imgui_update() {
		if (ImGui::Begin(get_name().c_str(), &get_enabled())) {
			ImGui::End();
		}
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