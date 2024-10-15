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
			     [self = shared_from_this(), pkt = std::make_shared<HelloPacket>(p)](const boost::system::error_code& error,
							 std::size_t bytes_transferred) {
			       if(error)
				 std::cerr << error.message() << std::endl;
			     });
    recv();
  }
  void recv() {
    LoginPacket l;
    receive(socket, l);
    std::cout << l.uid << std::endl;
    // std::istringstream ss;
    // boost::archive::text_iarchive a(ss);
    // LoginPacket p();
    // a & p;
    // boost::asio::async_read(socket, boost::asio::buffer(buf),
    // 			    [this, self = shared_from_this()](const boost::system::error_code& error,
    // 							std::size_t bytes_transferred) {
    // 			      std::cout << "odebrano " << bytes_transferred << " bajtów: ";
    // 			      std::cout.write(buf.data(), bytes_transferred);
    // 			      std::cout << std::endl;
    // 			      if(error)
    // 				std::cerr << error.message() << std::endl;
    // 			      respond();
    // 			    });
  }
  void respond() {
    // std::string command = std::string(buf.data()).substr(0, 5);
    // std::string content = std::string(buf.data()).substr(6, 5);
    // std::cout << "Login user " << content << std::endl;
    // uid = content;
    // recv();
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
			     }
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
