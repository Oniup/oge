#include "core/app.hpp"

int main(int argc, char** argv) {
	oge::App* app = new oge::App();
	app->run();
	delete app;
}