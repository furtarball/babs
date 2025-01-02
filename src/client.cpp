#include "packet.h"
#include "session.h"
#include <array>
#include <boost/asio.hpp>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>

using boost::asio::ip::tcp;

class Client {};

int main(int argc, char* argv[]) {
	boost::system::error_code error;
	try {

		boost::asio::io_context io_context;

		tcp::resolver resolver(io_context);
		std::cout << "łączenie z " << argv[1] << std::endl;
		tcp::resolver::results_type endpoints =
			resolver.resolve(argv[1], "52137", error);
		if (error)
			throw boost::system::system_error(error);

		tcp::socket socket(io_context);
		boost::asio::connect(socket, endpoints, error);
		if (error)
			throw boost::system::system_error(error);
		HelloPacket p;
		std::cout << "czekam na hello pakiet" << std::endl;
		receive(socket, p, error);
		if (error)
			throw boost::system::system_error(error);
		std::cout << p.name << std::endl;

		user_id_t uid = std::stoi(std::string(argv[2]));
		LoginPacket l(0, uid);
		prepare(l);
		send(socket, l, error);
		if (error)
			throw boost::system::system_error(error);

		for (;;) {
			user_id_t recipient;
			std::cin >> recipient;
			std::string content;
			std::getline(std::cin, content);
			MessagePacket m(0, uid, recipient, content);
			prepare(m);
			send(socket, m, error);

			if (error) {
				if (error == boost::asio::error::eof)
					break;
				else
					throw boost::system::system_error(error);
			}
		}
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}
