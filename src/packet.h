#ifndef PACKET_H
#define PACKET_H
#include <sstream>
#include <boost/asio.hpp>
#include <boost/serialization/string.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <array>
#include <iostream>
#include <sstream>

using user_id_t = std::uint16_t;

enum PacketTypes {
  NONE = 0,
  HELLO,
  STATE,
  LOGIN,
  MESSAGE,
  TYPES_TOTAL
};

struct Preamble {
  std::uint16_t version;
  std::uint16_t type;
  Preamble() : version(0), type(NONE) {}
  Preamble(std::uint16_t v, PacketTypes t) : version(v), type(t) {}
  Preamble(std::string& s) {
    std::istringstream ss(s);
    ss >> version;
    ss >> type;
    s = std::string(ss.str(), ss.tellg() + 1);
  }
  std::string serialize() {
    std::ostringstream ss;
    ss << version << " " << static_cast<int>(type);
    return ss.str();
  }
};

struct Packet {
protected:
  Packet(std::uint16_t v, PacketTypes t) : preamble(v, t) {}
public:
  std::uint32_t length;
  std::string serialized;
  Preamble preamble;
  std::array<boost::asio::mutable_buffer, 2> buffer;
  Packet() : length(0) {}
  Packet(std::string& s) : length(s.length()), serialized(s), preamble(s) {}
  virtual std::string serialize() {
    std::ostringstream ss;
    ss << preamble.serialize();
    return ss.str();
  }
  virtual ~Packet() = default;
};

struct HelloPacket : public Packet {
  std::string name;
  HelloPacket() {}
  HelloPacket(std::uint16_t v, std::string n) : Packet(v, HELLO), name(n) {}
  HelloPacket(std::string s) : Packet(s) {
    size_t quote = s.find('"', 0);
    if(quote != std::string::npos) {
      size_t unquote = s.find('"', quote + 1);
      if(unquote != std::string::npos)
	name = std::string(s, 1, unquote - 1);
    }
  }
  std::string serialize() {
    std::ostringstream ss;
    ss << preamble.serialize() << " \"" << name << "\"";
    return ss.str();
  }
};

struct LoginPacket : public Packet {
  user_id_t uid;
  LoginPacket() {}
  LoginPacket(std::uint16_t v, user_id_t u) : Packet(v, LOGIN), uid(u) {}
  LoginPacket(std::string s) : Packet(s) {
    std::istringstream ss(s);
    ss >> uid;
  }
  std::string serialize() {
    std::ostringstream ss;
    ss << preamble.serialize() << " " << uid;
    return ss.str();
  }
};

struct MessagePacket : public Packet {
  user_id_t from_uid;
  user_id_t to_uid;
  std::string content;
  MessagePacket() {}
  MessagePacket(std::uint16_t v, user_id_t s, user_id_t d, std::string c) : Packet(v, MESSAGE), from_uid(s), to_uid(d), content(c) {}
  MessagePacket(std::string s) : Packet(s) {
    std::istringstream ss(s);
    ss >> from_uid;
    ss >> to_uid;
    size_t quote = s.find('"', 0);
    if(quote != std::string::npos) {
      size_t unquote = s.find('"', quote + 1);
      if(unquote != std::string::npos)
	content = std::string(s, quote + 1, unquote - (quote + 1));
    }
  }
  std::string serialize() {
    std::ostringstream ss;
    ss << preamble.serialize() << " " << from_uid << " " << to_uid << " \"" << content << "\"";
    return ss.str();
  }
};

void prepare(Packet& pkt) {
  pkt.serialized = pkt.serialize();
  pkt.length = pkt.serialized.length();
  pkt.buffer[0] = boost::asio::buffer(&pkt.length, sizeof(pkt.length));
  pkt.buffer[1] = boost::asio::buffer(pkt.serialized, pkt.length);
}

template<typename T> void send(boost::asio::ip::tcp::socket& s, T& pkt, boost::system::error_code& error) {
  prepare(pkt);
  boost::asio::write(s, pkt.buffer, error);
}

template<typename T> void receive(boost::asio::ip::tcp::socket& s, T& pkt, boost::system::error_code& error) {
  pkt.buffer[0] = boost::asio::buffer(&pkt.length, sizeof(pkt.length));
  size_t len = boost::asio::read(s, pkt.buffer[0], error);
  if((len > 0) && (pkt.length > 0)) {
    pkt.serialized = std::string(pkt.length + 1, 0);
    pkt.buffer[1] = boost::asio::buffer(pkt.serialized, pkt.length);
    len = boost::asio::read(s, pkt.buffer[1], error);
    //pkt.decode();
  }
}
#endif
