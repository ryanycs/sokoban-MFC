/*
 * Simplified Endpoint C++ Socket Library
 * $Log: mysocket.cpp,v $
 * Revision 1.7  2022/05/18 14:42:17  solomon
 * Replace sprintf() with sprintf_s().
 *
 * Revision 1.6  2021/11/28 04:09:56  solomon
 * Temporarily disable L.93 freeaddrinfo(m_result) to avoid crash.
 *
 * Revision 1.5  2021/11/22 03:07:35  solomon
 * Add a constructor for Endpoint ep(TCP | CLIENT, "10.0.0.1:5000")
 *
 * Revision 1.4  2021/11/22 02:03:01  solomon
 * Check #ifdef _WIN32, but this file does not work well on Unix yet.
 *
 * Revision 1.3  2021/11/14 13:39:28  solomon
 * Split the class declaration and implementation to .h and .cpp
 *
 * Revision 1.2  2021/11/14 12:32:05  solomon
 * Add Create_TCP_SERVER()
 *
 */

// #define _CRT_SECURE_NO_WARNINGS


#include <iostream>
#include <string>
/*
#include <sys/types.h>	// struct addrinfo
#include <winsock2.h>
#include <ws2tcpip.h>	// getaddrinfo()
#pragma comment (lib, "Ws2_32.lib")
*/
#include "protocols.h"
#include "mysocket.h"
#define DEFAULT_PORT    5000
using std::string;



EndpointAddress::EndpointAddress() { m_result = NULL;  }
EndpointAddress::EndpointAddress(string hostname, int port) {
    char service[6];
    sprintf_s(service, 6, "%d", port);
    Create(hostname, service);
}
EndpointAddress::EndpointAddress(string hostname, string service) {
    Create(hostname, service);
}


bool EndpointAddress::Create(string hostname, string service) {
		struct addrinfo hints;
#ifdef _WIN32
		ZeroMemory(&hints, sizeof(hints));
#else
                memset(&hints, 0, sizeof(hints));
#endif
		// hints.ai_family = AF_UNSPEC;
		hints.ai_family = AF_INET;      // IPv4
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		// Resolve the server address and port
		int iResult = getaddrinfo( hostname.c_str(), service.c_str(), &hints, &m_result);
		if (iResult != 0) {
			printf("getaddrinfo failed with error: %d\n", iResult);			
			return false;
		}
		return true;
	}

// Return literal IP address
std::string EndpointAddress::IP() {
		char str[128];
		addrinfo* addr = this->m_result;

		/*if (!m_bool)
			return "???";*/

		memset(&str, 0, 128);
		// This is nasty, but necessary
		// From, I think, W. Richard Steven's work
		if (addr->ai_family == AF_INET) {
			inet_ntop(AF_INET, &(((sockaddr_in*)addr->ai_addr)->sin_addr),
				str, sizeof(str));
		}
		else if (addr->ai_family == AF_INET6) {
			inet_ntop(AF_INET6, &(((sockaddr_in6*)addr->ai_addr)->sin6_addr),
				str, sizeof(str));
		}
		else {
			return "";
		}

		return std::string(str);
	}
	
EndpointAddress::~EndpointAddress() {
	if (m_result != NULL) {
		// std::cout << IP() << std::endl;
		// freeaddrinfo(m_result);	// Temporarily disable this line to avoid crash.
		m_result = NULL;
	}
}

/*
 * ==============
 * class Endpoint
 * ==============
 */


Endpoint::Endpoint(int type, std::string hostname, string service)
	{
		Initialize();
		Create(type, hostname, service);
	}
Endpoint::Endpoint(int type, string address) {
	Initialize();
	size_t pos = address.find(":");
	if (pos == string::npos) {
		char port[6];
		sprintf_s(port, 6, "%d", DEFAULT_PORT);
		Create(type, address, port);
	} else {
		string hostname, service;
		hostname = address.substr(0, pos);
		service = address.substr(pos + 1);
		Create(type, hostname, service);
	}
}

bool Endpoint::Initialize() {
		/*if (Endpoint::g_initialized)
			return true;*/
#ifdef _WIN32
		WSADATA wsaData;
		// Need WinSock2 at least for raw sockets
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
#ifdef _EP_DEBUG
			std::cout << "couldn't start!" << std::endl;
#endif
			return false;
		}
#endif
		// return Endpoint::g_initialized = true;
		return true;
	}

// bool Create(int type, std::string host, std::string service) { };	
bool Endpoint::Create(int type, std::string host, string service) {
		m_sockfd = m_servfd = 0;
		m_bool = false;
		m_type = type & EP_SOCK_MASK;
		m_server = type & SERVER;
		EndpointAddress addr(host, service);
		if (m_server)
		{
			m_local = addr;
		}
		else {
			m_remote = addr;
		}

		// std::cout << "[DEBUG] " << addr.IP() << std::endl;
		switch (m_type) {
		case TCP:
			if (m_server)
				Create_TCP_SERVER();
			else
				Create_TCP_CLIENT();
		}
		return m_bool;
	}

