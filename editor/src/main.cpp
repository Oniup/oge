#include "gui/app.hpp"

int main(int argc, char** argv)
{
    App* app = new App();
    app->run();
    delete app;
}
