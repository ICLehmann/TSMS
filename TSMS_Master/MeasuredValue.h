#pragma once

enum Evaluation { NONE, PASS, LOW, HIGH, FAIL };

class MeasuredValue
{
private:
	double _value;
	unsigned int _type = 0;

public:
	MeasuredValue(unsigned int type, unsigned int station_nr);
		
	Evaluation eval; // evaluation by narrowed PAT tolerance limits
	Evaluation eval_nopat; // evaluation by original tolerance limits
	unsigned int time_ms;
	unsigned int station_nr;
	// pat related data for plotting on UI'charts
	char pat_enabled;
	double _pat_min;
	double _pat_max;
	void Reset();

	void SetValue(double value) { _value = value; }
	double GetValue() { return _value; }

	unsigned int GetID() { return _type; }
};

