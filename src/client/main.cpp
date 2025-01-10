#include "client.h"

const char* port = "52137";
const std::uint16_t api_ver = 0;
const char* name = "Babs Client";
const std::uint16_t app_ver = 0;

int main(int argc, char* argv[]) {
	if(argc < 3) {
		std::cerr << "Usage: " << argv[0] << " [server] [user id]" << std::endl;
		return 1;
	}
	try {
		ClientGui app(argv[1], std::stoi(std::string(argv[2])));
		return app.run();
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}
