#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include "StringProcess.h"
#include "object.h"

#define MAX 1000
#define PORT 3000

int sockets_list[MAX][MAX];
user user_list[MAX];
room room_list[MAX];
int count_user = 0;

void init();

// HAM CHINH
int login(char *username,char *password,int socket);
int logout(int userId,int socket);
int Register(char *userName,char *password,int socket);
void CreateRoom(int userId,int idSock);
void getListRoom(int idSock);
void pickRoom(int roomId,int userId,int idSock);
void play(int userId,char *message,int idSock);
void out(int socket_index,int sockfd);
int run_game(char *lenh,int player,int room_index);
void resume(int userId,int sockfd);

// HAM PHU TRO
int countLine(char *file_path);
void openFile();
void writeFile();
int get_user_by_id(int userId);
int get_room_by_id(int roomId);
int get_socket_by_id(int userId);
int find_socket_index(int sockfd);
void render_map(int index);
void send_map(int index,int socket);
void show_array(int index);
//main
int main(int argc , char *argv[]){   
    init();
    openFile();
    // __fpurge(stdin);
    int server_socket, addrlen, new_socket;
    int max_sd;
    struct sockaddr_in address;
    //set of socket descriptors
    fd_set readfds;
    char buffer[MAX];
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(atoi(argv[1]));
    if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", atoi(argv[1]));
    //try to specify maximum of 10 pending connections for the master socket
    if (listen(server_socket, 80) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    puts("Waiting for connections ... ");
    while (1){
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        max_sd = server_socket;

        for (int i = 0; i < MAX; i++){
            int sd = sockets_list[i][0]; //socket descriptor
            if (sd > 0)
                FD_SET(sd, &readfds); //if valid socket descriptor then add to read list
            if (sd > max_sd)
                max_sd = sd; //highest file descriptor number, need it for the select function
        }
        printf("Wait ... \n");
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)){
            printf("select error");
        }

        //new connect
        if (FD_ISSET(server_socket, &readfds)){
            int len = sizeof(address);
            if ((new_socket = accept(server_socket,(struct sockaddr *)&address, &len)) < 0){
                perror("accept");
                exit(EXIT_FAILURE);
            }
            //inform user of socket number - used in send and receive commands
            printf("New connection, socket fd is %d, port: %d \n", new_socket, ntohs(address.sin_port));
            //add new socket to array of sockets
            for (int i = 0; i < MAX; i++){
                //if position is empty
                if (sockets_list[i][0] == 0){
                    sockets_list[i][0] = new_socket;
                    break;
                }
            }
        }

        //else its some IO operation on some other socket
        else{
            int current_socket = 0;
            int index_currrent_socket = -1;
            // get client index
            for (int i = 0; i < MAX; i++){
                if (FD_ISSET(sockets_list[i][0], &readfds)){
                    current_socket = sockets_list[i][0];
                    index_currrent_socket = i;
                    break;
                }
            }
            // some one exit
            bzero(buffer,sizeof(buffer));
            if (read(current_socket, buffer, sizeof(buffer)) == 0){
                close(current_socket);
                sockets_list[index_currrent_socket][0] = 0;
                out(index_currrent_socket,current_socket);
            }
            // processing message
            else{
                printf("buffer: %s -- socket: %d\n",buffer,current_socket);
                clearString(buffer);
                if(strlen(buffer)!=0){
                int control = get_controller(buffer);
                switch (control){
                case 1:
                    printf("\tLogin\n");
                    login(get_first_param(buffer),get_second_param(buffer),current_socket);
                    break;
                case 2:
                    printf("\tRegister\n");
                    Register(get_first_param(buffer),get_second_param(buffer),current_socket);
                    break;
                case 3:
                    printf("\tCreate New Room\n");
                    CreateRoom(atoi(get_first_param(buffer)),current_socket);
                    break;
                case 4:
                    printf("\tGet List Room\n");
                    getListRoom(current_socket);
                    break;
                case 5:
                    printf("\tPick Room\n");
                    int userId = atoi(get_first_param(buffer));
                    int roomId = atoi(get_second_param(buffer));
                    pickRoom(roomId,userId,current_socket);
                    break;
                case 6:
                    printf("\tResume\n");
                    resume(atoi(get_first_param(buffer)),current_socket);
                    break;
                case 7:
                    printf("\tPlay\n");
                    play(atoi(get_first_param(buffer)),get_second_param(buffer),current_socket);
                    break;
                case 8:
                    printf("\tOut\n");
                    break;
                case 9:
                    printf("\tLogout\n");
                    logout(atoi(get_first_param(buffer)),current_socket);
                    break;
                }
                }
            }
        }
    }
    return 0;
}

