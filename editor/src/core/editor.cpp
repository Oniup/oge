#include "core/editor.hpp"

#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>

#include <GLFW/glfw3.h>

namespace oge {

	Docking::Docking() : Panel("Docking") {}

	void Docking::on_imgui_update() {
		ImGui::Begin("Test Window");

		ImGui::Text("This is a test window");

		ImGui::End();
	}

	EditorWorkspace::EditorWorkspace() : ogl::ApplicationLayer("editor workspace") {
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

	void EditorWorkspace::remove_panel(std::string_view name) {
		for (size_t i = 0; i < m_panels.size(); i++) {
			if (m_panels[i]->get_name() == name) {
				delete m_panels[i];
				m_panels.erase(m_panels.begin() + i);
				return;
			}
		}
	}

	void EditorWorkspace::on_update() {
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		for (Panel* panel : m_panels) {
			panel->on_imgui_update();
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