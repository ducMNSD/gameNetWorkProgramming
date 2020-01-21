#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <pthread.h>
#include <unistd.h>
#include "StringProcess.h"
#include "object.h"
#include "gui.h"

#define MAX 1000
#define PORT 3000
#define SA struct sockaddr 

user user_infor;
room room_infor;

int CreateRoom(GtkButton *button,gpointer userdata);
int GetListRoom(GtkButton *button,gpointer userdata);
int PickRoom(GtkButton *button,gpointer userdata);
int login(GtkButton *button,gpointer userdata);
int Register(GtkButton *button,gpointer userdata);
int Logout(GtkButton *button,gpointer userdata);
int run_game(char *lenh,int player);							// process map
void show_array();
int resume(GtkButton *button,gpointer userdata);
void Play(int sockfd);
void init();												 	// init room infor vs user infor
void read_map(int sockfd);
void Play_resume(int sockfd);									
char *render_map();
void set_play_input_text(char * text);
void set_textView_text(char * text);
void wait(int sockfd);
void play_left(GtkButton *button, gpointer userdata);
void play_right(GtkButton *button, gpointer userdata);

void *read_server(void *sockfd_pt){
	int sockfd = *(int*)sockfd_pt;
	char server_message[MAX];
	while (1){
		bzero(server_message,sizeof(server_message));
		read(sockfd,server_message,MAX);
		clearString(server_message);
		printf("Server Message: %s\n",server_message);
		if(strlen(server_message)!= 0){
			if(strcmp(server_message,"START")==0){
				set_play_input_text("");
				room_infor.turn = 1;	
			}
			else{
				run_game(server_message,room_infor.turn);
				gtk_label_set_text(GTK_LABEL(map),render_map());
				if(room_infor.turn == 1) room_infor.turn = 2;
				else room_infor.turn = 1;
				set_play_input_text("");
			}
			// break;
		}
		break;
	}
}

void Show_message(GtkWidget * parent , GtkMessageType type,  char * mms, char * content) {
	GtkWidget *mdialog;
	mdialog = gtk_message_dialog_new(GTK_WINDOW(parent),
	                                 GTK_DIALOG_DESTROY_WITH_PARENT,
	                                 type,
	                                 GTK_BUTTONS_OK,
	                                 "%s", mms);
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(mdialog), "%s",  content);
	gtk_dialog_run(GTK_DIALOG(mdialog));
	gtk_widget_destroy(mdialog);
}

void set_textView_text(char * text) {
	GtkTextBuffer *buffer;
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(listRoomArea));
	if (buffer == NULL) {
		printf("Get buffer fail!");
		buffer = gtk_text_buffer_new(NULL);
	}
	gtk_text_buffer_set_text(buffer, text, -1);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(listRoomArea), buffer);
}

void set_play_input_text(char * text) {
	gtk_entry_set_text(GTK_ENTRY(playInput),text);
}

void Run_Game(int *sockfd){
	gtk_widget_show_all(mainDialog);
	g_signal_connect(G_OBJECT(findRoomButton),"clicked",G_CALLBACK(GetListRoom),sockfd);
	g_signal_connect(G_OBJECT(createRoomButton),"clicked",G_CALLBACK(CreateRoom),sockfd);
	g_signal_connect(G_OBJECT(resumeButton),"clicked",G_CALLBACK(resume),sockfd);
	g_signal_connect(G_OBJECT(logoutButton),"clicked",G_CALLBACK(Logout),sockfd);
	g_signal_connect(G_OBJECT(leftButton),"clicked",G_CALLBACK(play_left),sockfd);
	g_signal_connect(G_OBJECT(rightButton),"clicked",G_CALLBACK(play_right),sockfd);
}

int Run_Program(int *sockfd){
	gtk_widget_show_all(loginDialog);
	g_signal_connect(G_OBJECT(loginButton),"clicked",G_CALLBACK(login),sockfd);
	g_signal_connect(G_OBJECT(registerButton),"clicked",G_CALLBACK(Register),sockfd);
	g_signal_connect(G_OBJECT(idRoomButton),"clicked",G_CALLBACK(PickRoom),sockfd);
}