// =====================================================================================================================//

void init(){
    for (int i = 0; i < MAX; i++)
    {
        sockets_list[i][0] = 0;
        sockets_list[i][1] = 1;
    }
    for (int i =0; i < MAX; i++){
        user_list[i].id = 0 ;
        user_list[i].status = 0;
        user_list[i].roomId = 0;
        strcpy(user_list[i].password,"");
        strcpy(user_list[i].userName,"");
    }
    for(int i = 0 ; i < MAX ; i++){
        room_list[i].turn = 1;
        room_list[i].id = i + 1;
        room_list[i].player_1_id = 0;
        room_list[i].player_2_id = 0;
        room_list[i].status = 0;
        room_list[i].player_1_result = 0;
        room_list[i].player_2_result = 0;
        room_list[i].player_1_point = 0;
        room_list[i].player_2_point = 0;
        room_list[i].player_1_king = 4;
        room_list[i].player_2_king = 4;
        for(int j=0; j < 12; j++){
            if( j == 5 || j == 11 ){
                room_list[i].array[j] = 1;
            }
            else
            {
                room_list[i].array[j] = 5;
            }
            
        }
    }
}

void CreateRoom(int userId,int idSock){
    int i;
    char success_message[MAX];
    char fail_message[] = "501 Create_Room_Failed";
    for(i=0; i<MAX;i++){
        if( room_list[i].status == 0) {
            room_list[i].status = 1;
            room_list[i].player_1_id = userId;
            sprintf(success_message,"201 %d Create_Room_Success",room_list[i].id);
            write(idSock,success_message,strlen(success_message));
            break;
        }
    }
    if(i == MAX) {
        write(idSock,fail_message,strlen(fail_message));
    }
}

void getListRoom(int idSock){
    char str[MAX];
    bzero(str,strlen(str));
    strcpy(str,"");
    char room_empty[] = "501 Not_found_available_room\n";
    int count = 0;
    for (int i = 0; i < MAX; i++)
    {
        if(room_list[i].status == 1 ) {
            sprintf(str,"%s\tRoom_Id: %d\n",str,room_list[i].id);
            count++;
        }
    }
    if(count == 0) write(idSock,room_empty,sizeof(room_empty));
    else{
        str[strlen(str)] = '\0';
        write(idSock,str,strlen(str));
    }
}

