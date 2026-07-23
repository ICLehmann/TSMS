#pragma once

class UDPSender
{
private:
	bool _bInit;
	
public:
	UDPSender();
	~UDPSender();

	bool send(const char* buf, unsigned int len);
	bool init(const char* pszIpAddress, unsigned int Port);
	void close();
};