int main(int argc , char *argv[]){ 
	init();
	gtk_init(&argc,&argv);

	builder = gtk_builder_new();
    gtk_builder_add_from_file(builder,"test_glade.glade",NULL);
    gui_init();

	g_object_unref(builder);

	int sockfd, connfd; 
	struct sockaddr_in servaddr, cli; 

	// socket create and varification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	servaddr.sin_port = htons(atoi(argv[1])); 

	// connect the client socket to server socket 
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
		printf("connection with the server failed...\n"); 
		exit(0); 
	} 
	else printf("connected to the server..\n"); 

	Run_Program(&sockfd);
    gtk_main();
	close(sockfd); 
} 

void init(){
	// init user_infor
	user_infor.id = 0;
	user_infor.roomId = 0;
	user_infor.status = 0;
    strcpy(user_infor.password,"");
    strcpy(user_infor.userName,"");
	// init room_infor
	
	room_infor.id = 0;
	room_infor.turn = 0;
	room_infor.player_1_id = 0;
	room_infor.player_2_id = 0;
	room_infor.player_1_result = 0;
    room_infor.player_2_result = 0;
    room_infor.player_1_point = 0;
    room_infor.player_2_point = 0;
    room_infor.player_1_king = 4;
    room_infor.player_2_king = 4;
    for(int j=0; j < 12; j++){
        if( j == 5 || j == 11 ){
            room_infor.array[j] = 1;
        }
        else
        {
            room_infor.array[j] = 5;
        }
            
    }
}

int PickRoom(GtkButton *button,gpointer userdata){
	int *sockfd_pt = (int *) userdata;
	int sockfd = *sockfd_pt;
	int room_id;
	char message[MAX];
	char buffer[MAX];
	// read data from GUI
	int user_id = user_infor.id;
	room_id = atoi(gtk_entry_get_text(GTK_ENTRY(idRoomText)));
	sprintf(message,"5 %d %d",user_id,room_id);
	// send to server
	write(sockfd,message,strlen(message));
	bzero(buffer,sizeof(buffer));
	// wait message from server
	// read(sockfd,buffer,sizeof(buffer));
	read(sockfd,buffer,MAX);
	clearString(buffer);
	// handle buffer
	if(get_controller(buffer) == 501){								// fail 
		printf("%s\n",get_first_param(buffer));
		Show_message(loginDialog,GTK_MESSAGE_ERROR,"Thất bại",get_first_param(buffer));
		gtk_widget_hide(listRoomDialog);
		gtk_widget_show_all(mainDialog);
		return 0;
	}
	else{
		if(get_controller(buffer) == 201 ){							// pick new room
			user_infor.status = 2;									// UPDATE USER_INFOR VA ROOM_INFOR
			room_infor.id = room_id;
			room_infor.player_2_id = user_infor.id;
			printf("%s\n",get_first_param(buffer));
			room_infor.turn = 1;
			gtk_widget_hide(listRoomDialog);					     // wait user 1 
			wait(sockfd);
			return 1;
		}
		else if(get_controller(buffer) == 202 ) {                   // pick resume room
			char turn[MAX];											// update room_infor vs user_infor
			strcpy(turn, get_second_param(buffer));
			room_infor.turn = turn[0] - 48;
			char player[MAX];
			int h = 0;
			int index = 0;
			for(int i=strlen(turn)-1; turn[i]!= '-'; i--){
				h = i;
			}
			index = h;
			for(h;h < strlen(turn);h++){
				player[h-index] = turn[h];
			}
			// add king point
			int arr[4];
			h = 0;
			char point[MAX];
			for(int i=2; i <= index;){
				strcpy(point,"");
				int j = 0;
				while( turn[i] != '-' && i <= index){
					point[j] = turn[i];
					j++;
					i++;
				}
				arr[h] = atoi(point);
				h++;
				i++;
			}
			room_infor.player_1_king = arr[0];
			room_infor.player_2_king = arr[1];
			room_infor.player_1_point = arr[2];
			room_infor.player_2_point = arr[3];
			
			// add player role
			player[7] = '\0';
			printf("player: %s\n",player);
			if(strcmp(player,"player1")==0){
				room_infor.player_1_id = user_infor.id;
			}
			else{
				room_infor.player_2_id = user_infor.id;	
			} 
			/// get map
			char first[MAX];
			strcpy(first,get_first_param(buffer));
			int k = 0;
			for(int i=0; i < strlen(get_first_param(buffer));){
				strcpy(point,"");
				bzero(point,MAX);
				int j = 0;
				while( first[i] != '-' && i < strlen(first)){
					point[j] = first[i];
					j++;
					i++;
				}
				point[strlen(point)] = '\0';
				room_infor.array[k] = atoi(point);
				k++;
				i++;
			}

			gtk_widget_hide(listRoomDialog);
			gtk_label_set_text(GTK_LABEL(map),render_map());
			printf("pick room player1: %d,player2: %d, turn: %d\n",room_infor.player_1_id,room_infor.player_2_id,room_infor.turn);		
			if(room_infor.player_1_id != 0 && room_infor.turn == 1){
				Play(sockfd);
			}
			if(room_infor.player_2_id != 0 && room_infor.turn == 2){
				Play(sockfd);
			}
			else{
				printf("PICK WAIT\n");
				wait(sockfd);
			}
			return 1;
		}
	}
}

