/**
	@file session.h
	@brief Definition of class Session.
*/

#ifndef SESSION_H
#define SESSION_H
#include "packet.h"
#include <boost/asio.hpp>
#include <functional>
#include <memory>

using boost::asio::ip::tcp;

/** @brief Structure for facilitating Boost i/o operations. */
struct Parcel {
	std::uint32_t length;
	std::string serialized;
	Parcel() : length(0) {}
	Parcel(std::uint32_t length, std::string serialized)
		: length(length), serialized(serialized) {}
};

/** @brief Class for maintaining user sessions and performing i/o operations. */
class Session : public std::enable_shared_from_this<Session> {
	user_id_t uid;
	static unsigned int count;
	unsigned int id;
	void async_receive_body(
		std::shared_ptr<Parcel> parcel,
		std::shared_ptr<std::array<boost::asio::mutable_buffer, 2>> buffer,
		std::function<void(std::string)> handle);
	tcp::socket socket;

	public:
	unsigned int get_id() { return id; }
	void set_uid(user_id_t u) { uid = u; }
	user_id_t get_uid() { return uid; }
	tcp::socket& get_socket() { return socket; }
	Session(boost::asio::io_context& ctx) : uid(0), id(++count), socket(ctx) {}
	Session(boost::asio::io_context& ctx, user_id_t uid)
		: uid(uid), id(++count), socket(ctx) {}

	/** @brief Send a packet asynchronously.
		@param s Packet to be sent, serialized.
	*/
	void async_send(std::string s);

	/** @brief Receive a packet asynchronously.
		@param handle Function to call after successfully receiving a packet.
		Must accept packet in serialized form.
	*/
	void async_receive(std::function<void(std::string)> handle);

	/** @brief Send a packet synchronously.
		@param s Packet to be sent, serialized.
	*/
	void send(std::string s);

	/** @brief Receive a packet synchronously.
		@return Received packet, serialized. */
	std::string receive();
};
#endif
