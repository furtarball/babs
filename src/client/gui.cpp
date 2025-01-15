#include "client.h"

void Talk::scroll_down() {
	auto adj = window.get_vadjustment();
	adj->set_value(adj->get_upper());
}
Talk::Talk() : buffer(Gtk::TextBuffer::create()), view(buffer) {
	window.set_child(view);
	view.set_editable(false);
	view.set_cursor_visible(false);
	view.set_monospace(true);
	view.set_wrap_mode(Gtk::WrapMode::WORD_CHAR);
}

void ClientGui::send_msg() {
	user_id_t to = std::stoi(stack->get_visible_child_name());
	auto entry = msgentry->get_buffer();
	MessagePacket mp(api_ver, user, to, entry->get_text());
	client.async_send(mp.serialize());
	auto talk = talks[to].get_buffer();
	talk->insert_markup(talk->end(), "<b>You:</b> " + mp.get_content() + "\n");
	entry->set_text("");
	talks[to].scroll_down();
}

void ClientGui::new_contact() {
	auto entry = contactentry->get_buffer();
	user_id_t contact = std::stoi(entry->get_text());
	if (talks.find(contact) == talks.end())
		new_talk(contact);
	entry->set_text("");
	stack->set_visible_child(talks[contact].get_window());
	msgentry->grab_focus();
}

void ClientGui::on_msg_received() {
	client.msg_sem[1]->acquire();
	MessagePacket mp(client.msg_to_process);
	user_id_t from = mp.get_from_uid();
	if (talks.find(from) == talks.end())
		new_talk(from);
	auto talk = talks[from].get_buffer();
	talk->insert_markup(talk->end(), "<b>" + std::to_string(from) + ":</b> " +
										 mp.get_content() + "\n");
	talks[from].scroll_down();
	client.msg_sem[0]->release();
}

void ClientGui::new_talk(user_id_t with) {
	stack->add(talks[with].get_window(), std::to_string(with),
			   std::to_string(with));
}

int ClientGui::run() { return app->run(); }

ClientGui::ClientGui(std::string server, user_id_t user)
	: ctx(), client(ctx, server, user, [this] { dispatcher.emit(); }),
	  user(user), style_mgr(adw_style_manager_get_default()),
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
	msgentry->signal_activate().connect(
		sigc::mem_fun(*this, &ClientGui::send_msg));
	msgentry->grab_focus();
	contactentry->signal_activate().connect(
		sigc::mem_fun(*this, &ClientGui::new_contact));
	dispatcher.connect(sigc::mem_fun(*this, &ClientGui::on_msg_received));
	new_talk(0); // uid 0 is server
	{
		auto server_talk = talks[0].get_buffer();
		server_talk->insert(talks[0].get_buffer()->end(),
							"Server: " + server + " port " + port + "\n");
		HelloPacket hello = client.connect();
		server_talk->insert(server_talk->end(), "Received welcome message: " +
													hello.get_name() + "\n");
	}
	client.login();
	client.start_worker();
}