int GetListRoom(GtkButton *button,gpointer userdata){ //client
	printf("GET LIST ROOM\n");
	int *sockfd_pt = (int *) userdata;
	int sockfd = *sockfd_pt;
	char buffer[MAX];
	int id = user_infor.id;
	char message[30];
	sprintf(message,"4 %d",id);
	write(sockfd,message,sizeof(message));
	bzero(buffer,MAX);
	read(sockfd,buffer,sizeof(buffer));
	clearString(buffer);
	
	if(get_controller(buffer)==501){
		printf("\n\t%s\n",get_first_param(buffer));
		Show_message(loginDialog,GTK_MESSAGE_ERROR,"Thất bại",get_first_param(buffer));
		return 0;
	}
	else{
		set_textView_text(buffer);
		gtk_widget_hide(mainDialog);
		gtk_widget_show_all(listRoomDialog);
		return 1;
	}
}
 
int CreateRoom(GtkButton *button,gpointer userdata){
	int *sockfd_pt = (int *) userdata;
	int sockfd = *sockfd_pt;
	char buffer[MAX];
	int id = user_infor.id;
	char message[30];
	sprintf(message,"3 %d",id);
	write(sockfd,message,sizeof(message));
	bzero(buffer,sizeof(buffer));
	// read(sockfd,buffer,sizeof(buffer));
	read(sockfd,buffer,MAX);
	clearString(buffer);
	if(get_controller(buffer) == 201){
		user_infor.status = 2;
		room_infor.status = 1;
		room_infor.id = atoi(get_first_param(buffer)); 
		room_infor.player_1_id = user_infor.id;
		printf("\n\t%s\n",get_second_param(buffer));
		gtk_widget_hide(mainDialog);
		wait(sockfd);
		return 1;
	}
	else{ 
		Show_message(loginDialog,GTK_MESSAGE_ERROR,"Thất bại",get_first_param(buffer));
		printf("\n\t%s\n",get_first_param(buffer));
		return 0;
	}

}

