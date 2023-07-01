#include "gui/app.hpp"
#include "core/project.hpp"
#include "gui/assets.hpp"
#include "gui/console.hpp"
#include "gui/docking.hpp"
#include "gui/viewport.hpp"
#include "gui/editor.hpp"
#include "gui/hierarchy.hpp"
#include "gui/properties.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <kryos/renderer/renderer.hpp>
#include <portable-file-dialogs/portable-file-dialogs.h>

KEditorApp::KEditorApp()
{
    assert(
        pfd::settings::available() && "Portable File Dialogs are not available for this platform, "
                                      "therefore cannot run this program, sorry"
    );

    // Framebuffers
    KLPipeline* pipeline = get_application_layer<KLPipeline>();
    KLWindow* window = get_application_layer<KLWindow>();
    window->set_title("Kryos - No Project Selected");
    window->set_size(WindowResolution_Maximize);
    static_cast<KBasicRenderer*>(pipeline->push_renderer(new KBasicRenderer()))
        ->use_default_framebuffer(false);

    // Debug Logger
    KLDebug* debug = get_application_layer<KLDebug>();
    debug->set_automatically_clear_on_update(false);
    debug->set_serialize(true);

    // Editor Project Layer
    push_layer<KLProject>();

    // Editor Workspace Layer
    KLEditorWorkspace* workspace = push_layer<KLEditorWorkspace>();
    workspace->push_panels({
        new workspace::KDocking(workspace),
        new workspace::KConsole(debug),
        new workspace::KViewport(pipeline->create_framebuffer("editor viewport", 1280, 720)),
        new workspace::KHierarchy(),
        new workspace::KAssets(),
    });
    workspace->push_panel<workspace::KProperties>(
        static_cast<workspace::KHierarchy*>(workspace->get_panel("Hierarchy"))
    );

    // get_application_layer<ReflectionRegistry>()->log_all_detailed_types();
    // get_application_layer<ReflectionRegistry>()->log_all_templated_types();
    get_application_layer<KLReflectionRegistry>()->log_all_types();
};
