#pragma once

#include <ctime>
#include <string>
#include <iomanip>
#include <sstream>
#include <time.h>

inline std::string GetCurrentDateTime()
{
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);
	std::ostringstream oss;
	// This MS SQL date format is culture independent:
	oss << std::put_time(&tm, "%Y%m%d %H:%M:%S");	//  converts the date and time information from a given calendar time
	return oss.str();
};

inline std::string GetCurrentDateTime_Minus24h()
{
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);
	tm.tm_hour -= 24; // substracting 24 hours
	std::mktime(&tm);
	std::ostringstream oss;
	// This MS SQL date format is culture independent:
	oss << std::put_time(&tm, "%Y%m%d %H:%M:%S");	//  converts the date and time information from a given calendar time
	return oss.str();
};

inline time_t StrTimeToTime(const std::string& time_str)
{
	std::tm tm = {};
	std::istringstream ss(time_str);
	ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");	// parses the character input as a date/time value according to format string
	return mktime(&tm);
}

// calculates the difference in seconds between now and t
inline int GetTimeDiffToNow(time_t t)
{
	auto t_now = std::time(nullptr);
	auto diff_sec = difftime(t_now, t); // calculates the difference in seconds between beginning and end.
	return (int) diff_sec;
}