int login(GtkButton *button,gpointer userdata){
	int *sockfd_pt = (int *) userdata;
	int sockfd = *sockfd_pt;
	char username[30];
	char password[30];
	char message[MAX];
	char message_login[100];
	bzero(username,sizeof(username));
	bzero(password,sizeof(password));
	// printf("Username: "); scanf("%s", userName);
	strcpy(username,gtk_entry_get_text(GTK_ENTRY(userName)));
	// printf("Password: "); scanf("%s", password);
	strcpy(password,gtk_entry_get_text(GTK_ENTRY(passWord)));
	sprintf(message, "1 %s %s", username, password);
	printf("Login_Message:1 %s %s\n", username, password);
	write(sockfd, message, strlen(message));
	bzero(message_login,sizeof(message_login));
	read(sockfd, message_login, sizeof(message_login));
	if(get_controller(message_login) == 201){
		user_infor.status = 1;
		user_infor.id = atoi(get_first_param(message_login));
		strcpy(user_infor.userName,username);
		strcpy(user_infor.password,password);
		printf("\n\t%s\n\n",get_second_param(message_login));
		// Show_message(loginDialog,GTK_MESSAGE_INFO,"Thành công","đăng nhập thành công");
		gtk_widget_hide(loginDialog);
		Run_Game(sockfd_pt);
	} 
	else{
		printf("\n\t%s\n\n",get_first_param(message_login));
		Show_message(loginDialog,GTK_MESSAGE_ERROR,"Thất bại",get_first_param(message_login));
		return 0;
	}
}

int Register(GtkButton *button,gpointer userdata){
	int *sockfd_pt = (int *) userdata;
	int sockfd = *sockfd_pt;
	char username[30];
	char password[30];
	char message[MAX];
	char message_reg[MAX];
	bzero(username,sizeof(username));
	bzero(password,sizeof(password));
	strcpy(username,gtk_entry_get_text(GTK_ENTRY(userName)));
	strcpy(password,gtk_entry_get_text(GTK_ENTRY(passWord)));
	sprintf(message, "2 %s %s", username, password);
	write(sockfd, message, strlen(message));
	bzero(message_reg,sizeof(message_reg));
	read(sockfd, message_reg, sizeof(message_reg));
	if(get_controller(message_reg) == 201){
		user_infor.status = 1;
		user_infor.id = atoi(get_first_param(message_reg));
		strcpy(user_infor.userName,username);
		strcpy(user_infor.password,password);
		printf("\n\t%s\n\n",get_second_param(message_reg));
		gtk_widget_hide(loginDialog);
		Run_Game(sockfd_pt);
		return 1;
	} 
	else{
		printf("\n\t%s\n\n",get_first_param(message_reg));
		Show_message(loginDialog,GTK_MESSAGE_ERROR,"Thất bại",get_first_param(message_reg));
		return 0;
		return 0;
	}
}

int Logout(GtkButton *button,gpointer userdata){
	int *sockfd_pt = (int *) userdata;
	int sockfd = *sockfd_pt;
	char message[MAX];
	char message_logout[100];
	sprintf(message, "9 %d", user_infor.id);
	write(sockfd, message, strlen(message));
	bzero(message_logout,sizeof(message_logout));
	read(sockfd, message_logout, sizeof(message_logout));
	if(get_controller(message_logout) == 201){
		user_infor.id = 0;
		strcpy(user_infor.userName,"");
		strcpy(user_infor.password,"");
		printf("\n\t%s\n",get_first_param(message_logout));
		// Show_message(mainDialog,GTK_MESSAGE_INFO,"Thành công","đăng xuất thành công");
		gtk_widget_hide(mainDialog);
		Run_Program(sockfd_pt);
		return 1;
	}
	else{
		printf("\n\t%s\n",get_first_param(message_logout));
		Show_message(mainDialog,GTK_MESSAGE_ERROR,"Thất bại",get_first_param(message_logout));
		return 0;
	}
}

void read_map(int sockfd){
	char buff[MAX];
	while (1){
    	bzero(buff, sizeof(buff));
        // read the message from client and copy it in buffer
        // read(sockfd, buff, sizeof(buff));
		read(sockfd,buff,MAX);
		// clearString(buff);
		if(strcmp(buff, "end_of_file") == 0)
            break;
        printf("%s", buff);
    }
}

