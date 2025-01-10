#include "client.h"

HelloPacket Client::connect() {
	boost::system::error_code error;
	tcp::resolver resolver(ctx);
	tcp::resolver::results_type endpoints =
		resolver.resolve(server, port, error);
	if (error)
		throw boost::system::system_error(error);
	boost::asio::connect(session->get_socket(), endpoints, error);
	if (error)
		throw boost::system::system_error(error);
	HelloPacket h(session->receive());
	return h;
}
void Client::login() { session->send(LoginPacket(api_ver, user).serialize()); }
void Client::start_worker() {
	listen();
	worker = std::thread([this]() { ctx.run(); });
	worker.detach();
}
void Client::handle_recv(std::string p) {
	Packet packet(p);
	switch (packet.get_type()) {
	case MESSAGE: {
		msg_sem[0]->acquire();
		msg_to_process = p;
		notify_ui();
		msg_sem[1]->release();
		break;
	}
	}
}
void Client::listen() {
	auto handle = [this](std::string p) {
		this->handle_recv(p);
		this->listen();
	};
	session->async_receive(handle);
}

Client::Client(boost::asio::io_context& ctx, const std::string& server, user_id_t user,
			   std::function<void()> notify)
	: ctx(ctx), notify_ui(notify), server(server), user(user) {
	session = std::make_shared<Session>(ctx, user);
	msg_sem[0] = std::make_shared<std::binary_semaphore>(1);
	msg_sem[1] = std::make_shared<std::binary_semaphore>(0);
}