void pickRoom(int roomId,int userId,int idSock){
    char message[MAX];
    bzero(message,sizeof(message));
    char *map;
    char error_message[] = "501 Room_was_full\n";
    if(room_list[get_room_by_id(roomId)].status == 3){
        room_list[get_room_by_id(roomId)].status = 2;
        // update phia client
        sprintf(message,"%s202 ",message);
        for(int i=0;i<12;i++){
            sprintf(message,"%s%d-",message,room_list[get_room_by_id(roomId)].array[i]);
        }
        if(room_list[get_room_by_id(roomId)].player_1_id == userId){
            sprintf(message,"%s %d-%d-%d-%d-%d-%s",message,room_list[get_room_by_id(roomId)].turn,
            room_list[get_room_by_id(roomId)].player_1_king,room_list[get_room_by_id(roomId)].player_2_king,
            room_list[get_room_by_id(roomId)].player_1_point,room_list[get_room_by_id(roomId)].player_2_point,"player1");
        }
        else{
            sprintf(message,"%s %d-%d-%d-%d-%d-%s",message,room_list[get_room_by_id(roomId)].turn,
            room_list[get_room_by_id(roomId)].player_1_king,room_list[get_room_by_id(roomId)].player_2_king,
            room_list[get_room_by_id(roomId)].player_1_point,room_list[get_room_by_id(roomId)].player_2_point,"player2");
        }
        write(idSock,message,strlen(message));
    }
    else if(room_list[get_room_by_id(roomId)].status == 1){    
        user user1 = user_list[get_user_by_id(userId)];    
        strcpy(message,"201 Join_Room\n");
        room_list[get_room_by_id(roomId)].status = 2;
        room_list[get_room_by_id(roomId)].player_2_id = userId;
        write(idSock,message,strlen(message));
        // send to user 1
        // printf("\tSocket: %d\n",get_socket_by_id(room_list[get_room_by_id(roomId)].player_1_id));
        int user1_socket = get_socket_by_id(room_list[get_room_by_id(roomId)].player_1_id);
        write(user1_socket,"START",strlen("START"));
    }
    else if(room_list[get_room_by_id(roomId)].status == 2){
        write(idSock,error_message,strlen(error_message));
    } 
    else if(room_list[get_room_by_id(roomId)].status == 4 ){
        write(idSock,"501 ROOM_DA_BI_KHOA",strlen("501 ROOM_DA_BI_KHOA"));
    }
    else if(room_list[get_room_by_id(roomId)].status == 0){
        write(idSock,"501 ROOM_CHUA_TON_TAI",strlen("501 ROOM_CHUA_TON_TAI"));
    }
    
}

void openFile(){
    // printf("open file\n");
    char *userName;
    char *password;
    int status;
    int id;
    int roomId;
    int line = countLine("account.txt");
    printf("Line: %d\n",line);
    FILE *fptr;
    if((fptr=fopen("account.txt","r"))==NULL){
        printf("Khong tim thay %s\n","account.txt");
        return;
    }
    for(int i=0; i < line;i++){
        char userName[MAX];
        char password[MAX];
        fscanf(fptr,"%d %s %s",&id,userName,password);
        user_list[count_user].id = id;
        strcpy(user_list[count_user].userName, userName);
        strcpy(user_list[count_user].password, password);
        count_user ++;
    }
    free(userName); 
    free(password);
    fclose(fptr);
}

void writeFile(){
    FILE *fptr;
    fptr=fopen("account.txt","w+");
    for (int i = 0; i < count_user; i++){
        fprintf(fptr, "%d %s %s\n", user_list[i].id, user_list[i].userName, user_list[i].password);
    }
    fclose(fptr);
}

int login(char *userName,char *password,int socket){
    char success_message[MAX];
    for (int i = 0; i < count_user; ++i){
        // printf("user_list: %s, userName: %s\n",user_list[i].userName,userName);
        if (strcmp(user_list[i].userName,userName) == 0){
            if(strcmp(user_list[i].password,password) == 0){
                if(user_list[i].status == 0){
                    user_list[i].status = 1;
                    sprintf(success_message,"201 %d Dang_nhap_thanh_cong",user_list[i].id); 
                    write(socket, success_message, strlen(success_message));  
                    int index = find_socket_index(socket);    // update id in socket list
                    sockets_list[index][1] = user_list[i].id;
                    printf("Dang nhap thanh cong\n");
                    return 1;
                }
                else
                {
                    char fail_message[] = "501 Tai_khoan_da_online\n";
                    write(socket, fail_message, strlen(fail_message));  
                }
                
            }
            else{
                char message[] = "501 Sai_mat_khau\n";
                write(socket, message, sizeof(message));
                printf("Dang nhap that bai\n");
                return 0;
            }
        }
    }
    char message[] = "501 Tai_khoan_khong_ton_tai\n";
    write(socket, message, sizeof(message));
    printf("Dang nhap that bai\n");
    return 0;
}