void Play_resume(int sockfd){
	show_array();
	char buffer[MAX];
	char message[MAX];
	char server_message[MAX];
	int n;
	if(room_infor.turn == 2 && room_infor.player_1_id != 0 ){
		printf("wait ... \n");										// read message
		read(sockfd,server_message,sizeof(server_message));
		clearString(server_message);
		run_game(server_message,2);
		room_infor.turn = 1;
		show_array();
	}
	if(room_infor.turn == 1 && room_infor.player_2_id != 0){
		printf("wait ... \n");										// read message
		read(sockfd,server_message,sizeof(server_message));
		clearString(server_message);
		run_game(server_message,1);
		room_infor.turn = 2;
		show_array();
	}
	 
	for (;;) {
		bzero(buffer, sizeof(buffer)); 
		printf("\nEnter the string : "); 
		n = 0; 
		__fpurge(stdin);
		while((buffer[n++] = getchar()) != '\n'){}
		sprintf(message,"7 %d %s",user_infor.id,buffer);
		write(sockfd, message, strlen(message));      				// send to server
		if(room_infor.player_1_id != 0 ){
			run_game(buffer,1);
			room_infor.turn = 2;
			show_array();
			printf("wait ... \n");										// read message
			read(sockfd,server_message,sizeof(server_message));
			clearString(server_message);
			run_game(server_message,2);
			room_infor.turn = 1;
			show_array();
		}
		else{
			run_game(buffer,2);
			room_infor.turn = 1;
			show_array();
			printf("wait ... \n");										// read message
			read(sockfd,server_message,sizeof(server_message));
			clearString(server_message);
			run_game(server_message,1);
			room_infor.turn = 2;
			show_array();
		}
	} 
}

int run_game(char *lenh,int player){
    int start_point ;
    int *array = room_infor.array;
    int next_index, second_next_index;
    if(lenh[0] == 'a') start_point = lenh[1] - 48;
    else start_point = lenh[1] - 48 + 6;
    // check dieu kien
    if( array[5] == 0 && array[11] == 0 ){                                         // end game
        return 2;                    
    }
    if( start_point == 5 || start_point == 11){                                    // Cham o quan
        return 1;
    }
    if(lenh[2]=='l'){
        next_index = (start_point + 12 -1) % 12;
        second_next_index = (start_point + 12 -2) % 12;
    }
    else{
        next_index = (start_point + 12 + 1) % 12;
        second_next_index = (start_point + 12 +2) % 12;
    }
    if(array[next_index] != 0 && array[start_point] == 0 ){              // an
        if(player == 1){
            room_infor.player_1_point += array[next_index];
            array[next_index] = 0;
            if(next_index == 5 ){
                room_infor.player_1_point += room_infor.player_1_king;
                room_infor.player_1_king = 0;
            }
            if(next_index == 11){
                room_infor.player_1_point += room_infor.player_2_king;
                room_infor.player_2_king = 0;
            }
        }
        else{       
            room_infor.player_2_point += array[next_index];
            array[next_index] = 0;
            if(next_index == 5 ){
                room_infor.player_2_point += room_infor.player_1_king;
                room_infor.player_1_king = 0;
            }
            if(next_index == 11){
                room_infor.player_2_point += room_infor.player_2_king;
                room_infor.player_2_king = 0;
            }
        }
        return 1;
    }
    if(array[next_index] == 0 && array[start_point]==0){                    // cach 2 o lien tiep
        return 1;
    }

    // run
    int j = -1;
    int num_loop = array[start_point];
    array[start_point] = 0;
    if(lenh[2]=='l'){
        j = (start_point + 12 - 1) % 12;
        for(int i = 0; i < num_loop ; i++){
            array[j] ++ ;
            j = (j + 12 -1) % 12;
            // printf("loop: j=%d\n",j);
			// gtk_label_set_text(GTK_LABEL(map),render_map());
			// sleep(1);
        }
    }
    else{
        j = (start_point + 12 + 1) % 12 ;
        for(int i = 0; i < num_loop ; i++){
            array[j] ++;
            j = (j + 12 + 1) % 12;
        }
    }
    // goi de quy
    char *new_lenh = (char*)malloc(MAX);
    new_lenh[3] ='\0';
    if(j <= 5){
        sprintf(new_lenh,"a%d%c",j,lenh[2]);
    }
    else{
        sprintf(new_lenh,"b%d%c",j-6,lenh[2]);
    }
    return run_game(new_lenh,player);
}