bool Endpoint::Create_TCP_SERVER() {
		int iResult;
		SOCKET ListenSocket = INVALID_SOCKET;
		SOCKET ClientSocket = INVALID_SOCKET;
		/*memset(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;*/
		// Create a SOCKET for connecting to server
		struct addrinfo* result = m_local.m_result;
		ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (ListenSocket == INVALID_SOCKET) {
#ifdef _WIN32
			printf("socket failed with error: %ld\n", WSAGetLastError());
#else
			printf("socket failed with error: %d\n", errno);
#endif
			/*freeaddrinfo(result);
			WSACleanup();
			return 1;*/
		}
		else {
			m_servfd = ListenSocket;
		}

		// Setup the TCP listening socket
		iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
#ifdef _WIN32
			printf("bind failed with error: %ld\n", WSAGetLastError());
#else
			printf("bind failed with error: %d\n", errno);
#endif
			/*freeaddrinfo(result);
			closesocket(ListenSocket);
			WSACleanup();
			return 1;*/
		}
		iResult = listen(ListenSocket, SOMAXCONN);		
		if (iResult == SOCKET_ERROR) {
#ifdef _WIN32
			printf("listen failed with error: %ld\n", WSAGetLastError());
#else
			printf("listen failed with error: %d\n", errno);
#endif
			/*closesocket(ListenSocket);
			WSACleanup();
			return 1;*/
		}

		// Accept a client socket
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
#ifdef _WIN32
			printf("accept failed with error: %ld\n", WSAGetLastError());
#else
			printf("accept failed with error: %d\n", errno);
#endif
			/*closesocket(ListenSocket);
			WSACleanup();
			return 1;*/
			m_bool = false;
		}
		else {
			m_sockfd = ClientSocket;
			m_bool = true;
		}
		return m_bool;
	}

bool Endpoint::Create_TCP_CLIENT() {
        // Attempt to connect to an address until one succeeds
        int iResult;
        SOCKET ConnectSocket = INVALID_SOCKET;
        struct addrinfo* ptr;
        for (ptr = m_remote.m_result; ptr != NULL; ptr = ptr->ai_next) {

                // Create a SOCKET for connecting to server
                ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                        ptr->ai_protocol);
                if (ConnectSocket == INVALID_SOCKET) {
#ifdef _WIN32
                        printf("socket failed with error: %ld\n", WSAGetLastError());
#else
                        printf("socket failed with error: %d\n", errno);
#endif
                }
                m_sockfd = ConnectSocket;
                // Connect to server.
                iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
                if (iResult == SOCKET_ERROR) {
                        Close();
                        ConnectSocket = INVALID_SOCKET;
                        continue;
                }
                m_bool = true;
                break;
        }

        if (ConnectSocket == INVALID_SOCKET) {
                printf("Unable to connect to server!\n");
#ifdef _WIN32
                WSACleanup();			
#endif
        }
        return m_bool;
}

int Endpoint::Send(const void *msg, int len, int flags)
{
        int bytes_sent;
        bytes_sent = send(m_sockfd, (char*)msg, len, 0);
        if (bytes_sent == (unsigned)-1 || bytes_sent == (unsigned)0)// error/ closed
        {
                /*m_bool = false;
                m_error_cat = EP_ERROR_SEND;
                SetLastError();*/
                return bytes_sent;
        }

        /*AddBytesSent(bytes_sent);*/
        return bytes_sent;
}

