#ifndef NETWORK_H
#define NETWORK_H


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> // close
#include <netdb.h>  // gethostbyname
#include <fcntl.h>

//not really usefull, or in the case of portability with windows
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)

//Some typedef in order not to write struct & "SOCKET" is more explicit than "int"
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#define CRLF	 	"\r\n"		//is used somewhere?
#define BUF_SIZE 	1024		//need some tests, we don't really know what is the real size
#define MAX_CLIENTS	100

#endif //NETWORK_H
