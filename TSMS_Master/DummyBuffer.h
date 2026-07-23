#pragma once
#include <vector>
#include "const.h"
#include "MeasuredValue.h"

class DummyBuffer
{
private:
	std::vector<MeasuredValue> _results[tsms_config::NUM_MEASUREMENTS];  // project specific // todo: generalize
	unsigned int _counter;
	bool _sim;

public:
	DummyBuffer();
	void Reset();
	void AddValue(MeasuredValue& value);
	bool Evaluation();
	unsigned int GetCounter() { return _counter; }
};

