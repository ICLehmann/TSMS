#include "UDPReceiver.h"

#include <string>
#include <stdexcept>

#include "../common/Logger.h"

#undef UNICODE
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib") 

SOCKET recv_sockect;
struct sockaddr_in recv_addr;

sockaddr_in RemoteAddr;
int RemoteAddrSize = sizeof(RemoteAddr);

UDPReceiver::UDPReceiver()
{
	_bInit = false;
	memset(_recv_buffer, 0, sizeof(_recv_buffer));
}

UDPReceiver::~UDPReceiver()
{
	close();
}

bool UDPReceiver::init(const char* pszIpAddress, unsigned int Port)
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
	if ((recv_sockect = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		WSACleanup();
		std::string errorMsg = "UDP ERROR: socket creation failed";
		logger::WriteLog(errorMsg);
		throw std::runtime_error(errorMsg);
	}

	memset(&recv_addr, 0, sizeof(recv_addr));

	// filling server information 
	recv_addr.sin_family = AF_INET;
	recv_addr.sin_port = htons(Port);
	recv_addr.sin_addr.s_addr = INADDR_ANY;


	if ((bind(recv_sockect, (struct sockaddr*)&recv_addr, sizeof(recv_addr))) < 0) {
		logger::WriteLog("UDP ERROR: binding failed");
		exit(-1);
	}

	InetPton(AF_INET, pszIpAddress, &recv_addr.sin_addr.s_addr);

	logger::WriteLog("UDP Receiver initialized successfully");
	_bInit = true;
	return true;
}

void UDPReceiver::close()
{
	if (_bInit)
	{
		closesocket(recv_sockect);
		WSACleanup();
		_bInit = false;
		logger::WriteLog("UDP Receiver closed");
	}
}

bool UDPReceiver::recv(char* data, unsigned int expected_len)
{
	fd_set fds;
	struct timeval tv;

	// Set up the file descriptor set.
	FD_ZERO(&fds);
	FD_SET(recv_sockect, &fds);

	// Set up the struct timeval for the timeout.
	tv.tv_sec = 0;
	tv.tv_usec = 100000;

	// Wait until timeout or data received.
	int n = select(recv_sockect, &fds, NULL, NULL, &tv);
	if (n == 0)
	{
		return false;		// timeout
	}
	else if (n == -1)
	{
		logger::WriteLog("UDP ERROR: receiving failed");
		return false;
	}

	memset(_recv_buffer, 0, sizeof(_recv_buffer));
	int ret = recvfrom(recv_sockect, _recv_buffer, sizeof(_recv_buffer), 0, (LPSOCKADDR)&RemoteAddr, &RemoteAddrSize);
	if (ret == expected_len)
	{
		memcpy(data, _recv_buffer, expected_len);
		return true;
	}

	return false;
}

