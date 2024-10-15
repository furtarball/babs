#include <array>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/serialization/string.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <sstream>
#include <cstdint>
#include "packet.h"

using boost::asio::ip::tcp;

int main(int argc, char* argv[]) {
  try {

    boost::asio::io_context io_context;

    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints =
      resolver.resolve("10.1.0.2", "52137");

    tcp::socket socket(io_context);
    boost::asio::connect(socket, endpoints);

    for (;;) {
      boost::system::error_code error;

      HelloPacket p;
      receive(socket, p);
      std::cout << p.name << std::endl;

      if (error == boost::asio::error::eof) {
        break; // Connection closed cleanly by peer.
        }
      else if (error)
        throw boost::system::system_error(error); // Some other error.

      LoginPacket l(0, 2137);
      prepare(l);
      send(socket, l);

      //std::istringstream ss(buf.data());
      //boost::archive::text_iarchive a(ss);
      //HelloPacket p;
      //a & p;
      //std::cout << p.name << std::endl;
      int len;
      std::cin >> len;
      //len = write(socket, boost::asio::buffer("login.Jakub"), error);
      if(error) {
	std::cerr << error.message() << std::endl;
	throw boost::system::system_error(error);
      }
      //std::cout << "wysłano " << len << " bajtów" << std::endl;
    }
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
