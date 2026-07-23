#pragma once
#include <string>
#include "vector"
#include <time.h>

#include "../common/Logger.h"
#include "../MeasuredValue.h"
#include "../TestConfig.h"

#include "DeviceConfig.h"

struct ReceivedValue
{
	double value = 0;
	bool vaild = false;
};

class Meter
{
protected:
	bool _is_init;
	
	std::vector<ValueConfig> _value_config;
	DeviceConfig _device_config;

	void set_base_config(DeviceConfig& dev_config);
	void FillMeasuredValue(MeasuredValue& m_val, ReceivedValue raw_value, unsigned int config_index);
	bool send_setup();

	virtual bool Send(std::string command, bool recv_answer) = 0;
	virtual bool ReceiveValues(std::vector<ReceivedValue>& values) = 0;
	virtual bool Wait_Until_RES2329_Finished() = 0;		// RES2329 only
	
public:
	Meter();
	bool set_value_config(std::vector<ValueConfig>& config);
	void update_PAT(std::vector<ValueConfig>& config);
	std::string get_device_name() { return _device_config.GetDeviceName(); }
	DeviceConfig& get_config() { return _device_config; }

	virtual void comp_init() = 0;
	virtual bool comp_short() = 0;
	virtual bool comp_open() = 0;

	virtual bool init(DeviceConfig& dev_config) = 0;
	virtual void clean_up() = 0;

	virtual std::vector<MeasuredValue> Measure(void);
};

