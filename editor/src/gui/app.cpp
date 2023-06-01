#include "gui/app.hpp"
#include "core/project.hpp"
#include "gui/editor.hpp"
#include "gui/properties.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <ogl/renderer/renderer.hpp>
#include <portable-file-dialogs/portable-file-dialogs.h>

namespace oge {

App::App() {
    assert(
        pfd::settings::available() && "Portable File Dialogs are not available for this platform, "
                                      "therefore cannot run this program, sorry"
    );

    // Framebuffers
    ogl::Pipeline::get()->get_window()->set_title("Oniup's Game Editor - No Project Selected");
    ogl::Pipeline::get()->get_window()->set_size(ogl::WindowResolution_Maximize);
    static_cast<ogl::BasicRenderer*>(ogl::Pipeline::get()->push_renderer(new ogl::BasicRenderer()))
        ->use_default_framebuffer(false);

    // Debug Logger
    ogl::Debug* debug = get_layer<ogl::Debug>();
    debug->set_automatically_clear_on_update(false);
    debug->set_serialize(true);

    // Editor Project Layer
    push_layer<Project>();

    // Editor Workspace Layer
    EditorWorkspace* workspace = push_layer<EditorWorkspace>();
    workspace->push_panels({
        new DockingEditorWorkspace(workspace),
        new ConsoleEditorWorkspace(debug),
        new ViewportEditorWorkspace(
            ogl::Pipeline::get()->create_framebuffer("editor viewport", 1280, 720)
        ),
        new HierarchyEditorWorkspace(),
        new AssetsEditorWorkspace(),
    });
    workspace->push_panel<PropertiesEditorWorkspace>(
        static_cast<HierarchyEditorWorkspace*>(workspace->get_panel("Hierarchy"))
    );
}

} // namespace oge
