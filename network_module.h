#ifndef NETWORK_MODULE_H
#define NETWORK_MODULE_H

#include "network.h"

int init_connection_module(const char *address, const int port);
void end_connection_module(int sock);

int read_server(SOCKET sock, char *buffer);
void write_server(SOCKET sock, const char *buffer);
void resetDescriptor(SOCKET *sock, fd_set *rdfs);

#endif // NETWORK_MODULE
