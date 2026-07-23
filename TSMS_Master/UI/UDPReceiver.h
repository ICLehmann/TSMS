#pragma once

class UDPReceiver
{
private:
	bool _bInit;
	char _recv_buffer[500];

public:
	UDPReceiver();
	~UDPReceiver();

	bool recv(char* buf, unsigned int len);
	bool init(const char* pszIpAddress, unsigned int Port);
	void close();
};

