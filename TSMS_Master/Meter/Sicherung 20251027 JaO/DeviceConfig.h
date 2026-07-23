#pragma once

#include "../TestConfig.h"

#include <string>
#include <vector>

struct MeasurementValueConfig
{
	std::string name;
	std::string unit;
	unsigned int id = 0;
	unsigned int answer_pos = 0;
};

class DeviceConfig
{
private:
	void Clear();
	void RemoveCommentAndTrimRight(std::string& line);
	void ReplaceSetupPlaceHolder(std::string place_holder, std::string replace_with);
	void ReadValueConfig(std::ifstream& file_stream);
	void ReadSequence(std::ifstream& file_stream, std::vector<std::string>& seq);
	bool ReadSingleValue(std::string& line, const char* find, std::string& value_str);

	std::vector<std::string> _setup_seq_template;
	std::string _devicename;
	unsigned int _station;
	unsigned int _trigger_delay_ms;
	unsigned int _wait_after_trigger_ms;
	bool _isGPIB;
	std::string _address;
	unsigned int _port_or_device;

public:

	DeviceConfig();
	std::vector<MeasurementValueConfig> measurements;
	std::vector<std::string> init_seq;
	std::vector<std::string> setup_seq_filled;
	std::vector<std::string> start_measure_seq;
	std::vector<std::string> ask_for_answer;

	std::vector<std::string> comp_init_seq;
	std::vector<std::string> comp_short_seq;
	std::vector<std::string> comp_open_seq;
	
	bool ReadConfigFile(std::string devicename);
	bool CreateSetupSeq(std::vector<ValueConfig>& config);
	bool IsGPIB() { return _isGPIB; }
	std::string& GetAddress() { return _address; }
	unsigned int GetTCPPortOrGPIBDevice() { return _port_or_device; }
	std::string& GetDeviceName() { return _devicename; }
	unsigned int GetStationNum() { return _station; }
	unsigned int GetTriggerDelay() { return _trigger_delay_ms; }
	unsigned int GetWaitAfterTrigger() { return _wait_after_trigger_ms; }
};

