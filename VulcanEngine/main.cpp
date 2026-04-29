// dix
#include <FirstApp/FirstApp.hpp>

// std
#include <cstdlib>
#include <iostream>
#include <exception>

int main() {
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