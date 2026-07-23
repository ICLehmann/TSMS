#include "MeterGPIB.h"
#include <Windows.h>
#include <iostream>
#include <sstream>      // std::stringstream
#include <mutex>
#include "../NI/ni4882.h"

#define NO_SECONDARY_ADDR     0     // Secondary address of device
#define TIMEOUT               T1s   // Timeout value = 1 seconds
#define EOTMODE               1     // Enable the END message  1
#define EOSMODE               0     // Disable the EOS mode      0


#include <chrono>

using namespace std::chrono;

static std::mutex gpib_mutex;

MeterGPIB::MeterGPIB() : Meter()
{
	_dev = 0;
	memset(_receivebuffer, 0x00, sizeof(_receivebuffer));
}

MeterGPIB::~MeterGPIB()
{
	clean_up();
}

bool MeterGPIB::init(DeviceConfig& dev_config)
{
	clean_up();

	set_base_config(dev_config);

	int address = std::atoi(dev_config.GetAddress().c_str());

	// Connect
	_dev = ibdev(dev_config.GetTCPPortOrGPIBDevice(), address, NO_SECONDARY_ADDR, TIMEOUT, EOTMODE, EOSMODE);
	if (Ibsta() & ERR)
	{
		OnError();
		return false;
	}

	// Clear
	ibclr(_dev);
	if (Ibsta() & ERR)
	{
		OnError();
		return false;
	}

	// Probe
	Send("*IDN?\n", true);
	if (strlen(_receivebuffer))
	{
		std::cout << _receivebuffer;
	}
	else
	{
		OnError();
		return false;
	}

	// Init device.
	for (auto& line : _device_config.init_seq)
	{
		Send(line.c_str(), true);
	}

	_is_init = true;
	return true;
}

void MeterGPIB::clean_up()
{
	if (_is_init)
	{
		logger::WriteLog("Clean up " + get_device_name() + " ...");

		ibonl(_dev, 0);
		if (Ibsta() & ERR)
		{
			printf("ibonl Error\n");
		}

		_is_init = false;
	}
}

void MeterGPIB::OnError(void)
{
	std::string errorMsg = "GPIB Error = " + std::to_string(Iberr()) + "\n";
	if (Iberr() == EDVR) errorMsg += " EDVR <Driver error>\n";
	if (Iberr() == ECIC) errorMsg += " ECIC <Not Controller-In-Charge>\n";
	if (Iberr() == ENOL) errorMsg += " ENOL <No Listener>\n";
	if (Iberr() == EADR) errorMsg += " EADR <Address error>\n";
	if (Iberr() == EARG) errorMsg += " EARG <Invalid argument>\n";
	if (Iberr() == ESAC) errorMsg += " ESAC <Not System Controller>\n";
	if (Iberr() == EABO) errorMsg += " EABO <Operation aborted>\n";
	if (Iberr() == ENEB) errorMsg += " ENEB <No GPIB board>\n";
	if (Iberr() == EOIP) errorMsg += " EOIP <Async I/O in progress>\n";
	if (Iberr() == ECAP) errorMsg += " ECAP <No capability>\n";
	if (Iberr() == EFSO) errorMsg += " EFSO <File system error>\n";
	if (Iberr() == EBUS) errorMsg += " EBUS <Command error>\n";
	if (Iberr() == ESTB) errorMsg += " ESTB <Status byte lost>\n";
	if (Iberr() == ESRQ) errorMsg += " ESRQ <SRQ stuck on>\n";
	if (Iberr() == ETAB) errorMsg += " ETAB <Table Overflow>\n";
	if (Iberr() == ELCK) errorMsg += " ELCK <Lock error>\n";
	if (Iberr() == EARM) errorMsg += " EARM <Ibnotify rearm error>\n";
	if (Iberr() == EHDL) errorMsg += " EHDL <Invalid Handle>\n";
	if (Iberr() == EWIP) errorMsg += " EWIP <Wait already in progress>\n";
	if (Iberr() == ERST) errorMsg += " ERST <Notification cancelled due to reset>\n";
	if (Iberr() == EPWR) errorMsg += " EPWR <Power error>\n";

	logger::WriteLog(errorMsg);
}

bool MeterGPIB::Send(std::string command, bool recv_answer)
{
	ibwrt(_dev, command.c_str(), strlen(command.c_str()));		// gpib write
	if (Ibsta() & ERR)
	{
		OnError();
		return false;
	}

	if (recv_answer && strchr(command.c_str(), '?') != NULL)
	{
		memset(_receivebuffer, 0x00, sizeof(_receivebuffer));
		ibrd(_dev, _receivebuffer, sizeof(_receivebuffer)); //read result
		if (Ibsta() & ERR)
		{
			logger::WriteLog("Error: Unable to read data from " + get_device_name() + ".");
			//onError();
			return false;
		}
	}

	return true;
}

