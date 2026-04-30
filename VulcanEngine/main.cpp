// dix
#include <Logger/Logger.hpp>
#include <FirstApp/FirstApp.hpp>

// std
#include <cstdlib>
#include <exception>

int main() {
	dix::FirstApp app {};

	try {
		app.run();
	}
	catch (const std::exception& e) {
		DixLogErr(e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}