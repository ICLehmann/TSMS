#include "MeterTCP.h"
#include <iostream>
#include <string.h>
#include <sstream>      // std::stringstream

MeterTCP::MeterTCP() : Meter()
{
	memset(_receivebuffer, 0x00, sizeof(_receivebuffer));
}

MeterTCP::~MeterTCP()
{
	clean_up();
}

bool MeterTCP::init(DeviceConfig& dev_config)
{
	set_base_config(dev_config);

	// Connect
	if (!_tcp.Connect(dev_config.GetAddress().c_str(), dev_config.GetTCPPortOrGPIBDevice()))
	{
		logger::WriteLog("TCP Error: Connection failed");
		return false;
	}

	// Probe.
	Send("*IDN?\r\n", true);
	if (strlen(_receivebuffer))
	{
		std::cout << _receivebuffer;
	}
	else
	{
		logger::WriteLog("TCP Error: No response");
		return false;
	}

	for (auto& line : _device_config.init_seq)
	{
		Send(line.c_str(), true);
	}

	_is_init = true;
	return true;
}

void MeterTCP::clean_up()
{
	if (_is_init)
	{
		logger::WriteLog("Clean up " + get_device_name() + " ...");
		_tcp.Close();
		_is_init = false;
	}
}

bool MeterTCP::Send(std::string command, bool recv_answer)
{
	if (!_tcp.Send(command.c_str()))
		return false;

	if (recv_answer && strchr(command.c_str(), '?') != NULL)
	{
		memset(_receivebuffer, 0x00, sizeof(_receivebuffer));
		if (!_tcp.Receive(_receivebuffer))
			return false;
	}
	return true;
}

bool MeterTCP::ReceiveValues(std::vector<ReceivedValue>& values)
{
	memset(_receivebuffer, 0x00, sizeof(_receivebuffer));

	if (_tcp.Receive(_receivebuffer))
	{
		std::stringstream rev_stream(_receivebuffer);
		while (rev_stream.good())
		{
			std::string value_str;
			getline(rev_stream, value_str, ',');
			ReceivedValue rv;
			try 
			{
				rv.value = std::stod(value_str);
				rv.vaild = true;
			}
			catch (const std::invalid_argument&) {
				//std::cerr << "Argument is invalid\n";
				rv.vaild = false;
			}
			catch (const std::out_of_range&) {
				//std::cerr << "Argument is out of range for a double\n";
				rv.vaild = false;
			}
			values.push_back(rv);
		}
		return true;
	}
	return false;
}

void MeterTCP::comp_init()
{
	if (!_is_init)
		return;

	for (auto& line : _device_config.comp_init_seq)
	{
		Send(line.c_str(), true);
	}

}

bool MeterTCP::comp_wait_IM7581()
{
	const int max_loops = 60;
	const int wait_ms = 1000;

	bool is_Fail = false;
	int cnt = 0;
	while (cnt < max_loops)
	{
		Sleep(wait_ms);
		std::cout << ".";
		cnt++;

		if (!_tcp.Send(":CORRection:ERRor?\r\n"))
		{
			is_Fail = true;
			break;
		}

		memset(_receivebuffer, 0x00, sizeof(_receivebuffer));
		if (_tcp.Receive(_receivebuffer, false))
		{
			int comp_err = atoi(_receivebuffer);
			is_Fail = comp_err != 0;
			break;
		}
	}
	
	return is_Fail;
}

bool MeterTCP::comp_IM7581(std::vector<std::string>& seq)
{
	for (auto& line : seq)
	{
		Send(line.c_str(), false);
	}

	bool cerr = comp_wait_IM7581();

	if (cerr)
		logger::WriteLog("Compensation '" + _device_config.GetDeviceName() + "' finished with error!");
	else
		logger::WriteLog("Compensation '" + _device_config.GetDeviceName() + "' finished successfully");
	return !cerr;
}

bool MeterTCP::comp_open()
{
	logger::WriteLog("Start open compensation " + _device_config.GetDeviceName() + "'");
	return comp_IM7581(_device_config.comp_open_seq); // IM7581 only
}

bool MeterTCP::comp_short()
{
	logger::WriteLog("Start short compensation " + _device_config.GetDeviceName() + "'");
	return comp_IM7581(_device_config.comp_short_seq);	// IM7581 only
}


