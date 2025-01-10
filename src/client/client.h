/**
	@file client.h
	@brief Implementation of the client and its graphical interface.
*/

#ifndef CLIENT_H
#define CLIENT_H
#include "../common/packet.h"
#include "../common/session.h"
#include <array>
#include <functional>
#include <gtkmm.h>
#include <map>
#include <memory>
#include <semaphore>
#include <thread>

using boost::asio::ip::tcp;
extern const char* port;
extern const std::uint16_t api_ver;
extern const char* name;
extern const std::uint16_t app_ver;

/** @brief Underlying client. */
class Client {
	boost::asio::io_context& ctx;
	std::shared_ptr<Session> session;
	/** @brief Worker thread for running the io_context.
	
		Stopped by destructor. */
	std::thread worker;
	/** @brief Function for notifying ClientGui's dispatcher object.
		@see ClientGui::dispatcher */
	std::function<void()> notify_ui;
	std::string server;
	user_id_t user;
	/** @brief Notifies dispatcher of incoming message.
		@see ClientGui::dispatcher */
	void handle_recv(std::string p);

	public:
	/** @brief A place to share received messages between threads.
		@see ClientGui::dispatcher */
	std::string msg_to_process;
	/** @brief Semaphores guarding msg_to_process.
		@see msg_to_process */
	std::array<std::shared_ptr<std::binary_semaphore>, 2> msg_sem;

	/** @brief Simply calls session->async_send.
		@see Session::async_send() */
	void async_send(std::string s) { session->async_send(s); }
	/** @brief Simply calls session->send.
		@see Session::send() */
	void send(std::string s) { session->send(s); }

	/** @brief Connect to server and return the hello packet it sent */
	HelloPacket connect();
	/** @brief Login with credentials passed earlier to constructor */
	void login();
	/** @brief Start worker thread.
		@see worker */
	void start_worker();
	/** @brief Listen to incoming packets.
	
		Executed in a loop by worker. Calls handle_recv().
		@see worker
		@see handle_recv() */
	void listen();
	Client(boost::asio::io_context& ctx, const std::string& server, user_id_t u,
		   std::function<void()> notify);
	~Client() { ctx.stop(); }
};

/** @brief Gtk-based GUI for the client. */
class ClientGui {
	boost::asio::io_context ctx;
	Client client;
	user_id_t user;

	Glib::RefPtr<Gtk::Application> app;
	Glib::RefPtr<Gtk::Builder> builder;
	Gtk::Window* window;
	Gtk::Stack* stack;
	Gtk::StackSidebar* sidebar;
	Gtk::Entry *contactentry, *msgentry;
	std::map<user_id_t, Glib::RefPtr<Gtk::TextBuffer>> talks;
	std::map<user_id_t, Gtk::TextView> talkviews;

	/** @brief The workaround for Gtk's lack of thread-safety. */
	Glib::Dispatcher dispatcher;

	void send_msg();
	void new_contact();
	void on_msg_received();
	void new_talk(user_id_t with);

	public:
	int run();
	ClientGui(std::string server, user_id_t user);
};

#endif
