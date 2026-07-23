#define WIN32_LEAN_AND_MEAN

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include "tcp.h"
#include "time.h"

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include <winsock2.h>
#include <stdio.h>


tcp::tcp()
{
	_bInit = false;
}

tcp::~tcp()
{
	Close();
}

bool tcp::Connect(const char* pszIpAddress, unsigned int Port)
{
	char OptVal = 1;
	int RetVal;

	// Initialize WinSock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
		return false;

	// Create socket
	m_Socket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (m_Socket == INVALID_SOCKET)
	{
		WSACleanup();
		return false;
	}

	// Convert address
	m_Address.sin_family = AF_INET;
	m_Address.sin_port = htons(static_cast<unsigned short>(Port));
	::inet_pton(AF_INET, pszIpAddress, &m_Address.sin_addr.s_addr);

	// Set to non blocking mode
	u_long block = 1;
	RetVal = ::ioctlsocket(m_Socket, FIONBIO, &block);
	if (RetVal != 0)
	{
		WSACleanup();
		return false;
	}

	if (connect(m_Socket, (struct sockaddr*)&m_Address, sizeof(m_Address)) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			// connection failed
			Close();
			return false;
		}

		// connection pending
		fd_set setW, setE;

		FD_ZERO(&setW);
		FD_SET(m_Socket, &setW);
		FD_ZERO(&setE);
		FD_SET(m_Socket, &setE);

		timeval time_out = { 0 };
		time_out.tv_sec = 2;
		time_out.tv_usec = 0;

		int ret = select(0, NULL, &setW, &setE, &time_out);
		if (ret <= 0)
		{
			// select() failed or connection timed out
			printf("tcp time out\n");

			Close();
			if (ret == 0)
				WSASetLastError(WSAETIMEDOUT);
			return false;
		}

		if (FD_ISSET(m_Socket, &setE))
		{
			// connection failed
			Close();
			return false;
		}
	}

	// connection successful


   // Set to blocking mode
	block = 0;
	RetVal = ::ioctlsocket(m_Socket, FIONBIO, &block);
	if (RetVal != 0)
	{
		Close();
		return false;
	}

	// Disable transmit delay (Nagle algorithm)
	RetVal = ::setsockopt(m_Socket, IPPROTO_TCP, TCP_NODELAY, &OptVal, sizeof(OptVal));
	if (RetVal != 0)
	{
		Close();
		return false;
	}

	_bInit = true;
	return true;
}

void tcp::Close(void)
{
	_bInit = false;
	shutdown(m_Socket, SD_SEND);
	closesocket(m_Socket);
	WSACleanup();
}

bool tcp::Send(const char* pszBuffer)
{
	int Offset = 0;
	int Remain = strlen(pszBuffer);
	while (Remain > 0)
	{
		int RetVal = ::send(m_Socket, pszBuffer + Offset, Remain, 0);
		if (RetVal == SOCKET_ERROR)
		{
			printf("send socket error\n");
			return false;
		}
		Offset += RetVal;
		Remain -= RetVal;
	}

	return true;
}

bool tcp::Receive(char* buffer, bool print_msg)
{
	fd_set fds;
	struct timeval tv;

	// Set up the file descriptor set.
	FD_ZERO(&fds);
	FD_SET(m_Socket, &fds);

	// Set up the struct timeval for the timeout.
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	// Wait until timeout or data received.
	int n = select(m_Socket, &fds, NULL, NULL, &tv);
	if (n == 0)
	{
		if(print_msg)
			printf("Timeout error while receiving data ..\n");
		return false;		// timeout
	}
	else if (n == -1)
	{
		if (print_msg)
			printf("Error while receiving tcp ..\n");
		return false;
	}

	memset(m_szBuffer, 0x00, sizeof(m_szBuffer));

	int ret = ::recv(m_Socket, m_szBuffer, sizeof(m_szBuffer) - 1, 0);
	if (ret > 0)
	{
		memcpy(buffer, m_szBuffer, ret);
		return true;
	}

	return false;
}