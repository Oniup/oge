#ifndef __OGE_CORE_APP_HPP__
#define __OGE_CORE_APP_HPP__

#include <ogl/ogl.hpp>

namespace oge {

	class App : public ogl::Application {
	public:
		App();
		virtual ~App() override = default;
	};

}

#endif