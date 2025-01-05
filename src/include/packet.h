/**
	@file packet.h
	@brief Definitions of packet structs.
*/

#ifndef PACKET_H
#define PACKET_H
#include <array>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/asio.hpp>
#include <boost/serialization/string.hpp>
#include <iostream>
#include <sstream>

using user_id_t = std::uint16_t;
enum PacketTypes { NONE = 0, HELLO, STATE, LOGIN, MESSAGE, TYPES_TOTAL };

/** @brief Parent packet class. Only contains the preamble. */
class Packet {
	void deserialize(std::string& s) {
		std::istringstream ss(s);
		ss >> version;
		ss >> type;
		s = std::string(ss.str(), static_cast<size_t>(ss.tellg()) + 1);
	}
	std::uint16_t version;
	std::uint16_t type;

	protected:
	/** @brief Initializes object removing preamble from input string. */
	void begin_deserialization(std::string& s) { deserialize(s); }
	Packet() : version(0), type(NONE) {}

	public:
	Packet(std::uint16_t version, PacketTypes type)
		: version(version), type(type) {}
	void set_version(std::uint16_t v) { version = v; }
	std::uint16_t get_version() { return version; }
	void set_type(std::uint16_t t) { type = t; }
	std::uint16_t get_type() { return type; }
	/** @brief Deserializing constructor. */
	Packet(std::string s) { deserialize(s); }
	std::string serialize() {
		std::ostringstream ss;
		ss << version << " " << static_cast<int>(type);
		return ss.str();
	}
	virtual ~Packet() = default;
};

class HelloPacket : public Packet {
	std::string name;
	void deserialize(std::string& s) {
		size_t quote = s.find('"', 0);
		if (quote != std::string::npos) {
			size_t unquote = s.find('"', quote + 1);
			if (unquote != std::string::npos)
				name = std::string(s, 1, unquote - 1);
		}
	}

	public:
	void set_name(const std::string& n) { name = n; }
	std::string get_name() const { return name; }
	HelloPacket(std::uint16_t version, std::string name)
		: Packet(version, HELLO), name(name) {}
	/** @brief Deserializing constructor. */
	HelloPacket(std::string s) {
		Packet::begin_deserialization(s);
		deserialize(s);
	}
	std::string serialize() {
		std::ostringstream ss;
		ss << Packet::serialize() << " \"" << name << "\"";
		return ss.str();
	}
};

class LoginPacket : public Packet {
	user_id_t uid;
	void deserialize(std::string& s) {
		std::istringstream ss(s);
		ss >> uid;
	}

	public:
	void set_uid(user_id_t id) { uid = id; }
	user_id_t get_uid() const { return uid; }
	LoginPacket(std::uint16_t version, user_id_t uid)
		: Packet(version, LOGIN), uid(uid) {}
	/** @brief Deserializing constructor. */
	LoginPacket(std::string s) {
		Packet::begin_deserialization(s);
		deserialize(s);
	}
	std::string serialize() {
		std::ostringstream ss;
		ss << Packet::serialize() << " " << uid;
		return ss.str();
	}
};

class MessagePacket : public Packet {
	user_id_t from_uid;
	user_id_t to_uid;
	std::string content;
	void deserialize(std::string& s) {
		std::istringstream ss(s);
		ss >> from_uid;
		ss >> to_uid;
		size_t quote = s.find('"', 0);
		if (quote != std::string::npos) {
			size_t unquote = s.find('"', quote + 1);
			if (unquote != std::string::npos)
				content = std::string(s, quote + 1, unquote - (quote + 1));
		}
	}

	public:
	void set_from_uid(user_id_t id) { from_uid = id; }
	user_id_t get_from_uid() const { return from_uid; }
	void set_to_uid(user_id_t id) { to_uid = id; }
	user_id_t get_to_uid() const { return to_uid; }
	void set_content(const std::string& c) { content = c; }
	const std::string& get_content() const { return content; }
	MessagePacket(std::uint16_t version, user_id_t sender, user_id_t recipient,
				  std::string content)
		: Packet(version, MESSAGE), from_uid(sender), to_uid(recipient),
		  content(content) {}
	/** @brief Deserializing constructor. */
	MessagePacket(std::string s) {
		Packet::begin_deserialization(s);
		deserialize(s);
	}
	std::string serialize() {
		std::ostringstream ss;
		ss << Packet::serialize() << " " << from_uid << " " << to_uid << " \""
		   << content << "\"";
		return ss.str();
	}
};
#endif
