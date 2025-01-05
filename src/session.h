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

struct Parcel {
	std::uint32_t length;
	std::string serialized;
	Parcel() : length(0) {}
	Parcel(std::uint32_t length, std::string serialized)
		: length(length), serialized(serialized) {}
};

class Session : public std::enable_shared_from_this<Session> {
	void async_receive_body(
		std::shared_ptr<Parcel> parcel,
		std::shared_ptr<std::array<boost::asio::mutable_buffer, 2>> buffer,
		std::function<void(std::string)> handle) {
		boost::asio::async_read(
			socket, (*buffer)[1],
			[self = shared_from_this(), parcel, buffer,
			 handle](const boost::system::error_code& error,
					 std::size_t bytes_transferred) {
				if (error) {
					std::cerr << "async_receive_body" << std::endl;
					throw boost::system::system_error(error);
				}
				handle(parcel->serialized);
			});
	}

	public:
	user_id_t uid;
	static unsigned int count;
	unsigned int id;
	tcp::socket socket;
	typedef std::shared_ptr<Session> pointer;
	Session(boost::asio::io_context& ctx) : socket(ctx), uid(0), id(++count) {
		std::cout << "session" << std::endl;
	}
	Session(boost::asio::io_context& ctx, user_id_t uid)
		: socket(ctx), uid(uid), id(++count) {}
	tcp::socket& get_socket() { return socket; }
	~Session() { std::cout << "sesja zakończona" << std::endl; }

	/** @brief Send a packet asynchronously.
		@param s Packet to be sent, serialized.
	*/
	void async_send(std::string s) {
		auto parcel = std::make_shared<const Parcel>(s.length(), s);
		auto buffer =
			std::make_shared<std::array<boost::asio::const_buffer, 2>>();
		(*buffer)[0] =
			boost::asio::buffer(&(parcel->length), sizeof(parcel->length));
		(*buffer)[1] = boost::asio::buffer(parcel->serialized, parcel->length);
		boost::asio::async_write(
			socket, *buffer,
			[self = shared_from_this(), parcel,
			 buffer](const boost::system::error_code& error,
					 std::size_t bytes_transferred) {
				std::cout << "wysłane " << bytes_transferred << std::endl;
				if (error) {
					std::cerr << "async_send" << std::endl;
					throw boost::system::system_error(error);
				}
			});
	}

	/** @brief Receive a packet asynchronously.
		@param handle Function to call after successfully receiving a packet.
	*/
	void async_receive(std::function<void(std::string)> handle) {
		auto parcel = std::make_shared<Parcel>();
		auto buffer =
			std::make_shared<std::array<boost::asio::mutable_buffer, 2>>();
		(*buffer)[0] =
			boost::asio::buffer(&(parcel->length), sizeof(parcel->length));
		// start with the header
		boost::asio::async_read(
			socket, (*buffer)[0],
			[self = shared_from_this(), parcel, buffer,
			 handle](const boost::system::error_code& error,
					 std::size_t bytes_transferred) {
				if (error) {
					if (error == boost::asio::error::eof) {
						handle("0 0"); // type 0 packet if EOF
						return;
					} else {
						std::cerr << "async_receive" << std::endl;
						throw boost::system::system_error(error);
					}
				}
				if ((bytes_transferred > 0) && (parcel->length > 0)) {
					parcel->serialized = std::string(parcel->length + 1, 0);
					(*buffer)[1] =
						boost::asio::buffer(parcel->serialized, parcel->length);
					// continue with the body
					self->async_receive_body(parcel, buffer, handle);
				}
			});
	}

	/** @brief Send a packet synchronously.
		@param s Packet to be sent, serialized.
	*/
	void send(std::string s) {
		boost::system::error_code error;
		const Parcel parcel(s.length(), s);
		std::array<boost::asio::const_buffer, 2> buffer;
		buffer[0] =
			boost::asio::buffer(&(parcel.length), sizeof(parcel.length));
		buffer[1] = boost::asio::buffer(parcel.serialized, parcel.length);
		boost::asio::write(socket, buffer, error);
		if (error)
			throw boost::system::system_error(error);
	}

	/** @brief Receive a packet synchronously.
		@return Received packet, serialized. */
	std::string receive() {
		Parcel parcel;
		std::array<boost::asio::mutable_buffer, 2> buffer;
		buffer[0] =
			boost::asio::buffer(&(parcel.length), sizeof(parcel.length));
		boost::system::error_code error;
		size_t len = boost::asio::read(socket, buffer[0], error);
		if (error)
			throw boost::system::system_error(error);
		if ((len > 0) && (parcel.length > 0)) {
			parcel.serialized = std::string(parcel.length + 1, 0);
			buffer[1] = boost::asio::buffer(parcel.serialized, parcel.length);
			len = boost::asio::read(socket, buffer[1], error);
			if (error)
				throw boost::system::system_error(error);
			std::cout << parcel.serialized << std::endl;
		}
		return parcel.serialized;
	}
};

unsigned int Session::count = 0;
#endif
