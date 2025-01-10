#include "server.h"
#include <boost/asio.hpp>

const std::uint16_t port = 52137;
const std::uint16_t api_ver = 0;
const char* name = "Babs Server";
const std::uint16_t app_ver = 0;

int main() {
	try {
		Server server;
		server.run();
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}
