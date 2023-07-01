#include "gui/app.hpp"

int main(int argc, char** argv)
{
    KEditorApp* app = new KEditorApp();
    app->run();
    delete app;
}