int resume(GtkButton *button, gpointer userdata){
	int *sockfd_pt = (int *) userdata;
	int sockfd = *sockfd_pt;
	char buffer[MAX];
	int id = user_infor.id;
	char message[30];
	sprintf(message,"6 %d",id);
	write(sockfd,message,sizeof(message));
	bzero(buffer,strlen(buffer));
	read(sockfd,buffer,sizeof(buffer));
	clearString(buffer);
	if(get_controller(buffer)==501){
		printf("\n\t%s\n",get_first_param(buffer));
		Show_message(playDialog,GTK_MESSAGE_ERROR,"thất bại",get_first_param(buffer));
		return 0;
	}
	else{
		set_textView_text(buffer);
		gtk_widget_hide(mainDialog);
		gtk_widget_show_all(listRoomDialog);
		return 1;
	}
}

void wait(int sockfd){
	printf("wait\n");
	pthread_t thread;
	gtk_label_set_text(GTK_LABEL(map),render_map());
	set_play_input_text("Wait ... ");
	gtk_widget_show_all(playDialog);
	int err = pthread_create(&thread,NULL,read_server,&sockfd);
}

void play_left(GtkButton *button, gpointer userdata){
	int *sockfd_pt = (int *) userdata;
	int sockfd = *sockfd_pt;
	char buffer[MAX];
	char message[MAX];
	bzero(buffer, sizeof(buffer)); 
	strcpy(buffer,gtk_entry_get_text(GTK_ENTRY(playInput)));
	int index_array = buffer[0] - 48;
	if(index_array < 0 || index_array > 4){
		Show_message(playDialog,GTK_MESSAGE_ERROR,"thất bại","Ô không tồn tại");
	}
	else{
		if(room_infor.player_1_id == 0 && room_infor.turn == 2){			// user2 turn 2 
			if(room_infor.array[index_array + 6] == 0){
				Show_message(playDialog,GTK_MESSAGE_ERROR,"thất bại","Không được chọn ô trống");
			}
			else{
				sprintf(buffer,"b%cl",buffer[0]);
				sprintf(message,"7 %d %s",user_infor.id,buffer);	
				write(sockfd, message, strlen(message));      					// send to server
				run_game(buffer,2);
				room_infor.turn = 1;
				wait(sockfd);
			}	
		}
		else if(room_infor.player_2_id == 0 && room_infor.turn == 1){		// user1 turn 1
			if(room_infor.array[index_array] == 0){
				Show_message(playDialog,GTK_MESSAGE_ERROR,"thất bại","Không được chọn ô trống");
			}
			else{
				sprintf(buffer,"a%cl",buffer[0]);
				sprintf(message,"7 %d %s",user_infor.id,buffer);	
				write(sockfd, message, strlen(message));      					// send to server
				run_game(buffer,1);
				room_infor.turn = 2;
				wait(sockfd);
			}
		}
		else{
		printf("Player_1_id = %d, Player_2_id = %d, turn = %d\n",room_infor.player_1_id,room_infor.player_2_id,room_infor.turn);								
		Show_message(playDialog,GTK_MESSAGE_ERROR,"thất bại","Chưa tới lượt bạn");
		}
	}
}

