#pragma once
#include "Meter.h"
#include "../common/tcp.h"

class MeterTCP : public Meter
{

protected:

	tcp _tcp;
	char _receivebuffer[2048];

	bool Send(std::string command, bool recv_answer) override;
	bool ReceiveValues(std::vector<ReceivedValue>& values) override;
	bool Wait_Until_RES2329_Finished() override { return true; }		// special for RES2329

	
	bool comp_wait_IM7581();
	bool comp_IM7581(std::vector<std::string>& seq);

public:
	MeterTCP();
	~MeterTCP();

	bool init(DeviceConfig& dev_config) override;
	void clean_up() override;

	void comp_init() override;
	bool comp_short() override;
	bool comp_open() override;
};