bool MeterGPIB::ReceiveValues(std::vector<ReceivedValue>& values)
{
	std::lock_guard<std::mutex> lk(gpib_mutex);

	memset(_receivebuffer, 0x00, sizeof(_receivebuffer));
	
	ibtmo(_dev, 9); // set timeout (6=3ms 7=10ms 8=30ms 9=100ms)
	ibrd(_dev, &_receivebuffer, sizeof(_receivebuffer));
	unsigned int stat = Ibsta();
	if (stat & ERR)
		return false;

	if (strlen(_receivebuffer) > 0 )
	{
		ReceivedValue rv;
		if (strchr(_receivebuffer, '>') == NULL)		// RES2329 sends ">>>" when no contact
		{
			// decompose telegram
			std::vector<double> temp_values;
			std::stringstream rev_stream(_receivebuffer);
			while (rev_stream.good())
			{
				std::string value_str;
				getline(rev_stream, value_str, ',');
				double value = atof(value_str.c_str());
				temp_values.push_back(value);
			}

			if (temp_values.size() == 1)
			{		
				rv.value = temp_values[0];
				rv.vaild = true;
			}
			else if (temp_values.size() == 2) //iso with contact check
			{				
				if (temp_values[1] > 0) //contact check
				{
					rv.value = temp_values[0];
					rv.vaild = true;
				}
				else
					logger::WriteLog("ISO: Contact check faild!");
			}
		}
		else
			logger::WriteLog("R1/2: No contact!");

		values.push_back(rv);
		return true;
	}
	
	return false;
}

bool MeterGPIB::RES2329_Measuring()
{
	std::lock_guard<std::mutex> lk(gpib_mutex);

	memset(_receivebuffer, 0x00, sizeof(_receivebuffer));
	Send("s:o:c?\n", false);		// get Status Operation Condition Register
	ibrd(_dev, &_receivebuffer, sizeof(_receivebuffer));
	if (Ibsta() & ERR)
		return false;

	if (strlen(_receivebuffer) > 0)
	{
		unsigned short SOC = atoi(_receivebuffer);
		unsigned short stat = SOC & 16;		// bit "Measuring"  (see manual page 79)
		if (stat == 16)
			return true;
	}
	
	return false;
}

bool MeterGPIB::RES2329_ValueAvailable()
{
	std::lock_guard<std::mutex> lk(gpib_mutex);

	memset(_receivebuffer, 0x00, sizeof(_receivebuffer));
	Send("s:o:c?\n", false);		// get Status Operation Condition Register
	ibrd(_dev, &_receivebuffer, sizeof(_receivebuffer));
	if (Ibsta() & ERR)
		return false;

	if (strlen(_receivebuffer) > 0)
	{
		unsigned short SOC = atoi(_receivebuffer);
		unsigned short stat = SOC & 256;		// bit "Resistance value available (EOC)"  (see manual page 79)
		if (stat == 256)
			return true;
	}

	return false;
}

bool MeterGPIB::Wait_Until_RES2329_Finished()
{
	const int max_loops = 100;
	const int wait_ms = 2;

	int cnt = 0;
	while (cnt < max_loops)
	{
		if (RES2329_ValueAvailable())
		{
			Sleep(2);
			return true;
		}

		Sleep(wait_ms);
		cnt++;
	}
	return false;
}

bool MeterGPIB::comp_short()
{
	logger::WriteLog("Check short with limit on '" + _device_config.GetDeviceName() + "'");

	bool comp_result = false;

	double limit = 0;
	for (auto& line : _device_config.comp_short_seq)
	{
		size_t found = line.find("limit:");
		if (found != std::string::npos)
		{
			found = line.rfind(":");
			if (found != std::string::npos)
			{
				std::string value_str = line.substr(found + 1, line.size());
				limit = std::atof(value_str.c_str());
				logger::WriteLog("limit: " + std::to_string(limit));
			}
		}
	}

	if (limit > 0)		// do a measurement and check limit 
	{
		bool device_error = false;

		// trigger measurement
		for (auto& line : _device_config.start_measure_seq)
		{
			if (!Send(line, false))
			{
				device_error = true;
				break;
			}
		}

		if (!device_error)
		{
			Sleep(_device_config.GetWaitAfterTrigger());

			// ask device for answer telegram
			for (auto& line : _device_config.ask_for_answer)
			{
				size_t found = line.find("FETC");		//RES2329 only
				if (found != std::string::npos)
				{
					if (!Wait_Until_RES2329_Finished())
					{
						device_error = true;
						break;
					}
				}

				if (!Send(line, false))
				{
					device_error = true;
					break;
				}
			}
		}

		if (!device_error)
		{
			std::vector<ReceivedValue> values;
			if (!ReceiveValues(values))
				device_error = true;

			if (!device_error && values.size() > 0 && values[0].vaild)
			{
				logger::WriteLog("Measured value: " + std::to_string(values[0].value));
				if (values[0].value <= limit)
					comp_result = true;
			}
		}

		if (device_error)
			logger::WriteLog("Error Compensation: No value received!");
	}
	else
		logger::WriteLog("Error Compensation: No limit in config file found!");

	if (comp_result)
		logger::WriteLog("Short compensation finished");
	else
		logger::WriteLog("Error while short compensation");

	return comp_result;
}
