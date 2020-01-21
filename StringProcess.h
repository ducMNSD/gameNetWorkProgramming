#include<stdio.h>
#include<string.h>
#include<stdlib.h>

void clearString(char *message){
    int i = strlen(message) - 1;
    for(i; i > 0 ; i--){
        if(message[i] > 126 || message[i] < 33){
            message[i] = '\0';
        }
        else{
            break;
        }
    }
}

int get_controller(char *message){
    char tmp[10];
    int i=0;
    for(i;message[i]!=' ';i++){
        tmp[i] = message[i];
    }
    tmp[i] = '\0';
    return atoi(tmp);
}

char* get_first_param(char *message){
    char *tmp = (char*)malloc(30*sizeof(char));
    int index = 0;
    for(index; message[index]!= ' '; index++){

    }
    index++;
    int i = index;
    for(i; message[i] != ' ' && message[i] != '\0'; i++){
        tmp[i-index] = message[i];
    }
    tmp[i-index] ='\0';
    return tmp;
}

char* get_second_param(char *message){
    int index_end_space = strlen(message);
    int i = strlen(message) - 1;
    for(i; message[i] != ' ' ; i--){
    }
    index_end_space = i + 1;
    char *tmp = (char*)malloc(30*sizeof(char));
    i = index_end_space;
    for(i;i<strlen(message);i++){
        tmp[i - index_end_space] = message[i];    
    }
    tmp[i-index_end_space] = '\0';
    return tmp;
}
