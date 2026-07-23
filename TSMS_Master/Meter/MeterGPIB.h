#pragma once
#include "Meter.h"
#include <tuple>

class MeterGPIB :  public Meter
{
protected:
	int  _dev;
	void OnError(void);
	
	char _receivebuffer[1024];

	bool RES2329_Measuring();
	bool RES2329_ValueAvailable();

	bool Send(std::string commnad, bool synchronous) override;
	bool ReceiveValues(std::vector<ReceivedValue>& values) override;
	bool Wait_Until_RES2329_Finished() override;

public:
	MeterGPIB();
	~MeterGPIB();

	bool init(DeviceConfig& dev_config) override;
	void clean_up() override;

	void comp_init() override {}					// we don't need it for our GPIB devices
	bool comp_open() override { return false; } 	// we don't need it for our GPIB devices
	bool comp_short() override;
};

