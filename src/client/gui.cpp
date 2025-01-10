#include "client.h"

void ClientGui::send_msg() {
	user_id_t to = std::stoi(stack->get_visible_child_name());
	MessagePacket mp(api_ver, user, to, msgentry->get_buffer()->get_text());
	client.async_send(mp.serialize());
	talks[to]->insert_markup(talks[to]->end(),
							 "<b>You:</b> " + mp.get_content() + "\n");
	msgentry->get_buffer()->set_text("");
}

void ClientGui::new_contact() {
	user_id_t contact = std::stoi(contactentry->get_buffer()->get_text());
	if (talks.find(contact) == talks.end())
		new_talk(contact);
	contactentry->get_buffer()->set_text("");
	stack->set_visible_child(talkviews[contact]);
}

void ClientGui::on_msg_received() {
	client.msg_sem[1]->acquire();
	MessagePacket mp(client.msg_to_process);
	user_id_t from = mp.get_from_uid();
	if (talks.find(from) == talks.end())
		new_talk(from);
	talks[from]->insert_markup(talks[from]->end(),
							   "<b>" + std::to_string(from) + ":</b> " +
								   mp.get_content() + "\n");
	client.msg_sem[0]->release();
}

void ClientGui::new_talk(user_id_t with) {
	talks[with] = Gtk::TextBuffer::create();
	talkviews[with].set_buffer(talks[with]);
	stack->add(talkviews[with], std::to_string(with), std::to_string(with));
}

int ClientGui::run() { return app->run(); }
ClientGui::ClientGui(std::string server, user_id_t user)
	: ctx(), client(ctx, server, user, [this] { dispatcher.emit(); }), user(user),
	  app(Gtk::Application::create()),
	  builder(Gtk::Builder::create_from_file("babs.ui")),
	  window(builder->get_widget<Gtk::ApplicationWindow>("Window")),
	  stack(builder->get_widget<Gtk::Stack>("Stack")),
	  sidebar(builder->get_widget<Gtk::StackSidebar>("Sidebar")),
	  contactentry(builder->get_widget<Gtk::Entry>("ContactEntry")),
	  msgentry(builder->get_widget<Gtk::Entry>("MsgEntry")) {
	app->signal_startup().connect([this] {
		app->add_window(*window);
		window->set_visible(true);
	});
	window->set_title(std::to_string(user) + " at " + server +
					  " — Babs Client");
	window->signal_hide().connect([this] { delete window; });
	sidebar->set_stack(*stack);
	msgentry->signal_activate().connect(sigc::mem_fun(*this, &ClientGui::send_msg));
	contactentry->signal_activate().connect(sigc::mem_fun(*this, &ClientGui::new_contact));
	dispatcher.connect(sigc::mem_fun(*this, &ClientGui::on_msg_received));
	new_talk(0); // uid 0 is server
	talks[0]->insert(talks[0]->end(),
					 "Server: " + server + " port " + port + "\n");
	HelloPacket hello = client.connect();
	talks[0]->insert(talks[0]->end(),
					 "Received welcome message: " + hello.get_name() + "\n");
	client.login();
	client.start_worker();
}
