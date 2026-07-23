#include "MeasuredValue.h"

MeasuredValue::MeasuredValue(unsigned int type, unsigned int station)
{
	Reset();
	this->_type = type;
	this->station_nr = station;
}

void MeasuredValue::Reset()
{
	_value = 0;
	time_ms = 0;
	station_nr = 0;
	_type = 0;
	eval = Evaluation::NONE;
	eval_nopat = Evaluation::NONE;
	pat_enabled=0;
	_pat_min=0;
	_pat_max=0;
}




