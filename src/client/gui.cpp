#include "client.h"
#include "gresource.h"

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
	MessagePacket mp(api_ver, client->get_user(), to, entry->get_text());
	client->async_send(mp.serialize());
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
	client->msg_sem[1]->acquire();
	MessagePacket mp(client->msg_to_process);
	user_id_t from = mp.get_from_uid();
	if (talks.find(from) == talks.end())
		new_talk(from);
	auto talk = talks[from].get_buffer();
	talk->insert_markup(talk->end(), "<b>" + std::to_string(from) + ":</b> " +
										 mp.get_content() + "\n");
	talks[from].scroll_down();
	client->msg_sem[0]->release();
}

void ClientGui::new_talk(user_id_t with) {
	stack->add(talks[with].get_window(), std::to_string(with),
			   std::to_string(with));
}

void ClientGui::on_login_confirmed() {
	auto serverentry =
		gtk_builder_get_object(builder->gobj(), "LoginDialogServerEntry");
	std::string server{gtk_editable_get_text(GTK_EDITABLE(serverentry))};
	auto uidentry =
		gtk_builder_get_object(builder->gobj(), "LoginDialogUIDEntry");
	std::string uid{gtk_editable_get_text(GTK_EDITABLE(uidentry))};
	client = std::make_unique<Client>(ctx, server, std::stoi(uid),
									  [this] { dispatcher.emit(); });
	new_talk(0); // uid 0 is server
	{
		auto server_talk = talks[0].get_buffer();
		server_talk->insert(talks[0].get_buffer()->end(),
							"Server: " + server + " port " + port + "\n");
		HelloPacket hello = client->connect();
		server_talk->insert(server_talk->end(), "Received welcome message: " +
													hello.get_name() + "\n");
	}
	auto logindialog = gtk_builder_get_object(builder->gobj(), "LoginDialog");
	adw_dialog_close(ADW_DIALOG(logindialog));
	msgentry->grab_focus();
	client->login();
	window->set_title(uid + " at " + server + " — Babs Client");
	client->start_worker();
}

void ClientGui::toggle_sidebar() {
	auto splitview = gtk_builder_get_object(builder->gobj(), "SplitView");
	adw_overlay_split_view_set_show_sidebar(ADW_OVERLAY_SPLIT_VIEW(splitview),
											sidebartoggle->get_active());
}

void ClientGui::setup_login_dialog() {
	auto logindialog = gtk_builder_get_object(builder->gobj(), "LoginDialog");
	adw_dialog_present(ADW_DIALOG(logindialog), GTK_WIDGET(window->gobj()));
	auto confirmbtn = builder->get_widget<Gtk::Button>("LoginDialogConfirmBtn");
	confirmbtn->signal_clicked().connect([this] { on_login_confirmed(); });
}

int ClientGui::run() { return app->run(); }

ClientGui::ClientGui()
	: ctx(), style_mgr(adw_style_manager_get_default()),
	  app(Gtk::Application::create()),
	  builder(Gtk::Builder::create_from_resource(
		  "/io/github/furtarball/babs/babs.ui")),
	  window(builder->get_widget<Gtk::ApplicationWindow>("Window")),
	  stack(builder->get_widget<Gtk::Stack>("Stack")),
	  sidebar(builder->get_widget<Gtk::StackSidebar>("Sidebar")),
	  contactentry(builder->get_widget<Gtk::Entry>("ContactEntry")),
	  msgentry(builder->get_widget<Gtk::Entry>("MsgEntry")),
	  sidebartoggle(builder->get_widget<Gtk::ToggleButton>("SidebarToggle")) {
	app->signal_startup().connect([this] {
		app->add_window(*window);
		window->set_visible(true);
	});
	window->set_title("Babs Client");
	window->signal_close_request().connect(
		[this]() -> bool {
			delete window;
			return true;
		},
		false);
	sidebar->set_stack(*stack);
	msgentry->signal_activate().connect([this] { send_msg(); });
	contactentry->signal_activate().connect([this] { new_contact(); });
	sidebartoggle->set_active(true);
	sidebartoggle->signal_toggled().connect([this] { toggle_sidebar(); });
	dispatcher.connect([this] { on_msg_received(); });
	setup_login_dialog();
}