int logout(int userId,int socket){
    printf("logout userId: %d\n",userId);
    for (int i = 0; i < count_user; ++i){
        if(user_list[i].id == userId){
            user_list[i].status = 0;
            char message[] = "201 Dang_xuat_thanh_cong\n";
            write(socket, message, sizeof(message));
            printf("Dang xuat thanh cong\n");
            return 1;
        }
    }
    char message[] = "501 Dang_xuat_that_bai\n";
    write(socket, message, sizeof(message));
    printf("Dang xuat that bai\n");
    return 0;
        
}

int Register(char *userName,char *password,int socket){
    char success_message[MAX];
    for (int i = 0; i < count_user; ++i){
        if (strcmp(user_list[i].userName,userName) == 0){
            char message[] = "501 Tai_khoan_da_ton_tai\n";
            write(socket, message, sizeof(message));
            printf("Dang ky khong thanh cong\n");
            return 0;
        }
    }
    user_list[count_user].status = 1;
    user_list[count_user].id = user_list[count_user-1].id + 1;
    strcpy(user_list[count_user].userName, userName);
    strcpy(user_list[count_user].password, password);
    user_list[count_user].roomId = 0;
    sprintf(success_message,"201 %d Dang_ky_thanh_cong",user_list[count_user].id);
    write(socket, success_message, strlen(success_message));
    int index = find_socket_index(user_list[count_user].id);    // update id in socket list
    sockets_list[index][1] = user_list[count_user].id;
    printf("Dang ky thanh cong\n");
    count_user ++;
    writeFile();
    return 1;
}

// return index of userId
int get_user_by_id(int userId){
    for(int i = 0; i < count_user; i++){
        if(user_list[i].id == userId){
            return i;
        }
    }
    return -1;
}

// return index of roomId
int get_room_by_id(int roomId){
    for(int i=0; i < MAX ;i++){
        if(room_list[i].id == roomId){
            return i;
        }
    }
    return -1;
}

// return socket of userId
int get_socket_by_id(int userId){
    for(int i=0;i<MAX;i++){
        if(sockets_list[i][0] != 0 && sockets_list[i][1] == userId){
            return sockets_list[i][0];
        }
    }
    return -1;
}

// return index of sockfd
int find_socket_index(int sockfd){
    for(int i=0; sockets_list[i][0] != 0; i++){
        if(sockets_list[i][0] == sockfd) return i;
    }
    return 0;
}

int countLine(char *file_path){
    FILE *file;
    file = fopen(file_path, "r");
    int n = 0;
    char s;
    do
    {
        s = fgetc(file);
        if (s == '\n')
            n++;
    } while (s != EOF);
    // fclose(file);
    return n;
}

int find_room_by_player1(int userId){
    for(int i=0; i < MAX ;i++){
        // if(room_list[i].status == 2 && room_list[i].player_1_id == userId ){
        if(room_list[i].status != 4 && room_list[i].player_1_id == userId ){
            return i;
        }
    }
    return -1;
}   

int find_room_by_player2(int userId){
    for(int i=0; i < MAX ;i++){
        // if(room_list[i].status == 2 && room_list[i].player_2_id == userId ){
        if(room_list[i].status != 4 && room_list[i].player_2_id == userId ){
            return i;
        }
    }
    return -1;
}

void play(int userId,char *lenh,int idSock){
    char *message;
    int index = find_room_by_player1(userId);
    if(index != -1){
        run_game(lenh,1,index);
        room_list[index].turn = 2;
        // show_array(index);
        // write(idSock,"201 success",strlen("201 success"));
        int socket = get_socket_by_id(room_list[index].player_2_id);
        write(socket,lenh,strlen(lenh));
    }
    else{
        index = find_room_by_player2(userId);
        if(index != -1){
            run_game(lenh,2,index);
            room_list[index].turn = 1;
            // show_array(index);
            // write(idSock,"201 success",strlen("201 success"));
            int socket = get_socket_by_id(room_list[index].player_1_id);        
            write(socket,lenh,strlen(lenh));    
        }
        else{
            // write(idSock,"501 fail",strlen("501 fail"));
        }
        
    }

}

