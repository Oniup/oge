#include "gui/app.hpp"
#include "core/project.hpp"
#include "gui/editor.hpp"
#include "gui/properties.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <kryos/renderer/renderer.hpp>
#include <portable-file-dialogs/portable-file-dialogs.h>

App::App()
{
    assert(
        pfd::settings::available() && "Portable File Dialogs are not available for this platform, "
                                      "therefore cannot run this program, sorry"
    );

    // Framebuffers
    kryos::Pipeline* pipeline = get_application_layer<kryos::Pipeline>();
    kryos::Window* window = get_application_layer<kryos::Window>();
    window->set_title("Kryos - No Project Selected");
    window->set_size(kryos::WindowResolution_Maximize);
    static_cast<kryos::BasicRenderer*>(pipeline->push_renderer(new kryos::BasicRenderer()))
        ->use_default_framebuffer(false);

    // Debug Logger
    kryos::Debug* debug = get_application_layer<kryos::Debug>();
    debug->set_automatically_clear_on_update(false);
    debug->set_serialize(true);

    // Editor Project Layer
    push_layer<Project>();

    // Editor Workspace Layer
    EditorWorkspace* workspace = push_layer<EditorWorkspace>();
    workspace->push_panels({
        new DockingEditorWorkspace(workspace),
        new ConsoleEditorWorkspace(debug),
        new ViewportEditorWorkspace(pipeline->create_framebuffer("editor viewport", 1280, 720)),
        new HierarchyEditorWorkspace(),
        new AssetsEditorWorkspace(),
    });
    workspace->push_panel<PropertiesEditorWorkspace>(
        static_cast<HierarchyEditorWorkspace*>(workspace->get_panel("Hierarchy"))
    );

    // get_application_layer<kryos::ReflectionRegistry>()->log_all_detailed_types();
    get_application_layer<kryos::ReflectionRegistry>()->log_all_templated_types();

    std::cout << ecs::type_descriptor::get_wrapped_type_name<double>();
};
