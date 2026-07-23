#pragma once

#include <windows.h>

#include <stdio.h>
#include <string.h>
#include <winbase.h>

#define TCP_DEFAULT_BUFFERLEN 1024

class tcp
{
private:
	bool _bInit;
	sockaddr_in m_Address;
	SOCKET m_Socket;
	char m_szBuffer[TCP_DEFAULT_BUFFERLEN];

public:
	tcp();
	~tcp();

	bool Connect(const char* pszIpAddress, unsigned int Port);
	void Close(void);
	bool Receive(char* buffer, bool print_msg = true);
	bool Send(const char* pszBuffer);
};

