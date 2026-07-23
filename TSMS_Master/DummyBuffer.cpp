#include "DummyBuffer.h"
#include "common/IniFile.h"
#include "common/Logger.h"

#include <iostream>

DummyBuffer::DummyBuffer()
{
	_counter = 0;
	_sim = false;
	IniFile ini(tsms_config::INI_FILE.c_str());
	int SimMode = ini.ReadInteger("Simulation", "Mode", 0);
	if (SimMode > 0)
		_sim = true;
}

void DummyBuffer::Reset()
{
	_counter = 0;
	for (auto &res : _results)
		res.clear();
}

void DummyBuffer::AddValue(MeasuredValue &value)
{
	auto id = value.GetID();
	if (id < tsms_config::NUM_MEASUREMENTS)
	{
		_results[id].push_back(value);
		_counter++;
	}

	// SPECIAL TREATMENT due to machine ejection
		
	// first nok part we test only L
	if (id == tsms_config::eMeasurement::Z1 && _results[id].size() == 3)
		value.eval = Evaluation::PASS;

	// second nok part we test only Z1
	if (id == tsms_config::eMeasurement::L && _results[id].size() == 4)
		value.eval = Evaluation::PASS;

	// first nok part we test only R1
	if (id == tsms_config::eMeasurement::R2 && _results[id].size() == 3)
		value.eval = Evaluation::PASS;

	// second nok part we test only R2
	if (id == tsms_config::eMeasurement::R1 && _results[id].size() == 4)
		value.eval = Evaluation::PASS;
}

bool DummyBuffer::Evaluation()
{
	bool result = true;

	logger::WriteLog("E-Dummy evaluation ...");
	
	// check the number of measured values
	for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
	{
#if WITH_C
		if (i == tsms_config::eMeasurement::Z2 || i == tsms_config::eMeasurement::Z3 ||
			i == tsms_config::eMeasurement::C2 || i == tsms_config::eMeasurement::C3)
			continue;
#else
		if (i == tsms_config::eMeasurement::Z2 || i == tsms_config::eMeasurement::Z3 ||
			i == tsms_config::eMeasurement::C1 || i == tsms_config::eMeasurement::C2 || i == tsms_config::eMeasurement::C3)
			continue;
#endif

		if (_results[i].size() < 4)
		{
			logger::WriteLog("E-Dummy: Not enough values in buffer '" + tsms_config::MeasurementNames[i] + "'!");
			return false;
		}
	}

	if (_sim)
	{
		// all simulated values should be good
		for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
		{
#if WITH_C
			if (i == tsms_config::eMeasurement::Z2 || i == tsms_config::eMeasurement::Z3 ||
				i == tsms_config::eMeasurement::C2 || i == tsms_config::eMeasurement::C3)
				continue;
#else
			if (i == tsms_config::eMeasurement::Z2 || i == tsms_config::eMeasurement::Z3 ||
				i == tsms_config::eMeasurement::C1 || i == tsms_config::eMeasurement::C2 || i == tsms_config::eMeasurement::C3)
				continue;
#endif
			for (unsigned int j = 0; j < 4; j++)
			{
				if (_results[i][j].eval != PASS)
				{
					logger::WriteLog("E-Dummy: Value " + std::to_string(j) + " from '" + tsms_config::MeasurementNames[i] + "' should be ok, but is nok!");
					result = false;
				}
			}
		}
	}
	else
	{
		// es gibt 10 Teile
		// Teil 1 + 2  sind Gutteile, Rest schlecht
		// Gutteile werden immer getriggert
		// Teil 3 + 4 nur Iso -> muss schlecht sein -> dann ok
#if WITH_C
		// Teil 5  nur L -> muss schlecht sein  -> dann ok
		// Teil 6  nur Z -> muss schlecht sein  -> dann ok
		// Teil 7 + 8 nur C  -> muss schlecht sein  -> dann ok
#else
		// Teil 5 + 6 nur L -> muss schlecht sein  -> dann ok		
		// Teil 7 + 8 nur Z  -> muss schlecht sein  -> dann ok
#endif
		// Teil 9 nur R1 -> muss schlecht sein  -> dann ok
		// Teil 10 nur R2 -> muss schlecht sein  -> dann ok
		// 
		// sind alle Ergebnisse  richtig ????


		// check part 1 + 2 -> must be ok
		for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
		{
			for (unsigned int j = 0; j < 2; j++)
			{
#if WITH_C == 0
				// skip all C values
				if (i == tsms_config::eMeasurement::C1 || i == tsms_config::eMeasurement::C2 || i == tsms_config::eMeasurement::C3)
					continue;
#endif 
				if (_results[i][j].eval != PASS)
				{
					logger::WriteLog("E-Dummy: Value " + std::to_string(j) + " from '" + tsms_config::MeasurementNames[i] + "' should be ok, but is nok!");
					result = false;
				}
			}
		}

		if (result)		// check the two NIO parts
		{
			for (unsigned int i = 0; i < tsms_config::NUM_MEASUREMENTS; i++)
			{
				for (unsigned int j = 2; j < 4; j++)
				{
#if WITH_C
					// first nok part (index 2) we test only L -> skip Z
					if (i == tsms_config::eMeasurement::Z1 && j == 2)
						continue;

					// second nok part (index 3) we test only Z1 -> skip L
					if (i == tsms_config::eMeasurement::L && j == 3)
						continue;
#else
					// skip all C values
					if (i == tsms_config::eMeasurement::C1 || i == tsms_config::eMeasurement::C2 || i == tsms_config::eMeasurement::C3)
						continue;

#endif
					// first nok part we test only R1 -> skip R2
					if (i == tsms_config::eMeasurement::R2 && j == 2)
						continue;

					// second nok part we test only R2 -> skip R1
					if (i == tsms_config::eMeasurement::R1 && j == 3)
						continue;

					if (_results[i][j].eval == PASS)
					{
						logger::WriteLog("E-Dummy: Value " + std::to_string(j) + " from '" + tsms_config::MeasurementNames[i] + "' should be nok, but is ok!");
						result = false;
					}
				}
			}
		}

	}

	return result;
}
