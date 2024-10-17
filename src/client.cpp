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
  boost::system::error_code error;
  try {

    boost::asio::io_context io_context;

    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints =
      resolver.resolve("10.1.0.2", "52137", error);
    if(error) throw boost::system::system_error(error);

    tcp::socket socket(io_context);
    boost::asio::connect(socket, endpoints, error);
    if(error) throw boost::system::system_error(error);

    for(;;) {

      HelloPacket p;
      receive(socket, p, error);
      if(error == boost::asio::error::eof) break;
      else if (error) throw boost::system::system_error(error);
      std::cout << p.name << std::endl;
      
      LoginPacket l(0, 2137);
      prepare(l);
      send(socket, l, error);

      if(error) throw boost::system::system_error(error);
    }
  }
  catch(std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
