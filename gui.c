#include "client.h"
#include "game_client.c"

GtkWidget *window;
GtkWidget *login_window = NULL;
GtkWidget *inputUsername;
GtkWidget *inputPassword;
GtkWidget *inputRePassword;

GtkWidget* label_Name;
GtkWidget* label_atk;
GtkWidget* label_def;
GtkWidget* label_hp;
GtkWidget* label_mp;
GtkWidget* label_level;
GtkWidget* label_point;
GtkWidget* label_win;
GtkWidget* label_lose;
char password[100];
char repassword[100];
char username[100];

void showMessage(GtkWidget *parent, GtkMessageType type, char *mms, char *content);

void onExit(GtkWidget *widget, gpointer data)
{
	exit(0);
}
void on_window_main_destroy(GtkWidget *widget, gpointer gp)
{
	exit(0);
}
void onLoginButtonClicked(GtkWidget *widget, gpointer gp)
{
	strncpy(username, (char *)gtk_entry_get_text(GTK_ENTRY(inputUsername)),32);
	strncpy(password, (char *)gtk_entry_get_text(GTK_ENTRY(inputPassword)),32);
	if (strlen(username) < 1 || strlen(password) < 1)
		showMessage(login_window, GTK_MESSAGE_WARNING, sLOGIN_FAILED, sNOT_EMPTY);
	else login(username,password);
}
void onSignupButtonClicked(GtkWidget *widget, gpointer gp)
{
	strncpy(username, (char *)gtk_entry_get_text(GTK_ENTRY(inputUsername)),32);
	strncpy(password, (char *)gtk_entry_get_text(GTK_ENTRY(inputPassword)),32);
	strncpy(repassword, (char *)gtk_entry_get_text(GTK_ENTRY(inputRePassword)),32);
	if (strlen(username) < 1 || strlen(password) < 1)
		showMessage(login_window, GTK_MESSAGE_WARNING, sSIGNUP_FAILED, sNOT_EMPTY);
	else if (strcmp(password,repassword)) 
		showMessage(login_window, GTK_MESSAGE_WARNING, sSIGNUP_FAILED, sNOT_SAME);
	else signup(username,password);
}

void showMessage(GtkWidget *parent, GtkMessageType type, char *mms, char *content)
{
	GtkWidget *mdialog;
	mdialog = gtk_message_dialog_new(GTK_WINDOW(parent),
									 GTK_DIALOG_DESTROY_WITH_PARENT,
									 type,
									 GTK_BUTTONS_OK,
									 "%s", mms);
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(mdialog), "%s", content);
	gtk_widget_show_all(mdialog);
	gtk_dialog_run(GTK_DIALOG(mdialog));
	gtk_widget_destroy(mdialog);
}

void initLogin()
{
	GtkBuilder      *builder; 
 
    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "glade/a.glade", NULL);
 
    login_window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
    inputUsername = GTK_WIDGET(gtk_builder_get_object(builder, "u"));
    inputPassword = GTK_WIDGET(gtk_builder_get_object(builder, "p"));
    inputRePassword = GTK_WIDGET(gtk_builder_get_object(builder, "rp"));
    gtk_builder_connect_signals(builder, NULL);
 
    g_object_unref(builder);
 
    gtk_widget_show(login_window);                
}
void initMain()
{
	GtkBuilder      *builder; 
 
    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "glade/b.glade", NULL);
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
    gtk_builder_connect_signals(builder, NULL);
    label_Name = GTK_WIDGET(gtk_builder_get_object(builder, "name"));
    label_atk = GTK_WIDGET(gtk_builder_get_object(builder, "atk"));
    label_def = GTK_WIDGET(gtk_builder_get_object(builder, "def"));
    label_hp = GTK_WIDGET(gtk_builder_get_object(builder, "hp"));
    label_mp = GTK_WIDGET(gtk_builder_get_object(builder, "mp"));
    label_level = GTK_WIDGET(gtk_builder_get_object(builder, "level"));
    label_point = GTK_WIDGET(gtk_builder_get_object(builder, "point"));
    label_win = GTK_WIDGET(gtk_builder_get_object(builder, "win"));
    label_lose = GTK_WIDGET(gtk_builder_get_object(builder, "lose"));
 	
    g_object_unref(builder);
 
    gtk_widget_show(window);   
}
void switchMain(){
	logined =1 ;
	// printf("switchMain\n");
	// gtk_main_quit();
	gtk_widget_destroy(login_window);
	// g_timeout_add(100,recv_loop,NULL);
	initMain();
	// gtk_main();
}
void switchGame()
{
	gtk_widget_hide(window);
	g_source_remove(loopEvent);

	if (game()) showMessage(NULL, GTK_MESSAGE_WARNING, sYOU_WIN, "");
	else showMessage(NULL, GTK_MESSAGE_WARNING, sYOU_LOSE, "");
	gaming=0;
	initMain();//back to main
	updateView();
	loopEvent = g_timeout_add(100,recv_loop,NULL);
	gtk_main();
}
void on_game(GtkWidget *widget, gpointer gp){if (gaming) return;packet[0]=GAME_REQ;sendPacket(1);gaming=1;}
void updateView()
{
	char atk[10];
	char def[10];
	char hp[10];
	char mp[10];
	char level[10];
	char point[10];
	char win[10];
	char lose[10];
	sprintf(atk,"%d",User.atk);
	sprintf(def,"%d",User.def);
	sprintf(hp,"%d",User.hp);
	sprintf(mp,"%d",User.mp);
	sprintf(level,"%d",User.level);
	sprintf(point,"%d",User.point);
	sprintf(win,"%d",User.win);
	sprintf(lose,"%d",User.lose);
	gtk_label_set_text(GTK_LABEL(label_atk),atk);
	gtk_label_set_text(GTK_LABEL(label_Name),User.user);
	gtk_label_set_text(GTK_LABEL(label_def),def);
	gtk_label_set_text(GTK_LABEL(label_hp),hp);
	gtk_label_set_text(GTK_LABEL(label_mp),mp);
	gtk_label_set_text(GTK_LABEL(label_level),level);
	gtk_label_set_text(GTK_LABEL(label_point),point);
	gtk_label_set_text(GTK_LABEL(label_win),win);
	gtk_label_set_text(GTK_LABEL(label_lose),lose);
}
void on_upgrade_hp(GtkWidget *widget, gpointer gp) {sendUpdate(HPUP);}
void on_upgrade_mp(GtkWidget *widget, gpointer gp) {sendUpdate(MPUP);}
void on_upgrade_def(GtkWidget *widget, gpointer gp) {sendUpdate(DEFUP);}
void on_upgrade_atk(GtkWidget *widget, gpointer gp) {sendUpdate(ATKUP);}