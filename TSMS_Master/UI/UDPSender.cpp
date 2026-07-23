#include "UDPSender.h"

#include <iostream>
#include <string>
#include <stdexcept>

#include "../common/Logger.h"

#undef UNICODE
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib") 

SOCKET send_socket;
struct sockaddr_in send_addr;

UDPSender::UDPSender()
{
	_bInit = false;
}

UDPSender::~UDPSender()
{
	close();
}

bool UDPSender::init(const char* pszIpAddress, unsigned int Port)
{
	// Initialize WinSock
	WSADATA wsaData;
	int ret = WSAStartup(0x0202, &wsaData);
	if (ret != 0) {
		int ErrCode = ::GetLastError();
		std::string errorMsg = "UDP ERROR: initialize WinSock failed (" + std::to_string(ErrCode) + ")";
		logger::WriteLog(errorMsg);
		throw std::runtime_error(errorMsg);
	}

	// Creating socket file descriptor 
	if ((send_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		WSACleanup();
		std::string errorMsg = "UDP ERROR: socket creation failed";
		logger::WriteLog(errorMsg);
		throw std::runtime_error(errorMsg);
	}

	memset(&send_addr, 0, sizeof(send_addr));

	// filling server information 
	send_addr.sin_family = AF_INET;
	send_addr.sin_port = htons(Port);
	send_addr.sin_addr.s_addr = INADDR_ANY;

	InetPton(AF_INET, pszIpAddress, &send_addr.sin_addr.s_addr);

	logger::WriteLog("UDP Sender initialized successfully");
	_bInit = true;
	return true;
}

void UDPSender::close()
{
	if (_bInit)
	{
		closesocket(send_socket);
		WSACleanup();
		_bInit = false;
		logger::WriteLog("UDP Sender closed");

	}
}

bool UDPSender::send(const char* buf, unsigned int len)
{
	if (!_bInit)
		return false;

	int iResult = sendto(send_socket, buf, len, 0, (LPSOCKADDR)&send_addr, sizeof(send_addr));

	if (iResult == SOCKET_ERROR)
	{
		close();

		int ErrCode = ::WSAGetLastError();
		logger::WriteLog("UDP ERROR: sendto failed " + std::to_string(ErrCode));
		return false;
	}
	return true;
}
