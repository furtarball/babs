/**
	@file client.cpp
	@brief Implementation of the client app.
*/

#include "packet.h"
#include "session.h"
#include <array>
#include <boost/asio.hpp>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

using boost::asio::ip::tcp;
const std::uint16_t api_ver = 0;
const char* name = "Babs Client";
const std::uint16_t app_ver = 0;

class Client {
	public:
	boost::asio::io_context& ctx;
	Session::pointer session;
	std::thread worker;
	void connect(std::string server, user_id_t uid) {
		boost::system::error_code error;
		tcp::resolver resolver(ctx);
		tcp::resolver::results_type endpoints =
			resolver.resolve(server, "52137", error);
		if (error)
			throw boost::system::system_error(error);
		std::cout << "łączenie: " << server << std::endl;
		boost::asio::connect(session->get_socket(), endpoints, error);
		if (error)
			throw boost::system::system_error(error);
		std::cout << "czekam na hello pakiet" << std::endl;
		HelloPacket h(session->receive());
		std::cout << "odebrane" << std::endl;
		std::cout << h.get_name() << std::endl;
		std::cout << "wysyłanie pakietu login" << std::endl;
		session->send(LoginPacket(api_ver, uid).serialize());

		listen();
	}
	void handle_recv(std::string p) {
		std::cout << "Odebrany pakiet: " << p << std::endl;
	}
	void listen() {
		auto handle = [this](std::string p) {
			this->handle_recv(p);
			this->listen();
		};
		session->async_receive(handle);
	}
	Client(boost::asio::io_context& c, const std::string& s, user_id_t u)
		: ctx(c), session(new Session(c, u)) {
		connect(s, u);
		worker = std::thread([this]() { ctx.run(); });
	}
};

int main(int argc, char* argv[]) {
	boost::system::error_code error;
	boost::asio::io_context io_context;
	try {
		Client c(io_context, argv[1], std::stoi(std::string(argv[2])));
		for (;;) {
			std::string content;
			std::getline(std::cin, content);
			std::istringstream ss_content(content);
			boost::system::error_code error;
			user_id_t recipient;
			ss_content >> recipient;
			content = std::string(ss_content.str(),
								  static_cast<size_t>(ss_content.tellg()) + 1);
			MessagePacket mp(0, c.session->uid, recipient, content);
			std::cout << mp.serialize() << std::endl;
			c.session->async_send(mp.serialize());

			if (error) {
				if (error == boost::asio::error::eof)
					;
				else
					throw boost::system::system_error(error);
			}
		}
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}
