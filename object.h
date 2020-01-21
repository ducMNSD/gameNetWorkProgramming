typedef struct user{
    int status;			
    int id; 			    
	char userName[30];		
	char password[30];              
	int roomId;                            
} user;

typedef struct room{
    int status;  
    int id;   
    int player_1_id;   
    int player_2_id;
    int turn;
    int array[12];
    int player_1_king;
    int player_2_king;
    int player_1_result;
    int player_2_result;
    int player_1_point;
    int player_2_point; 
}room;


