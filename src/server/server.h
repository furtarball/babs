/**
	@file server.h
	@brief Implementation of the server program.
*/

#ifndef SERVER_H
#define SERVER_H
#include "../common/packet.h"
#include "../common/session.h"
#include <boost/asio.hpp>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory>

using boost::asio::ip::tcp;
extern const std::uint16_t port;
extern const std::uint16_t api_ver;
extern const char* name;
extern const std::uint16_t app_ver;

class Server {
	boost::asio::io_context ctx;
	tcp::acceptor acceptor;
	/** @brief Initialize new connection. */
	void init(std::shared_ptr<Session> s);
	/** @brief Data structure for session objects.
	
		One user can maintain multiple sessions. */
	std::map<user_id_t, std::vector<std::shared_ptr<Session>>> sessions;
	/** @brief React to received packet.

		Part of argument "handle" passed by recv() to Session::async_receive.
		@return Whether session continues.
	*/
	bool handle_recv(std::string p, std::shared_ptr<Session> s);
	/** @brief Start accepting new connections asynchronously. */
	void start_accept();
	/** @brief Receive packets from a given connection asynchronously. */
	void recv(std::shared_ptr<Session> s);

	public:
	Server();
	/** @brief Run the io_context. */
	void run() { ctx.run(); }
};

#endif
