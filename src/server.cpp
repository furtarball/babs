#include <ctime>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <boost/asio.hpp>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <cstdint>
#include <boost/serialization/string.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <sstream>
#include "packet.h"

using boost::asio::ip::tcp;
const std::uint16_t api_ver = 0;
const char* name = "Babs Server";
const std::uint16_t app_ver = 0;

std::string name_version() {
  return std::string(name) + " v" + std::to_string(api_ver) + "." + std::to_string(app_ver);
}

class Session
  : public std::enable_shared_from_this<Session> {
  tcp::socket socket;
  user_id_t uid;
  Session(boost::asio::io_context& io_ctx) : socket(io_ctx) {}
public:
  typedef std::shared_ptr<Session> pointer;
  static pointer create(boost::asio::io_context& io_ctx) {
    return pointer(new Session(io_ctx));
  }

  tcp::socket& get_socket() {
    return socket;
  }

  void init() {
    HelloPacket p(api_ver, name_version());
    prepare(p);
    boost::asio::async_write(socket, p.buffer,
			     [self = shared_from_this(), pkt = std::make_shared<HelloPacket>(p)]
			     (const boost::system::error_code& error, std::size_t bytes_transferred) {
			       if(error) throw boost::system::system_error(error);
			     });
  }
  void recv() {
    boost::system::error_code error;
    LoginPacket l;
    receive(socket, l, error);
    if(error) throw boost::system::system_error(error);
    std::cout << l.uid << std::endl;
  }
  void respond() {
  }
  ~Session() {
    std::cout << "sesja zakończona" << std::endl;
  }
};

class Server {
  boost::asio::io_context& io_context_;
  tcp::acceptor acceptor_;
  std::vector<Session::pointer> sessions;
  void start_accept() {
    Session::pointer new_connection = Session::create(io_context_);
    //sessions.push_back(new_connection);
    acceptor_.async_accept(new_connection->get_socket(),
			   [this, new_connection](const boost::system::error_code& error) {
			     if(!error) {
			       new_connection->init();
			       new_connection->recv();
			     }
			     else throw boost::system::system_error(error);
			     start_accept();
			   });
  }
public:
  Server(boost::asio::io_context& io_context)
    : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), 52137)) {
    start_accept();
  }
};

int main() {
  try {
    boost::asio::io_context io_context;
    Server server(io_context);
    io_context.run();
  }
  catch(std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
