#ifndef SESSION_H
#define SESSION_H
#include <memory>
#include <boost/asio.hpp>
#include <functional>
#include "packet.h"

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
  Session(boost::asio::io_context& io_ctx) : socket(io_ctx) {}
  void async_receive_body(std::shared_ptr<Packet> pkt, std::function<void(std::shared_ptr<Packet>)> handle) {
    boost::asio::async_read(socket, pkt->buffer[1],
			    [self = shared_from_this(), pkt, handle]
			    (const boost::system::error_code& error, std::size_t bytes_transferred) {
			      if(error) throw boost::system::system_error(error);
			      handle(pkt);
			    });
  }
public:
  user_id_t uid;
  tcp::socket socket;
  typedef std::shared_ptr<Session> pointer;
  static pointer create(boost::asio::io_context& io_ctx) {
    return pointer(new Session(io_ctx));
  }

  tcp::socket& get_socket() {
    return socket;
  }
  ~Session() {
    std::cout << "sesja zakończona" << std::endl;
  }
  
  void async_send(Packet& pkt) {
    prepare(pkt);
    boost::asio::async_write(socket, pkt.buffer,
			     [self = shared_from_this(), pkt] // no big deal if we slice the object
			     (const boost::system::error_code& error, std::size_t bytes_transferred) {
			       if(error) throw boost::system::system_error(error);
			     });
  }
  void async_receive(std::function<void(std::shared_ptr<Packet>)> handle) {
    auto pkt = std::make_shared<Packet>();
    pkt->buffer[0] = boost::asio::buffer(&(pkt->length), sizeof(pkt->length));
    // start with the header
    boost::asio::async_read(socket, pkt->buffer[0],
			    [self = shared_from_this(), pkt, handle]
			    (const boost::system::error_code& error, std::size_t bytes_transferred) {
			      if(error) throw boost::system::system_error(error);
			      if((bytes_transferred > 0) && (pkt->length > 0)) {
				pkt->serialized = std::string(pkt->length + 1, 0);
				pkt->buffer[1] = boost::asio::buffer(pkt->serialized, pkt->length);
				// continue with the body
				self->async_receive_body(pkt, handle);
			      }
			    });
  }
};
#endif
