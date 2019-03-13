#include "Protocol.h"

Protocol::Protocol()
{ 
} // end Protocol()

bool Protocol::init_protocol(const char* ip,const int port)
{
	if(!this->m_csock.InitSocket(ip, port))
		return false;
	return true;
} // end init_protocol(const char* ip,const int port)

Protocol::~Protocol(void)
{
} // end ~Protocol(void)

void Protocol::init_board(int piece_count[14], char current_position[32], struct History& history, int &time)
{ 
	char *buffer= NULL; 
	if(this->m_csock.Recieve(&buffer)){
		if( strstr(buffer, "/start") != NULL){
			Start(&buffer[strlen("/start")+1], piece_count, current_position, history, time);	
		}
	}
	this->m_csock.Send("/start");
	if(buffer != NULL) free(buffer); buffer = NULL;
} // end init_board(int piece_count[14], int current_position[32], History& history, int &time)


void Protocol::Start(const char* state, int piece_count[], char current_position[], History& history,int &time)
{ 
	const char skind[] = {'-', 'K', 'G', 'M', 'R', 'N', 'C', 'P', 'X', 'k', 'g', 'm', 'r', 'n', 'c', 'p'};
	const int  darkPiece[14] = {1, 2, 2, 2, 2, 2, 5, 1, 2, 2, 2, 2, 2, 5};
	char *tmp = NULL, *token = NULL;  

	if(state != NULL){
		tmp = (char*)malloc(sizeof(char)*strlen(state)+1);
		strcpy(tmp, state);
		// initial board
		token = strtok(tmp, ",") ;
		current_position[28] = skind[atoi(token)];  
		for(int i = 1; i < 32; ++i){
			token = strtok(NULL, ",");
			current_position[(7-i/4)*4+i%4] = skind[atoi(token)]; 
		}	 
		// initial alive piece
		for(int i = 0; i < 14; ++i){
			token = strtok(NULL, ",");
			piece_count[i] = atoi(token); 
		}	 
	}else{
		for(int i = 0; i < 32; ++i) current_position[i] = 8; // DarkChess
		for(int i = 0; i < 14; ++i) piece_count[i] = darkPiece[i];
	} 
	// moveRecord
	token = strtok(NULL, ",");
	history.number_of_moves = atoi(token);  
	if(history.number_of_moves){
		history.move = (char**)malloc(sizeof(char*)*history.number_of_moves); 
		for(int i = 0; i < history.number_of_moves; ++i){
			history.move[i] = (char*)malloc(sizeof(char)*6);
			char num[3] = {0};
			int  src, dst;
			token = strtok(NULL, ",");
			num[0] = token[0], num[1] = token[1];
			src = atoi(num); 
			num[0] = token[3], num[1] = token[4];
			dst = atoi(num);  
			if(token[2] == '-') 
				sprintf(history.move[i], "%c%c-%c%c",'a'+(src%4), '1'+(src/4), 'a'+(dst%4), '1'+(dst/4));
			else
				sprintf(history.move[i], "%c%c(%c)",'a'+(src%4), '1'+(src/4), skind[dst]); 
			history.move[i][5] = '\0'; 
			//fprintf(stderr, "[%d.%s, %d-%d, %s]\n", i, history.move[i], src, dst, token); 
		} 
	} 
	//回合制時間
	token = strtok(NULL, ",");
	if(token != NULL) time = atoi(token);
	fprintf(stderr, "time: %d (ms)\n",time); 
} // end Start(const char* state, int piece_count[], char current_position[], History& history,int &time)

void Protocol::get_turn(bool &turn, PROTO_CLR &color)
{
	char *buffer = NULL;
	if(this->m_csock.Recieve(&buffer)){
		if( strstr(buffer, "/turn") !=NULL ){
			turn  = (buffer[6] == '0')?false:true;
			color = (buffer[8] == '0')?PCLR_RED:((buffer[8]=='1')?PCLR_BLACK:PCLR_UNKNOW);
		}
	} 
	this->m_csock.Send("/turn");
	if(buffer != NULL) free(buffer); buffer = NULL;
} // end get_turn(int &turn, int &color)

void Protocol::send(const char src[3], const char dst[3])
{ 
	char buffer[1 + BUFFER_SIZE] = {0}; 
	if(strcmp(src, dst)) 
		sprintf(buffer,"/move %d %d %d %d", src[0]-'a', src[1]-'1', dst[0]-'a', dst[1]-'1');
	else
		sprintf(buffer,"/flip %d %d", src[0]-'a', src[1]-'1'); 
	//fprintf(stderr, "send=%s\n", buffer);
	this->m_csock.Send(buffer);
} // end send(const char src[3], const char dst[3])

void Protocol::send(const char move[6])
{ 
	char buffer[1 + BUFFER_SIZE] = {0};  
	if(move[0] == move[3] && move[1] == move[4]) // Flip src = dst
		sprintf(buffer, "/flip %d %d", move[0]-'a', move[1]-'1');
	else
		sprintf(buffer, "/move %d %d %d %d", move[0]-'a', move[1]-'1', move[3]-'a', move[4]-'1');
	//fprintf(stderr, "send=%s\n", buffer);
	this->m_csock.Send(buffer);
} // end send(const char move[6])

void Protocol::recv(char move[6], int &time)
{
	static const char skind[]={'-','K','G','M','R','N','C','P','O','k','g','m','r','n','c','p'};
	char *buffer = NULL;
	int srcX, srcY, dstX, dstY, fin;
	if(this->m_csock.Recieve(&buffer)){
		//fprintf(stderr, "recv=%s\n", buffer);
		if(strstr(buffer, "/move") != NULL){
			sscanf(buffer,"%*s %d %d %d %d %d", &srcX, &srcY, &dstX, &dstY, &time); 
			sprintf(move,"%c%c-%c%c", 'a'+srcX, '1'+srcY, 'a'+dstX, '1'+dstY);
		}else if(strstr(buffer, "/flip") != NULL){
			sscanf(buffer, "%*s %d %d %d %d", &srcX, &srcY, &fin, &time); 
			sprintf(move,"%c%c(%c)", 'a'+srcX, '1'+srcY, skind[fin]); 
		}
		fprintf(stderr, "remaining time = %d ms\n",time);
	}  
	if(buffer != NULL) free(buffer); buffer = NULL;
} // end recv(char mov[6])

PROTO_CLR Protocol::get_color(const char move[6])
{
	static const char skind[]={'K', 'G', 'M', 'R', 'N', 'C', 'P', 'k', 'g', 'm', 'r', 'n', 'c', 'p'};
	if (move[2] != '(') {fprintf(stderr, "get_color error\n"); return PCLR_UNKNOW;}
	for (int i = 0; i < 14; ++i) {
		if (move[3] == skind[i])
			return (i<7)?PCLR_RED:PCLR_BLACK;
	}
	return PCLR_UNKNOW;
} // end get_color(char move[6])