void out(int index_socket,int sockfd){
    printf("\tOUT\n");
    int user_id = sockets_list[index_socket][1];
    sockets_list[index_socket][1] = 0;
    int room_index = find_room_by_player1(user_id);
    if(room_index != -1){
        if(room_list[room_index].status == 1 || room_list[room_index].status == 3 ){
            room_list[room_index].status = 4;
        }
        if(room_list[room_index].status == 2){
            room_list[room_index].status = 3;
        }
    }
    else{
        room_index = find_room_by_player2(user_id);
        if(room_index != -1){
            if(room_list[room_index].status == 1 || room_list[room_index].status == 3 ){
                room_list[room_index].status = 4;
            }
            if(room_list[room_index].status == 2){
                room_list[room_index].status = 3;
            }
        }
    }
    int index_user = get_user_by_id(user_id);
    user_list[index_user].status = 0;
}

int run_game(char *lenh,int player,int room_index){
    int start_point ;
    int *array = room_list[room_index].array;
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
            room_list[room_index].player_1_point += array[next_index];
            array[next_index] = 0;
            if(next_index == 5 ){
                room_list[room_index].player_1_point += room_list[room_index].player_1_king;
                room_list[room_index].player_1_king = 0;
            }
            if(next_index == 11){
                room_list[room_index].player_1_point += room_list[room_index].player_2_king;
                room_list[room_index].player_2_king = 0;
            }
        }
        else{       
            room_list[room_index].player_2_point += array[next_index];
            array[next_index] = 0;
            if(next_index == 5 ){
                room_list[room_index].player_2_point += room_list[room_index].player_1_king;
                room_list[room_index].player_1_king = 0;
            }
            if(next_index == 11){
                room_list[room_index].player_2_point += room_list[room_index].player_2_king;
                room_list[room_index].player_2_king = 0;
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
    return run_game(new_lenh,player,room_index);
}

void show_array(int index){
    printf("\n    *************************************************\n");
    printf("    *\t_______________O AN QUAN________________    *\n");
    printf("    *\t player1: %d      ---       player2: %d       *\n",room_list[0].player_1_point,room_list[0].player_2_point);
    printf("\n    *\t   a4     a3     a2     a1     a0           *");
    printf("\n    *\t");
    for(int i = 4;i >= 0;i--){
        printf("   %d   ",room_list[index].array[i]);
    }
    printf("        *   ");
    printf("\n\n    *\t%d                                  %d        *\n",room_list[index].array[5],room_list[index].array[11]);
    printf("\n    *\t");
    for(int i = 6;i < 11;i++){
        printf("   %d   ",room_list[index].array[i]);
    }
    printf("         *   ");
    printf("\n    *\t   b0     b1     b2     b3     b4           *");
    printf("\n    *************************************************\n\n");
}

void send_map(int index,int sockfd){
    char file_name[MAX];
    sprintf(file_name,"%d.txt",index);
    FILE *file;
    file = fopen(file_name,"r");
    char FileContent[MAX];
    if (file == NULL)
        printf("File khong ton tai\n");
    else{
        int numOfLine = countLine(file_name);
        for (int i = 0; i < numOfLine; i++){
            bzero(FileContent, sizeof(FileContent));
            fgets(FileContent,80,file);
            write(sockfd, FileContent, sizeof(FileContent));
        }
        write(sockfd,"end_of_file", strlen("end_of_file"));
    }
    
}

void resume(int userId,int sockfd){
    int count = 0;
    char message[MAX];
    char error_message[] = "501 KHONG_CON_PHONG_CHO";
    for(int i=0;i<MAX;i++){
        if(room_list[i].status == 3){
            if(room_list[i].player_1_id == userId){
                sprintf(message,"Room_ID: %d player1\n",room_list[i].id);
                count ++;
            }
            else if(room_list[i].player_2_id == userId){
                sprintf(message,"Room_ID: %d player2\n",room_list[i].id);
                count ++;
            }
        }
    }
    if(count == 0){
        write(sockfd,error_message,strlen(error_message));
    }
    else{
        write(sockfd,message,strlen(message));
    }
}