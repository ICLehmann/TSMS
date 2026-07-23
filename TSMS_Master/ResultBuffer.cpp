#include "ResultBuffer.h"
#include <iostream>

#include "common/Logger.h"

// check whether part'a all parameters are good
bool ResultBuffer::PartOK(std::vector<MeasuredValue>& values) 
{
	bool isOK = true;
	for (auto& v : values)
	{
		if (v.eval != Evaluation::PASS)
			isOK = false;
	}
	return isOK;
}

// add new item into the sequence with empty measured values
void ResultBuffer::AddNewEntry(int last_cycle_time)
{
	std::vector<MeasuredValue> empty_vector;
	_buffer.push_back(std::tuple(_current_part_num, last_cycle_time, empty_vector));
}

// increments part counters
void ResultBuffer::NewPart()
{
	_counter.input_parts++;
	_current_part_num++;
}

// is there any measurement
bool ResultBuffer::ValueExists(std::vector<MeasuredValue>& values, unsigned int measurement_id)
{
	bool val_exists = false;
	for (auto& v : values)
	{
		if (v.GetID() == measurement_id)
		{
			val_exists = true;
			break;
		}
	}
	return val_exists;
}

// adds a measurement value to the queue
void ResultBuffer::AddValue(MeasuredValue value, bool isDummy)
{
	if (_buffer.size() == 0)
	{
		logger::WriteLog("BUFFER ERROR: Can not add value because part buffer is empty.");
		return;
	}

	int insertPos = _buffer.size() - 1;
	if (!isDummy)	// dummy test without shift
		insertPos -= value.station_nr;	// calculates the insertion position that fits to the station where the measurement was executed

	if (insertPos < 0 || insertPos > (int)_buffer.size() - 1)
	{
		logger::WriteLog("BUFFER ERROR: No suitable position for value with id =" + std::to_string(value.GetID()) + "(" + std::to_string(value.GetValue()) + ") in buffer!");
	}
	else
	{
		if (ValueExists(std::get<2>(_buffer.at(insertPos)), value.GetID()))	// checks if this value has already added
		{
			logger::WriteLog("BUFFER ERROR: Value with id = " + std::to_string(value.GetID()) + "(" + std::to_string(value.GetValue()) + ") already exists in buffer!");
		}
		else
		{
			std::get<2>(_buffer.at(insertPos)).push_back(value);	// pushes the value

			// incremets the corresponding counter 
			if (value.eval == Evaluation::PASS)
				_counter.res_pass[value.GetID()]++;
			else if (value.eval == Evaluation::LOW)
				_counter.res_low[value.GetID()]++;
			else if (value.eval == Evaluation::HIGH)
				_counter.res_high[value.GetID()]++;			
			else
				_counter.res_fail[value.GetID()]++;
		}
	}
}


// takes out the last station's part from the queue
std::tuple<long, int, std::vector<MeasuredValue>> ResultBuffer::GetAndRemoveFirstPart()
{
	auto first_entry = _buffer.front();

	if (PartOK(std::get<2>(first_entry)))	// gets the measured values from the tupple and calculates if the part values are ok or not ok
		_counter.good_parts++;	// increments the number of the good parts

	_buffer.pop_front();	// removes the the station's part from the queue and effectively reduces its size
	return first_entry;
}

// pushes measured values to the first station
void ResultBuffer::PushBack(long part_num, int last_cycle_time, std::vector<MeasuredValue>& values)
{
	_buffer.push_back(std::tuple(part_num, last_cycle_time, values));
}

// returns the part index number at the first station
long ResultBuffer::GetLastPartNum()
{
	if (_buffer.size() > 0)
		return std::get<0>(_buffer.back()); // gets the tupple first elemet that is the index number of the part
	else
		return 0;
}

void ResultBuffer::SetCurrentPartNum(long current_part_num)
{
	if (current_part_num >= 0)
		_current_part_num = current_part_num;
}

// prints one part's data
void ResultBuffer::PrintOnePart(std::tuple<long, int, std::vector<MeasuredValue>> part)
{
	std::cout << "Buffer: " << std::get<0>(part) << "\t";
	const unsigned int n = tsms_config::NUM_MEASUREMENTS;
	double values[n] = { 0 };
	int times[n] = { 0 };
	for (auto& v : std::get<2>(part))
	{
		values[v.GetID()] = v.GetValue();
		times[v.GetID()] = v.time_ms;
	}

	for (int i = 0; i < n; i++)
	{
		if (values[i] == 0.0 && times[i] == 0)
			std::cout << "-----------\t";
		else
			std::cout << values[i] << "(" << times[i] << "ms)\t";
	}
	
	std::cout << "\n";
}

// clears the queue
void ResultBuffer::ResetParts()
{
	_buffer.clear();
	logger::WriteLog("Result buffer cleared!");
}

// prints all parts' data
void ResultBuffer::PrintAll()
{
	std::cout << "\n" << std::flush;
	for (auto& part : _buffer)
	{
		PrintOnePart(part);
	}
	std::cout << "\n";
}

// prints the part's data at the first station
void ResultBuffer::PrintLast()
{
	PrintOnePart(_buffer.back());
}

