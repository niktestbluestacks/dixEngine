#include <FirstApp/FirstApp.hpp>

#include <cstdlib>
#include <iostream>
#include <stdexcept>

auto main(void) -> signed {
	dix::FirstApp app {};

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}