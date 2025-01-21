#include "client.h"

const char* port = "52137";
const std::uint16_t api_ver = 0;
const char* name = "Babs Client";
const std::uint16_t app_ver = 0;

int main() {
	try {
		ClientGui app;
		return app.run();
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}
