#ifndef PROTOCOL_INCLUDED
#define PROTOCOL_INCLUDED
#include "ClientSocket.h"

struct History{
	char** move;
	int number_of_moves;
	History(){move = NULL, number_of_moves = 0;}
	~History(){
		if(number_of_moves != 0) {
			for(int i = 0; i < number_of_moves; i++) {
				if(move[i]!=NULL)
					free(move[i]);
			}
			free(move);
		}
	}
};

enum PROTO_CLR{PCLR_RED, PCLR_BLACK, PCLR_UNKNOW};

class Protocol
{
private:
	ClientSocket m_csock;
	void Start(const char *sState, int piece_count[14], char current_position[32], struct History &history,int &time);
public:
	Protocol();
	~Protocol(); 
	bool init_protocol(const char *ip, const int port);
	void init_board(int piece_count[14], char current_position[32], struct History &history,int &time);
	void get_turn(bool &turn, PROTO_CLR &color); 
	void send(const char src[3], const char dst[3]);
	void send(const char move[6]);
	void recv(char move[6],int &time);
	PROTO_CLR get_color(const char move[6]);
};

#endif
