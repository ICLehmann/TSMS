#pragma once
#include "windows.h" 

#include <string>

class RS232
{
private:
	HANDLE serialHandle;
	DCB serialParams = { 0 };
	COMMTIMEOUTS timeouts = { 0 };
	
	bool b_init;
	int _comport;
	int _baud;
	
public:
	RS232();
	~RS232();

	bool Open(int com, int baud);
	void Close();
	bool Write(std::string write_data);
	bool Read(std::string &read_data);
};

