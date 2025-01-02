#include <array>
#include <iostream>
#include <boost/asio.hpp>
#include <sstream>
#include <cstdint>
#include "packet.h"
#include "session.h"
#include <string>

using boost::asio::ip::tcp;

class Client {
  
};

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
      std::cout << "czekam na hello pakiet" << std::endl;
      receive(socket, p, error);
      if(error == boost::asio::error::eof) break;
      else if (error) throw boost::system::system_error(error);
      std::cout << p.name << std::endl;

      user_id_t uid = std::stoi(std::string(argv[1]));
      LoginPacket l(0, uid);
      prepare(l);
      send(socket, l, error);

      int n;
      std::cin >> n;
      MessagePacket m(0, 2137, 666, "okrutniku");
      prepare(m);
      send(socket, m, error);

      if(error) throw boost::system::system_error(error);
    }
  }
  catch(std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