// Write all bytes in data
// Returns number of bytes written
int Endpoint::Write(std::string data)
{
        unsigned int bytes_left, bytes_sent;
        bytes_left = data.length();

        /*if (!m_bool)
        {
                return -1;
        }*/

        // Do not actually send 0 bytes, doing this will kill connection.
        if (bytes_left == 0)
        {
                return 0;
        }

        // If raw socket, include the IP header, unless its RAW_RAW
        // (Note: raw sockets are IPv4-only, use datalink access for raw IPv6)
//		if (m_type >= RAW_BASE && m_type != RAW_RAW) {
//			char* buff = new char[8192];   // TODO: MTU
//			char* pseudobuff = new char[8192];
//			int iphdrsz = 20;              // No IP options, 20 byte IP header
//			int layer4sz;                  // Layer 4 (TCP, UDP, etc.) hdr size
//
//			switch (m_type)
//			{
//			case RAW_UDP:  layer4sz = 8; break;
//			case RAW_ICMP: layer4sz = 4; break;
//			default: layer4sz = 0; break;
//			}
//
//			// Copy data to IP buff
//			memcpy(buff + iphdrsz + layer4sz, data.data(), data.length());
//
//			// Build an IP header. Always do this if IP_HDRINCL is set.
//			/*ep_iphdr* iphdr = (struct ep_iphdr*)buff;*/
//
////#if __BYTE_ORDER == __LITTLE_ENDIAN
////			iphdr->ver_ihl = 0x40 | iphdrsz >> 2;
////#elif __BYTE_ORDER == __BIG_ENDIAN
////			iphdr->ver_ihl = ((iphdrsz >> 2) << 4) | 0x4;     // really ihl_ver
////#else
////# error "Please fix <bits/endian.h>"
////#endif
//
//			//iphdr->ihl = iphdrsz >> 2;   //  Header Length(=20 bytes)
//			//iphdr->version = 4;   // IPv4
//
//			//iphdr->dscp = 8;      // Differentiated Services Field (?)
//			//iphdr->tot_len = 20 + (m_type == RAW_UDP ? 8 : 0) + data.length() 
//			//iphdr->tot_len = iphdrsz + layer4sz + data.length();
//			//iphdr->id = 0;        // IPID, let kernel fill it in (randomize??)
//			//iphdr->frag_off = 0;  // Fragment offset & flags = not fragmented
//			//iphdr->ttl = 128;     // Time to Live
//			//iphdr->protocol = (m_type >> 1) - (RAW_BASE >> 1);   // Next protocol
//			//iphdr->check = 0;     // Checksum, filled in by kernel
//			//iphdr->saddr = inet_addr(m_local.IP().c_str());      // Source & 
//			//iphdr->daddr = inet_addr(m_remote.IP().c_str());     // dest address
//
//			//if (m_type == RAW_UDP) {
//			//	ep_udphdr* udphdr = (struct ep_udphdr*)(buff + iphdrsz);
//			//	ep_pseudohdr* pseudohdr = (struct ep_pseudohdr*)pseudobuff;
//
//			//	udphdr->sport = htons(m_local.IntPort());
//			//	udphdr->dport = htons(m_remote.IntPort());
//			//	udphdr->sum = 0;
//			//	udphdr->ulen = htons(data.length() + 8);
//
//			//	// Copy UDP header and data into pseudo header
//			//	memcpy(pseudobuff + sizeof(struct ep_pseudohdr), buff + iphdrsz,
//			//		layer4sz + data.length());
//
//			//	pseudohdr->saddr = iphdr->saddr;
//			//	pseudohdr->daddr = iphdr->daddr;
//			//	pseudohdr->zero = 0;
//			//	pseudohdr->protocol = IPPROTO_UDP;
//			//	pseudohdr->length = udphdr->ulen;
//
//			//	// XXX: Incorrect checksum if source port is 0? Inconsistantly.
//			//	// Or if we specify 0 as src addr so it gets filled in
//			//	udphdr->sum = Endpoint::in_cksum((u_short*)pseudobuff,
//			//		sizeof(struct ep_pseudohdr) + layer4sz + data.length());
//
//			//}
//			//else if (m_type == RAW_ICMP) {
//			//	ep_icmphdr* icmphdr = (struct ep_icmphdr*)(buff + iphdrsz);
//
//			//	// Port number, upper 8 bits are type, lower are code XXX:ENDIAN
//			//	icmphdr->type = m_remote.IntPort() >> 8;
//			//	icmphdr->code = m_remote.IntPort() & 0xff;
//			//	icmphdr->sum = 0;
//
//			//	icmphdr->sum = Endpoint::in_cksum((u_short*)icmphdr,
//			//		layer4sz + data.length());
//			//}
//
//			int err = sendto(m_sockfd, buff, data.length() + iphdrsz + layer4sz, 0,
//				(sockaddr*)m_remote.ai_addr, m_remote.ai_addrlen);
//
//			if (err < 0)
//			{
//#ifdef _EP_DEBUG
//				std::cout << "send error: " << strerror(errno)
//					<< std::endl;
//#endif
//				m_bool = false;
//				m_error_cat = EP_ERROR_SEND;
//				SetLastError();
//			}
//
//			delete[] buff;
//			delete[] pseudobuff;
//
//			return err;
//		}

        // Loop until either a) all bytes are sent b) no more bytes could be sent
        do
        {
                // XXX: shall flags be MSG_WAITALL, if supported? 
                bytes_sent = send(m_sockfd, (data.data() + data.length() - bytes_left), bytes_left, 0);

                if (bytes_sent == (unsigned)-1 || bytes_sent == (unsigned)0)// error/ closed
                {
                        /*m_bool = false;
                        m_error_cat = EP_ERROR_SEND;
                        SetLastError();*/
                        return bytes_sent;
                }

                //m_bytes_sent += bytes_sent;
                //Endpoint::g_bytes_sent += bytes_sent;
                // AddBytesSent(bytes_sent);

                bytes_left -= bytes_sent;
        } while (bytes_left > 0 && bytes_sent != 0);

        if (bytes_sent != data.length()) {
#ifdef _EP_DEBUG
                std::cout << "Socket::Write() partial write, " << bytes_sent
                        << " != " <<
                        data.length() << std::endl;
#endif
                return bytes_sent;
        }
        return bytes_sent;
	}


