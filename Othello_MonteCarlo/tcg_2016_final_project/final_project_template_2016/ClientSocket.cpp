#include "ClientSocket.h"
#define CLIENT_SOCKET_MAX_BUFFER  1024
#define CLIENT_SOCKET_LEN_BUFFER  2

ClientSocket::ClientSocket( char* ip,  int port)
{
	while(!InitSocket(ip,port))
		ShowErrorMsg("protocol port init failed! \n") ; 
}

ClientSocket::ClientSocket()
{
}

ClientSocket::~ClientSocket(void)
{
	CloseSocket();
}

void ClientSocket::CloseSocket()
{
#ifdef _WIN32
     closesocket(this->m_Socket);
     WSACleanup();
#else
     close(m_Socket);
#endif
}


bool ClientSocket::InitSocket(const char*ip, const int port)
{
#ifdef _WIN32 
	SOCKADDR_IN servAddr;
	WSADATA wsaData ;
	int retVal;
	if (::WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) // minorVer:函式庫檔案的副版本(高位元組), majorVer:主版本(低位元組)
	{ 
		ShowErrorMsg("WSAStartup() failed. \n") ;
		return false ;
	}
	// create socket
	m_Socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(m_Socket == INVALID_SOCKET)
	{ 
		WSACleanup();
		ShowErrorMsg("Create socket failed\n") ;
		return false ;
	}
	// initial address
	servAddr.sin_family = AF_INET ;
	servAddr.sin_addr.s_addr = inet_addr(ip) ;
	servAddr.sin_port = htons(port) ;
	// connect to remote server
	retVal = ::connect(m_Socket, (LPSOCKADDR)&servAddr, sizeof(servAddr)) ;
	if (SOCKET_ERROR == retVal)
	{ 
		ShowErrorMsg("///// Connect Error.  please contact system administrator./////\n") ;
		return false ;
	} 
#else     
	struct sockaddr_in servAddr;
	m_Socket = socket(AF_INET , SOCK_STREAM , 0);
	if (m_Socket == -1)
	{
		ShowErrorMsg("Create socket failed\n") ;
		return false;
	} 
	// initial address
	servAddr.sin_addr.s_addr = inet_addr(ip);
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(port);
	//Connect to remote server
	if (connect(m_Socket , (struct sockaddr *)&servAddr , sizeof(servAddr)) < 0)
	{
		ShowErrorMsg("///// Connect Error.  please contact system administrator./////\n") ;
		return false;
	} 
#endif
	return true;

}

void ClientSocket::ShowErrorMsg(const char *msg)
{
	fprintf(stderr, "\n#ERROR. %s\n", msg);
#ifdef _WIN32
	int nErrCode = WSAGetLastError(); 
	HLOCAL hlocal = NULL;
	// error string
	BOOL fOk = FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, nErrCode, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
		(PTSTR)&hlocal, 0, NULL);

	// show error message
	if (hlocal != NULL) LocalFree(hlocal); 
#endif
} // end ShowErrorMsg()

bool ClientSocket::Recieve(char **recvbuf)
{
	int  bytesRecv = 0;
	int  lastLen   = 0;
	int  index     = 0;
	char lenBuffer[1 + CLIENT_SOCKET_LEN_BUFFER] = {0}; 
	char tmpBuffer[1 + CLIENT_SOCKET_MAX_BUFFER] = {0};
	char *packet = NULL; 
	bytesRecv = recv(this->m_Socket, lenBuffer, CLIENT_SOCKET_LEN_BUFFER, 0);  
	lenBuffer[CLIENT_SOCKET_LEN_BUFFER]='\0'; 
	if(bytesRecv < 2)
	{
		if(bytesRecv == 0)
			ShowErrorMsg("Disconnect to server.\n"); 
		ShowErrorMsg("Receive error(lastBuffer).\n"); 
		return false;
	}
	else
	{
		const int totalLen = lastLen =  (lenBuffer[1] << 7) | lenBuffer[0]; 
		packet = (char*)malloc(sizeof(char)*(1 + totalLen)); 
		index = 0;
		*recvbuf = packet;
		while(lastLen > 0)
		{    
			bytesRecv = recv(this->m_Socket, tmpBuffer, lastLen>=CLIENT_SOCKET_MAX_BUFFER?CLIENT_SOCKET_MAX_BUFFER:lastLen, 0); 
			if(bytesRecv < 0){ 
				ShowErrorMsg("Receive error(lastBuffer).\n"); 
				return false;
			}
			//puts(tmpBuffer);
			for(int i = 0; i < bytesRecv; ++i) packet[index++] = tmpBuffer[i];
			lastLen -= bytesRecv;
		} 
		packet[totalLen]='\0';
	}  
	return true;
} // end Recieve(char **recvbuf)
 
bool ClientSocket::Send(const char* sendbuf)
{
	const int sendbufLen = strlen(sendbuf);
	const int totalLen   = strlen(sendbuf) + 2;
	int bytesSend = 0;
	char *packet = (char*)malloc(sizeof(char)*(1 + totalLen));   
	packet[0] = sendbufLen % 128 ;
	packet[1] = sendbufLen / 128 ;
	for (int i = 2; i < totalLen; ++i) packet[i] = sendbuf[i - 2] ; 
	packet[totalLen] = '\0';  
	bytesSend = send(this->m_Socket, packet, totalLen, 0);
	if(packet != NULL) free(packet); packet = NULL;
	if(bytesSend < totalLen)
	{ 
		ShowErrorMsg("Send error(lastBuffer).\n"); 
		return false;
	}
	return true;
} // end Send(const char* sendbuf)
