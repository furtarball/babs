/**
	@file server.cpp
	@brief Implementation of the server app.
*/

#include "packet.h"
#include "session.h"
#include <boost/asio.hpp>
#include <cstdint>
#include <ctime>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <unistd.h>

using boost::asio::ip::tcp;
const std::uint16_t api_ver = 0;
const char* name = "Babs Server";
const std::uint16_t app_ver = 0;

std::string name_version() {
	return std::string(name) + " v" + std::to_string(api_ver) + "." +
		   std::to_string(app_ver);
}

class Server {
	boost::asio::io_context& ctx;
	tcp::acceptor acceptor;
	/** Data structure for session objects.
		One user can maintain multiple sessions. */
	std::map<user_id_t, std::vector<Session::pointer>> sessions;
	void start_accept() {
		Session::pointer new_connection(new Session(ctx));
		acceptor.async_accept(
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
	Server(boost::asio::io_context& ctx)
		: ctx(ctx), acceptor(ctx, tcp::endpoint(tcp::v4(), 52137)) {
		start_accept();
	}
	void init(std::shared_ptr<Session> s) {
		HelloPacket p(api_ver, name_version());
		s->async_send(p.serialize());
	}
	/** @brief React to received packet.

		Part of argument "handle" passed by recv() to Session::async_receive.
		@return Whether session continues.
	*/
	bool handle_recv(std::string pkt, std::shared_ptr<Session> s) {
		std::cout << pkt << std::endl;
		Packet packet(pkt);
		switch (packet.get_type()) {
		case NONE: {
			for (auto i = sessions[s->uid].begin(); i < sessions[s->uid].end();
				 i++) {
				if ((*i)->id == s->id)
					sessions[s->uid].erase(i);
			}
			return false;
		}
		case LOGIN: {
			LoginPacket lp(pkt);
			std::cout << "Zalogowany użytkownik UID = " << lp.get_uid()
					  << " (id sesji: " << s->id << ")" << std::endl;
			s->uid = lp.get_uid();
			sessions[lp.get_uid()].push_back(s);
			break;
		}
		case MESSAGE: {
			MessagePacket mp(pkt);
			std::cout << "Wiadomość od " << mp.get_from_uid() << " do " << mp.get_to_uid()
					  << ": „" << mp.get_content() << "”" << std::endl;
			for (auto i : sessions[mp.get_to_uid()]) {
				std::cout << "wysyłanie…" << std::endl;
				i->async_send(mp.serialize());
			}
			break;
		}
		}
		return true;
	}
	void recv(std::shared_ptr<Session> s) {
		auto handle = [this, s](std::string p) {
			if (this->handle_recv(p, s))
				this->recv(s);
		};
		s->async_receive(handle);
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
