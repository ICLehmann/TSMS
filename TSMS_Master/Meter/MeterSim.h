#pragma once
#include "Meter.h"

class MeterSim : public Meter
{
protected:
	double _dev_percent;
	bool ReceiveValues(std::vector<ReceivedValue>& values) override { return true; }
	bool Send(std::string command, bool recv_answer) override { return true; }
	bool Wait_Until_RES2329_Finished() override { return true; }		// special for RES2329

	void comp_init() override { };
	bool comp_short() override;
	bool comp_open() override;

public:
	MeterSim(double dev_percent);
	~MeterSim();

	void set_hardware_config(int device, int address, int wait_ms) {} //GPIB
	void set_hardware_config(char* ip, int port, int num, int wait_ms) {} //TCP

	bool init(DeviceConfig& dev_config) override;
	void clean_up() override;
	std::vector<MeasuredValue> Measure();
};

