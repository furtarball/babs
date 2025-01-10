#include "session.h"

unsigned int Session::count = 0;

void Session::async_receive_body(
	std::shared_ptr<Parcel> parcel,
	std::shared_ptr<std::array<boost::asio::mutable_buffer, 2>> buffer,
	std::function<void(std::string)> handle) {
	boost::asio::async_read(socket, (*buffer)[1],
							[self = shared_from_this(), parcel, buffer,
							 handle](const boost::system::error_code& error,
									 std::size_t) {
								if (error) {
									throw boost::system::system_error(error);
								}
								handle(parcel->serialized);
							});
}

void Session::async_send(std::string s) {
	auto parcel = std::make_shared<const Parcel>(s.length(), s);
	auto buffer = std::make_shared<std::array<boost::asio::const_buffer, 2>>();
	(*buffer)[0] =
		boost::asio::buffer(&(parcel->length), sizeof(parcel->length));
	(*buffer)[1] = boost::asio::buffer(parcel->serialized, parcel->length);
	boost::asio::async_write(socket, *buffer,
							 [self = shared_from_this(), parcel,
							  buffer](const boost::system::error_code& error,
									  std::size_t) {
								 if (error) {
									 throw boost::system::system_error(error);
								 }
							 });
}

void Session::async_receive(std::function<void(std::string)> handle) {
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

void Session::send(std::string s) {
	boost::system::error_code error;
	const Parcel parcel(s.length(), s);
	std::array<boost::asio::const_buffer, 2> buffer;
	buffer[0] = boost::asio::buffer(&(parcel.length), sizeof(parcel.length));
	buffer[1] = boost::asio::buffer(parcel.serialized, parcel.length);
	boost::asio::write(socket, buffer, error);
	if (error)
		throw boost::system::system_error(error);
}

std::string Session::receive() {
	Parcel parcel;
	std::array<boost::asio::mutable_buffer, 2> buffer;
	buffer[0] = boost::asio::buffer(&(parcel.length), sizeof(parcel.length));
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
	}
	return parcel.serialized;
}
