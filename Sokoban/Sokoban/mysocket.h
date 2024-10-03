/*
 * Simplified Endpoint C++ Socket Library
 * This source code downloaded from https://sourceforge.net/projects/endpoint/
 * runs well on Unix, but raise runtime error on Windows, so I edited
 * this simplified version for my class.
 *
 * $Log: mysocket.h,v $
 * Revision 1.2  2021/11/22 03:08:34  solomon
 * Add a constructor for Endpoint ep(TCP | CLIENT, "10.0.0.1:5000")
 *
 * Revision 1.1  2021/11/14 13:40:08  solomon
 * Initial revision
 *
 *
 */

#ifndef MYSOCKET_H
#define MYSOCKET_H

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#ifdef _WIN32
#include <winsock2.h>
#include <sys/types.h>	// struct addrinfo
#include <ws2tcpip.h>	// getaddrinfo()
#define LastError       WSAGetLastError()
#pragma comment (lib, "Ws2_32.lib")
#else // !_WIN32
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef unsigned int UINT_PTR, *PUINT_PTR;
typedef UINT_PTR        SOCKET;
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#define LastError       errno
#endif // !_WIN32
#include "protocols.h"
using std::string;


class EndpointAddress : public addrinfo {
public:
    EndpointAddress();
    EndpointAddress(string hostname, int service);
    EndpointAddress(string hostname, string service);
    bool Create(string hostname, string service); 
    std::string IP(); // Return literal IP address
    ~EndpointAddress();

    // TODO: if you store data in addrinfo,
    // you don't need an extra data member m_result
    struct addrinfo *m_result;
};



class Endpoint {
public:
	Endpoint(int type, std::string hostname, string service);
	Endpoint(int type, string address);

	bool Initialize();

	// bool Create(int type, std::string host, std::string service) { };	
	bool Create(int type, std::string host, string service);

	bool Create_TCP_SERVER() ;

	bool Create_TCP_CLIENT() ;

	int Send(const void *msg, int len, int flags);

	// Write all bytes in data
	// Returns number of bytes written
	int Write(std::string data);

	int Recv(void *buf, int len, unsigned int flags);

	// Read bytes_to_read bytes into data, return FALSE if end-of-file
	// bytes_to_read=x can be either:
	// x>0 - read AT LEAST x bytes, blocking
	// x<0 - read AT MOST -x bytes, blocking
	// x=0 - Non-blocking I/O (not supported)
	// (This is copied almost verbatim from my own IPv4-only Socket)
	int Read(int bytes_to_read, std::string& data);

	bool Close();
	bool CloseServer();
	~Endpoint();

	int m_sockfd;	// client Socket
	int m_servfd;	// listening Socket
	int m_type;		// TCP or UDP
	int m_server;	// CLIENT or SERVER
	bool m_bool;	// Socket created successfully or not
	static bool g_initialized;
	EndpointAddress m_local;
	EndpointAddress m_remote;
};
#endif MYSOCKET_H