int Endpoint::Recv(void *buf, int len, unsigned int flags)
{
        int returnVal;
        returnVal = recv(m_sockfd, (char*)buf, len, flags);

        // Check for conditions that may terminate reading prematurely
        if (returnVal == -1)        // -1  is error or EWOULDBLOCK
        {
                // Check if caller wanted non-blocking, and we got it
                if (/*!m_blocking && */errno == EAGAIN)
                        return returnVal;

                // Otherwise, this is an error
                /*m_bool = false;
                m_error_cat = EP_ERROR_RECV;
                SetLastError();*/
                return -1 * returnVal;   // Error condition
        }
        else if (returnVal == 0)    // 0 is EOF
        {
                return returnVal;
        }

        // AddBytesRecv(returnVal);
        return returnVal;
}

// Read bytes_to_read bytes into data, return FALSE if end-of-file
// bytes_to_read=x can be either:
// x>0 - read AT LEAST x bytes, blocking
// x<0 - read AT MOST -x bytes, blocking
// x=0 - Non-blocking I/O (not supported)
// (This is copied almost verbatim from my own IPv4-only Socket)
int Endpoint::Read(int bytes_to_read, std::string& data)
{
        int bytes_left = bytes_to_read;
        int bytes_read = 0;
        int bytes_just_read = 0;
        //bool eof            = false;
        bool read_all = true;
        //bool ret            = true;

        if (bytes_to_read == 0) {            // Non-blocking I/O (not supported)
#ifdef _EP_DEBUG
                std::cout << "Error, can't read 0 bytes" << std::endl;
#endif
        }

        if (bytes_to_read < 0)       // Negatives = don't read all, do one call
        {
                bytes_left = -bytes_to_read;
                read_all = false;
        }

        do
        {
                char* buf = new char[bytes_left];
                if (!buf)
                {
#ifdef _EP_DEBUG
                        std::cout << "OUT OF MEMORY!" << std::endl;
#endif
                        // force-close connection and return 0
                        /*m_bool = false;
                        m_error_cat = EP_ERROR_RECV;
                        m_error_code = 0;
                        m_error_str = "";*/
                        Close();
                        CloseServer();
                        return 0;
                }

                bytes_just_read = recv(m_sockfd, (char*)buf, bytes_left, 0);
                if (bytes_just_read > 0)
                {
                        bytes_read += bytes_just_read;
                        data += std::string(buf, bytes_just_read);
                }
                delete[] buf;      // TODO: Less inefficient 

                // Check for conditions that may terminate reading prematurely
                if (bytes_just_read == -1)        // -1  is error or EWOULDBLOCK
                {
                        // Check if caller wanted non-blocking, and we got it
                        if (/*!m_blocking && */errno == EAGAIN)
                                return bytes_read;

                        // Otherwise, this is an error
                        /*m_bool = false;
                        m_error_cat = EP_ERROR_RECV;
                        SetLastError();*/
                        return -bytes_read;   // Error condition
                        break;
                }
                else if (bytes_just_read == 0)    // 0 is EOF
                        return bytes_read;

                bytes_left -= bytes_just_read;

                // If read_all is set (the default, for + values), loop again.
                // If not set (for - bytes_to_read values), don't loop anymore
                if (!read_all)
                        break;
        } while (bytes_left != 0);

        //m_bytes_recv += data.length();
        //Endpoint::g_bytes_recv += data.length();
        //AddBytesRecv(data.length());

        return bytes_read;
}

bool Endpoint::Close()
{
        EndpointAddress blank;
        m_remote = blank;
        if (m_sockfd > 0)
        {
#ifdef _WIN32
                closesocket(m_sockfd);
#else
                close(m_sockfd);
#endif
        }
        m_sockfd = 0;
        return true;
}

bool Endpoint::CloseServer()
{
        if (m_servfd > 0)
        {
#ifdef _WIN32
                closesocket(m_servfd);
#else
                close(m_servfd);
#endif
        }
        m_servfd = 0;
        return true;
}

Endpoint::~Endpoint()
{
        Close();
        CloseServer();
#ifdef _WIN32
        WSACleanup();
#endif
}

// Initialization of static class variable should be placed
// in .cpp instead of .h to avoid duplicate initialization.
bool Endpoint::g_initialized = false;