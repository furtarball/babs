#include "packet.h"

void Packet::deserialize(std::string& s) {
	std::istringstream ss(s);
	ss >> version;
	ss >> type;
	s = std::string(ss.str(), static_cast<size_t>(ss.tellg()) + 1);
}

std::string Packet::serialize() {
	std::ostringstream ss;
	ss << version << " " << static_cast<int>(type);
	return ss.str();
}

void HelloPacket::deserialize(std::string& s) {
	size_t quote = s.find('"', 0);
	if (quote != std::string::npos) {
		size_t unquote = s.find('"', quote + 1);
		if (unquote != std::string::npos)
			name = std::string(s, 1, unquote - 1);
	}
}

std::string HelloPacket::serialize() {
	std::ostringstream ss;
	ss << Packet::serialize() << " \"" << name << "\"";
	return ss.str();
}

void LoginPacket::deserialize(std::string& s) {
	std::istringstream ss(s);
	ss >> uid;
}

std::string LoginPacket::serialize() {
	std::ostringstream ss;
	ss << Packet::serialize() << " " << uid;
	return ss.str();
}

void MessagePacket::deserialize(std::string& s) {
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

std::string MessagePacket::serialize() {
	std::ostringstream ss;
	ss << Packet::serialize() << " " << from_uid << " " << to_uid << " \""
	   << content << "\"";
	return ss.str();
}
