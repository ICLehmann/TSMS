#include "RS232.h"

#include <cstdio>
#include <iostream>

#include "../common/Logger.h"

RS232::RS232()
{
	b_init = false;
	_comport = 0;
	_baud = 0;
	serialHandle = 0;
}

RS232::~RS232()
{
	Close();
}

bool RS232::Open(int com, int baud)
{
	if (b_init)
	{
		Close();
	}

	_comport = com;
	_baud = baud;

	wchar_t PortNo[20] = { 0 };
	swprintf_s(PortNo, 20, L"\\\\.\\COM%d", _comport);
	serialHandle = CreateFile(PortNo, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (serialHandle == INVALID_HANDLE_VALUE)
	{
		logger::WriteLog("Error opening COM" + std::to_string(_comport));
		return false;
	}

	serialParams.DCBlength = sizeof(serialParams);

	bool status = GetCommState(serialHandle, &serialParams);
	if (status == FALSE)
	{
		logger::WriteLog("ERROR: Getting state from COM" + std::to_string(_comport) + " failed");
		return false;
	}
	serialParams.BaudRate = _baud;
	serialParams.ByteSize = 8;
	serialParams.StopBits = ONESTOPBIT;
	serialParams.Parity = NOPARITY;
	serialParams.fTXContinueOnXoff = DTR_CONTROL_DISABLE;

	status = SetCommState(serialHandle, &serialParams);
	if (status == FALSE)
	{
		logger::WriteLog("ERROR: Setting state for COM" + std::to_string(_comport) + " failed");
		return false;
	}

	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;

	if (SetCommTimeouts(serialHandle, &timeouts) == FALSE)
	{
		logger::WriteLog("ERROR: Setting timeouts for COM" + std::to_string(_comport) + " failed");
		return false;
	}

	if (!PurgeComm(serialHandle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR))
	{
		logger::WriteLog("ERROR: 'PurgeComm' on COM" + std::to_string(_comport) + " failed");
	}

	logger::WriteLog("COM" + std::to_string(_comport) +" initialized");

	b_init = true;
	return true;
}

void RS232::Close()
{
	if (b_init)
	{
		if (!PurgeComm(serialHandle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR))
		{
			logger::WriteLog("ERROR: 'PurgeComm' on COM" + std::to_string(_comport) + " failed");
		}
		CloseHandle(serialHandle);
		serialHandle = 0;
		b_init = false;
	}
}

bool RS232::Write(std::string write_data)
{
	if (!b_init)
		return false;

	bool status;
	DWORD bytes_written;

	status = WriteFile(serialHandle, write_data.c_str(), write_data.size(), &bytes_written, NULL);
	if (status == FALSE)
	{
		logger::WriteLog("ERROR: Write to COM" + std::to_string(_comport) + " failed");
		return false;
	}

	if (bytes_written != write_data.size())
	{
		logger::WriteLog("ERROR: Write to COM" + std::to_string(_comport) + " not complete");
		return false;
	}
	return true;
}

bool RS232::Read(std::string& read_data)
{
	if (!b_init)
		return false;

	DWORD bytes_read;
	char data = 0;

	read_data.clear();
	do
	{
		if (ReadFile(serialHandle, &data, 1, &bytes_read, NULL))
		{
			if (bytes_read)
				read_data.append(1, data);
		}
		else
			return false;

	} while (bytes_read > 0);

	return (read_data.size() > 0);
}
