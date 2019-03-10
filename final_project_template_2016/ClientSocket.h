#ifndef CLIENTSOCKET_INCLUDED
#define CLIENTSOCKET_INCLUDED

#ifdef _WIN32
#include <cstdio>
#include "winsock2.h"

#pragma comment(lib,"Ws2_32.lib")
#else
    typedef int SOCKET;
	#include <sys/socket.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <errno.h>
    #include <cstdio>
    #include <cstring>
    #include <cstdlib>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define SOCKET_ERROR -1
    #define INVALID_SOCKET -1

#endif
#define BUFFER_SIZE 1024

class ClientSocket
{
private: 
    SOCKET m_Socket;
	void CloseSocket();
public:
	ClientSocket();
	ClientSocket(char *ip,int port);
	~ClientSocket(void);
	bool InitSocket(const char *ip, const int port);
	bool Recieve(char **recvbuf);
	bool Send(const char *sendbuf);
    void ShowErrorMsg(const char *msg);
};

#endif
