#include "server.h"

std::string name_version() {
	return std::string(name) + " v" + std::to_string(api_ver) + "." +
		   std::to_string(app_ver);
}

void Server::start_accept() {
	std::shared_ptr<Session> new_connection(new Session(ctx));
	acceptor.async_accept(
		new_connection->get_socket(),
		[this, new_connection](const boost::system::error_code& error) {
			if (!error) {
				init(new_connection);
				recv(new_connection);
			} else
				throw boost::system::system_error(error);
			start_accept();
		});
}

void Server::init(std::shared_ptr<Session> s) {
	HelloPacket p(api_ver, name_version());
	s->async_send(p.serialize());
}

bool Server::handle_recv(std::string p, std::shared_ptr<Session> s) {
	Packet packet(p);
	switch (packet.get_type()) {
	case NONE: {
		for (auto i = sessions[s->get_uid()].begin();
			 i < sessions[s->get_uid()].end(); i++) {
			if ((*i)->get_id() == s->get_id())
				sessions[s->get_uid()].erase(i);
		}
		return false;
	}
	case LOGIN: {
		LoginPacket lp(p);
		std::cout << "User Logged in: " << lp.get_uid()
				  << " (session id: " << s->get_id() << ")" << std::endl;
		s->set_uid(lp.get_uid());
		sessions[lp.get_uid()].push_back(s);
		break;
	}
	case MESSAGE: {
		MessagePacket mp(p);
		std::cout << "Routing message from " << mp.get_from_uid() << " to "
				  << mp.get_to_uid() << std::endl;
		for (auto i : sessions[mp.get_to_uid()])
			i->async_send(mp.serialize());
		break;
	}
	}
	return true;
}

void Server::recv(std::shared_ptr<Session> s) {
	auto handle = [this, s](std::string p) {
		if (this->handle_recv(p, s))
			this->recv(s);
	};
	s->async_receive(handle);
}

Server::Server()
	: ctx(), acceptor(ctx, tcp::endpoint(tcp::v4(), port)) {
	start_accept();
}
