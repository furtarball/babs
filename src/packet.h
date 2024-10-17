#ifndef PACKET_H
#define PACKET_H
#include <sstream>
#include <boost/asio.hpp>
#include <boost/serialization/string.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <array>
#include <iostream>

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
  std::uint8_t type;
  Preamble() : version(0), type(NONE) {}
  Preamble(std::uint16_t v, PacketTypes t) : version(v), type(t) {}
  template<class Archive> void serialize(Archive& a, const unsigned int) {
    a & version;
    a & type;
  }
};

struct Packet {
  std::uint32_t length;
  std::string serialized;
  Preamble preamble;
  std::array<boost::asio::mutable_buffer, 2> buffer;
  Packet() : length(0) {}
  Packet(std::uint16_t v, PacketTypes t) : preamble(v, t) {}
  virtual ~Packet() {}
};

struct HelloPacket : public Packet {
  std::string name;
  HelloPacket() {}
  HelloPacket(std::uint16_t v, std::string n) : Packet(v, HELLO), name(n) {}
  template<class Archive> void serialize(Archive& a, const unsigned int) {
    a & preamble;
    a & name;
  }
};

struct LoginPacket : public Packet {
  user_id_t uid;
  LoginPacket() {}
  LoginPacket(std::uint16_t v, user_id_t u) : Packet(v, LOGIN), uid(u) {}
  template<class Archive> void serialize(Archive& a, const unsigned int) {
    a & preamble;
    a & uid;
  }
};

template<typename T> void prepare(T& pkt) {
  std::ostringstream ss;
  boost::archive::text_oarchive a(ss);
  a & pkt;
  pkt.serialized = ss.str();
  pkt.length = pkt.serialized.length();
  pkt.buffer[0] = boost::asio::buffer(&pkt.length, sizeof(pkt.length));
  pkt.buffer[1] = boost::asio::buffer(pkt.serialized, pkt.length);
}

template<typename T> void decode(T& pkt) {
  std::istringstream ss(pkt.serialized);
  boost::archive::text_iarchive a(ss);
  a & pkt; // throws exceptions on error!
}

template<typename T> void send(boost::asio::ip::tcp::socket& s, T& pkt, boost::system::error_code& error) {
  prepare(pkt);
  boost::asio::write(s, pkt.buffer, error);
}

template<typename T> void send_async(boost::asio::ip::tcp::socket& s, T& pkt) {}

template<typename T> void receive(boost::asio::ip::tcp::socket& s, T& pkt, boost::system::error_code& error) {
  pkt.buffer[0] = boost::asio::buffer(&pkt.length, sizeof(pkt.length));
  size_t len = boost::asio::read(s, pkt.buffer[0], error);
  if((len > 0) && (pkt.length > 0)) {
    pkt.serialized = std::string(pkt.length + 1, 0);
    pkt.buffer[1] = boost::asio::buffer(pkt.serialized, pkt.length);
    len = boost::asio::read(s, pkt.buffer[1], error);
    decode(pkt);
  }
}

template<typename T> void receive_async(boost::asio::ip::tcp::socket& s, T& pkt) {}
#endif
