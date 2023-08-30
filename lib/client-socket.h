/**
 * File: client-socket.h
 * ---------------------
 * Presents the interface of the routine network
 * clients can use to connect to a specific port of
 * a remote host.
 * 这是创建客户端套接字的类
 */

#ifndef _client_socket_
#define _client_socket_

#include <string>

/**
 * Constant: kClientSocketError
 * ----------------------------
 * Sentinel used to communicate that a connection
 * to a remote server could not be made.
 */

const int kClientSocketError = -1;

/**
 * Function: createClientSocket
 * ----------------------------
 * Establishes a connection to the provided host
 * on the specified port, and returns a bidirectional 
 * socket descriptor that can be used for two-way
 * communication with the service running on the
 * identified host's port.
 */
// 一个表示地址，一个表示端口号
int createClientSocket(const std::string& host, 
		       unsigned short port);

#endif

