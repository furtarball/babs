#include "packet.h"
#include "session.h"
#include <boost/asio.hpp>
#include <cstdint>
#include <ctime>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

using boost::asio::ip::tcp;
const std::uint16_t api_ver = 0;
const char* name = "Babs Server";
const std::uint16_t app_ver = 0;

std::string name_version() {
	return std::string(name) + " v" + std::to_string(api_ver) + "." +
		   std::to_string(app_ver);
}

class Server {
	boost::asio::io_context& io_context_;
	tcp::acceptor acceptor_;
	std::vector<Session::pointer> sessions;
	void start_accept() {
		Session::pointer new_connection = Session::create(io_context_);
		sessions.push_back(new_connection);
		acceptor_.async_accept(
			new_connection->get_socket(),
			[this, new_connection](const boost::system::error_code& error) {
				if (!error) {
					init(new_connection);
					recv(new_connection);
				} else
					throw boost::system::system_error(error);
				start_accept();
			});
	}

	public:
	Server(boost::asio::io_context& io_context)
		: io_context_(io_context),
		  acceptor_(io_context, tcp::endpoint(tcp::v4(), 52137)) {
		start_accept();
	}
	void init(std::shared_ptr<Session> s) {
		HelloPacket p(api_ver, name_version());
		s->async_send(p);
	}
	void handle_recv(std::shared_ptr<Packet> pkt, std::shared_ptr<Session> s) {
		*pkt = Packet(pkt->serialized);
		switch (pkt->preamble.type) {
		case LOGIN: {
			LoginPacket lp(pkt->serialized);
			std::cout << "Zalogowany użytkownik UID = " << lp.uid << std::endl;
			s->uid = lp.uid;
			break;
		}
		case MESSAGE: {
			MessagePacket mp(pkt->serialized);
			std::cout << "Wiadomość od " << mp.from_uid << " do " << mp.to_uid
					  << ": „" << mp.content << "”" << std::endl;
			for (auto&& i : sessions) {
				if (i->uid == mp.to_uid) {
					std::cout << "wysyłanie…" << std::endl;
					i->async_send(mp);
				}
			}
			break;
		}
		}
	}
	void recv(std::shared_ptr<Session> s) {
		s->async_receive([this, s](std::shared_ptr<Packet> p) {
			this->handle_recv(p, s);
			this->recv(s);
		});
	}
};

int main() {
	try {
		boost::asio::io_context io_context;
		Server server(io_context);
		io_context.run();
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}
