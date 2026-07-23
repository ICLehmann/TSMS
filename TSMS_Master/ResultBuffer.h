#pragma once
#include <deque>
#include <vector>
#include "MeasuredValue.h"
#include "Counter.h"

class ResultBuffer
{
private:
	// fast sequence for storing measuremet results
	// tuple: index number of the part (incremented), last cycle time, measurement value array
	std::deque<std::tuple<long, int, std::vector<MeasuredValue>>> _buffer;	
	bool PartOK(std::vector<MeasuredValue> &values);
	void PrintOnePart(std::tuple<long, int, std::vector<MeasuredValue>> line);
	bool ValueExists(std::vector<MeasuredValue>& values, unsigned int measurement_id);
	long _current_part_num;	// part counter (incremented)
	TSMSCounter _counter;

public:
	void AddNewEntry(int last_cycle_time);
	void NewPart();
	void AddValue(MeasuredValue value, bool isDummy);
	std::tuple<long, int, std::vector<MeasuredValue>> GetAndRemoveFirstPart();
	void PushBack(long part_num, int last_cycle_time, std::vector<MeasuredValue>& values);
	long GetLastPartNum();
	void SetCurrentPartNum(long current_part_num);

	void SetCounter(TSMSCounter& counter) { _counter = counter; }
	TSMSCounter& GetCounter() { return _counter; }
	void ResetCounter() { _counter.Reset(); }

	void ResetParts();

	void PrintAll();
	void PrintLast();
	unsigned int GetNumEntries() { return _buffer.size(); }

	
};

