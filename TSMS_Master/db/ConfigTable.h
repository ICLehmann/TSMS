#pragma once
#include "BaseTable.h"
#include "../Meter/DeviceConfig.h"

class ConfigTable : public BaseTable
{
public:
	bool Create();
	bool Insert(MeasurementValueConfig& config, std::string device, int station);
};