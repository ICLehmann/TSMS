#include "DeviceConfig.h"

#include "../common/Logger.h"

#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>    // std::remove()

DeviceConfig::DeviceConfig()
{
	Clear();
}

void DeviceConfig::Clear()
{
	_devicename = "unknown";
	_isGPIB = false;
	_station = 0;
	measurements.clear();
	init_seq.clear();
	setup_seq_filled.clear();
	start_measure_seq.clear();
	_wait_after_trigger_ms = 0;
	_trigger_delay_ms = 0;
}

void DeviceConfig::RemoveCommentAndTrimRight(std::string& line)
{
	// remove comment
	size_t found = line.find("#");
	if (found != std::string::npos)
	{
		line = line.substr(0, found);
	}

	// trim from right
	const char* t = " \t\n\r\f\v";
	line.erase(line.find_last_not_of(t) + 1);
}

void DeviceConfig::ReadValueConfig(std::ifstream& file_stream)
{
	MeasurementValueConfig value_config;
	std::string line;
	while (getline(file_stream, line))
	{
		if (line.size() == 0 || line.front() == '#')		// skip empty lines and comments 
			continue;

		size_t found = line.find("@end");
		if (found != std::string::npos)
		{
			measurements.push_back(value_config);
			break;
		}

		RemoveCommentAndTrimRight(line);

		found = line.find("name:");
		if (found != std::string::npos)
		{
			found = line.find(":");
			if (found != std::string::npos)
				value_config.name = line.substr(found + 1, line.size());
			continue;
		}

		found = line.find("unit:");
		if (found != std::string::npos)
		{
			found = line.find(":");
			if (found != std::string::npos)
				value_config.unit = line.substr(found + 1, line.size());
			continue;
		}

		found = line.find("position:");
		if (found != std::string::npos)
		{
			found = line.find(":");
			if (found != std::string::npos)
			{
				std::string pos_str = line.substr(found + 1, line.size());
				value_config.answer_pos = std::atoi(pos_str.c_str());
			}
			continue;
		}
	}
}

void DeviceConfig::ReadSequence(std::ifstream& file_stream, std::vector<std::string>& seq)
{
	std::string line;
	while (getline(file_stream, line))
	{
		if (line.size() == 0 || line.front() == '#')		// skip empty lines and comments 
			continue;

		size_t found = line.find("@end");
		if (found != std::string::npos)
			break;
		else
		{
			RemoveCommentAndTrimRight(line);
			seq.push_back(line + "\n");
		}
	}
}

bool DeviceConfig::ReadSingleValue(std::string& line, const char* find, std::string& value_str)
{
	size_t found = line.find(find);
	if (found != std::string::npos)
	{
		found = line.rfind(":");
		if (found != std::string::npos)
		{
			value_str = line.substr(found + 1, line.size());
			return true;
		}
	}
	return false;
}

bool DeviceConfig::ReadConfigFile(std::string devicename)
{
	_devicename = devicename;
	std::string filename = _devicename + ".txt";

	std::ifstream cfg_file(filename);
	if (cfg_file.is_open())
	{
		std::string line;

		while (getline(cfg_file, line))
		{
			if (line.size() == 0 || line.front() == '#')		// skip empty lines and comments 
				continue;

			RemoveCommentAndTrimRight(line);

			std::string value_str;
			if (ReadSingleValue(line, "@gpib", value_str))
				_isGPIB = std::atoi(value_str.c_str());

			if (ReadSingleValue(line, "@address", value_str))
				_address = value_str;

			if (ReadSingleValue(line, "@port_or_device", value_str))
				_port_or_device = std::atoi(value_str.c_str());
	
			if (ReadSingleValue(line, "@station:", value_str))
				_station = std::atoi(value_str.c_str());
			
			if (ReadSingleValue(line, "@trigger_delay:", value_str))
				_trigger_delay_ms = std::atoi(value_str.c_str());
		
			if (ReadSingleValue(line, "@wait_after_start:", value_str))
				_wait_after_trigger_ms = std::atoi(value_str.c_str());


			size_t found = line.find("@value");
			if (found != std::string::npos)			
				ReadValueConfig(cfg_file);
			
			found = line.find("@initialization");
			if (found != std::string::npos)			
				ReadSequence(cfg_file, init_seq);

			found = line.find("@setup");
			if (found != std::string::npos)
				ReadSequence(cfg_file, _setup_seq_template);

			found = line.find("@start_measurement");
			if (found != std::string::npos)
				ReadSequence(cfg_file, start_measure_seq);
			
			found = line.find("@get_answer");
			if (found != std::string::npos)			
				ReadSequence(cfg_file, ask_for_answer);		

			found = line.find("@compensation_init");
			if (found != std::string::npos)
				ReadSequence(cfg_file, comp_init_seq);

			found = line.find("@compensation_open");
			if (found != std::string::npos)
				ReadSequence(cfg_file, comp_open_seq);		

			found = line.find("@compensation_short");
			if (found != std::string::npos)			
				ReadSequence(cfg_file, comp_short_seq);		

		}
		cfg_file.close();
	}
	else
	{
		logger::WriteLog("Unable to open file: " + filename);
		return false;
	}

	return true;
}

void DeviceConfig::ReplaceSetupPlaceHolder(std::string place_holder, std::string replace_with)
{
	for (auto& setup_line : setup_seq_filled)
	{
		size_t found = 0;	
		while(found != std::string::npos)
		{
			found = setup_line.find(place_holder);		
			if (found != std::string::npos)
				setup_line.replace(found, place_holder.size(), replace_with);
		}
	}
}

bool DeviceConfig::CreateSetupSeq(std::vector<ValueConfig>& config)
{
	if (_setup_seq_template.size() == 0)
		return true;		// is ok, not all devices must have a setup config

	setup_seq_filled = _setup_seq_template;

	for (unsigned int i = 0; i < config.size(); i++)
	{
		std::string placeholder = "<Frequency" + std::to_string(i + 1) + ">";
		ReplaceSetupPlaceHolder(placeholder, std::to_string(config[i].frequency));

		placeholder = "<StimuLevel" + std::to_string(i + 1) + ">";
		ReplaceSetupPlaceHolder(placeholder, std::to_string(config[i].stimuli_level));
	}


	// check and save result in a file
	bool result = true;
	std::string filename = _devicename + "_setup_dbg.txt";
	std::ofstream debug_file(filename);
	for (auto& setup_line : setup_seq_filled)
	{
		debug_file << setup_line;

		size_t found = setup_line.find("<");
		if (found != std::string::npos)
			result = false;		// not all placeholders have been replaced
	}
	debug_file.close();


	return result;
}
