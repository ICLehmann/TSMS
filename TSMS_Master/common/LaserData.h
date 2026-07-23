#pragma once
#include <string>

struct LaserData
{
	std::string Mark;
	LaserData(char product_name, char location, long long lot_num);
};