void play_right(GtkButton *button, gpointer userdata){
	int *sockfd_pt = (int *) userdata;
	int sockfd = *sockfd_pt;
	char buffer[MAX];
	char message[MAX];
	bzero(buffer, sizeof(buffer)); 
	strcpy(buffer,gtk_entry_get_text(GTK_ENTRY(playInput)));
	int index_array = buffer[0] - 48;
	if(index_array < 0 || index_array > 4){
		Show_message(playDialog,GTK_MESSAGE_ERROR,"thất bại","Ô không tồn tại");
	}
	else{
		if(room_infor.player_1_id == 0 && room_infor.turn == 2){			// user2 turn 2 
			if(room_infor.array[index_array + 6] == 0){
				Show_message(playDialog,GTK_MESSAGE_ERROR,"thất bại","Không được chọn ô trống");
			}
			else{
				sprintf(buffer,"b%cr",buffer[0]);
				sprintf(message,"7 %d %s",user_infor.id,buffer);	
				write(sockfd, message, strlen(message));      					// send to server
				run_game(buffer,2);
				room_infor.turn = 1;
				wait(sockfd);
			}		
		}
		else if(room_infor.player_2_id == 0 && room_infor.turn == 1){		// user1 turn 1   USER 1 PLAY
			if(room_infor.array[index_array ] == 0){
				Show_message(playDialog,GTK_MESSAGE_ERROR,"thất bại","Không được chọn ô trống");
			}
			else{
				sprintf(buffer,"b%cr",buffer[0]);
				sprintf(message,"7 %d %s",user_infor.id,buffer);	
				write(sockfd, message, strlen(message));      					// send to server
				run_game(buffer,1);
				room_infor.turn = 2;
				wait(sockfd);
			}	
		}
		else{																// sai turn
			printf("Player_1_id = %d, Player_2_id = %d, turn = %d\n",room_infor.player_1_id,room_infor.player_2_id,room_infor.turn);								
			Show_message(playDialog,GTK_MESSAGE_ERROR,"thất bại","Chưa tới lượt bạn");
		}
	}
}

void Play(int sockfd){
	printf("play\n");
	pthread_t thread;
	gtk_label_set_text(GTK_LABEL(map),render_map());
	gtk_widget_show_all(playDialog);
	int err = pthread_create(&thread,NULL,read_server,&sockfd);	
}

char *render_map(){
    char *map = (char*)malloc(MAX);
    if(room_infor.player_2_id != 0){
		sprintf(map,"     player2: %d      ---       player1: %d\n",room_infor.player_2_point,room_infor.player_1_point);
    	sprintf(map,"%s\n",map);
    	for(int i = 4;i >= 0;i--){
        	sprintf(map,"%s          %d",map,room_infor.array[i]);
    	}
   		sprintf(map,"%s\n\n%d                                                                   %d\n",map,room_infor.array[5],room_infor.array[11]);
    	sprintf(map,"%s\n",map);
    	for(int i = 6;i < 11;i++){
        	sprintf(map,"%s          %d",map,room_infor.array[i]);
    	}
    	sprintf(map,"%s\n__________________________________\n",map);
		sprintf(map,"%s          0          1          2          3          4",map);
	}
	else{
		sprintf(map,"     player1: %d      ---       player2: %d\n",room_infor.player_1_point,room_infor.player_2_point);
    	sprintf(map,"%s\n",map);
    	for(int i = 10; i > 5; i--){
        	sprintf(map,"%s          %d",map,room_infor.array[i]);
    	}
   		sprintf(map,"%s\n\n%d                                                                   %d\n",map,room_infor.array[11],room_infor.array[5]);
    	sprintf(map,"%s\n",map);
    	for(int i = 0 ;i < 5 ; i ++ ){
        	sprintf(map,"%s          %d",map,room_infor.array[i]);
    	}
    	sprintf(map,"%s\n__________________________________\n",map);
		sprintf(map,"%s          0          1          2          3          4",map);
	}
	return map;
}

void show_array(){
    printf("\n    *************************************************\n");
    printf("    *\t_______________O AN QUAN________________    *\n");
    printf("    *\t player1: %d      ---       player2: %d       *\n",room_infor.player_1_point,room_infor.player_2_point);
    printf("\n    *\t   a4     a3     a2     a1     a0           *");
    printf("\n    *\t");
    for(int i = 4;i >= 0;i--){
        printf("   %d   ",room_infor.array[i]);
    }
    printf("        *   ");
    printf("\n\n    *\t%d                                  %d        *\n",room_infor.array[5],room_infor.array[11]);
    printf("\n    *\t");
    for(int i = 6;i < 11;i++){
        printf("   %d   ",room_infor.array[i]);
    }
    printf("         *   ");
    printf("\n    *\t   b0     b1     b2     b3     b4           *");
    printf("\n    *************************************************\n\n");
}
