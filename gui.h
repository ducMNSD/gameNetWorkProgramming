#include <gtk/gtk.h>


GtkBuilder *builder;
GtkWidget *button_login;

GtkWidget *loginDialog;
GtkWidget *mainDialog;
GtkWidget *playDialog;
GtkWidget *listRoomDialog;
GtkWidget *loginButton;
GtkWidget *registerButton;
GtkWidget *userName;
GtkWidget *passWord;
GtkWidget *findRoomButton;
GtkWidget *createRoomButton;
GtkWidget *resumeButton;
GtkWidget *logoutButton;
GtkWidget *listRoomArea;
GtkWidget *idRoomButton;
GtkWidget *idRoomText;
GtkWidget *map;
GtkWidget *playInput;
GtkWidget *leftButton;
GtkWidget *rightButton;


void gui_init(){
    loginDialog = GTK_WIDGET(gtk_builder_get_object(builder,"loginDialog"));
    mainDialog = GTK_WIDGET(gtk_builder_get_object(builder,"mainDialog"));
    playDialog = GTK_WIDGET(gtk_builder_get_object(builder,"playDialog"));
    listRoomDialog = GTK_WIDGET(gtk_builder_get_object(builder,"listRoomDialog"));
    loginButton = GTK_WIDGET(gtk_builder_get_object(builder,"loginButton"));
    registerButton = GTK_WIDGET(gtk_builder_get_object(builder,"registerButton"));
	userName = GTK_WIDGET(gtk_builder_get_object(builder,"userName"));
	passWord = GTK_WIDGET(gtk_builder_get_object(builder,"passWord"));
	findRoomButton = GTK_WIDGET(gtk_builder_get_object(builder,"findRoomButton"));
	createRoomButton = GTK_WIDGET(gtk_builder_get_object(builder,"createRoomButton"));
	resumeButton = GTK_WIDGET(gtk_builder_get_object(builder,"resumeButton"));
	logoutButton = GTK_WIDGET(gtk_builder_get_object(builder,"logoutButton"));
	listRoomArea = GTK_WIDGET(gtk_builder_get_object(builder,"listRoomArea"));
	idRoomButton = GTK_WIDGET(gtk_builder_get_object(builder,"idRoomButton"));
	idRoomText = GTK_WIDGET(gtk_builder_get_object(builder,"idRoomText"));
	map = GTK_WIDGET(gtk_builder_get_object(builder,"map"));
	playInput = GTK_WIDGET(gtk_builder_get_object(builder,"playInput"));
	leftButton = GTK_WIDGET(gtk_builder_get_object(builder,"leftButton"));
	rightButton = GTK_WIDGET(gtk_builder_get_object(builder,"rightButton"));

}