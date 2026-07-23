#include "LaserData.h"
#include <time.h> 

/////////////////////////////////////////////////////////////////////////////
// helper from martin
/////////////////////////////////////////////////////////////////////////////

	/* DIN/ISO Kalenderwoche berechnen */
long JD(int y, int m, int d)
{ //calculate and return Julian Day Number from calendar date
	long jd = (1461L * (y + 4800 + (m - 14) / 12)) / 4 +
		(367 * (m - 2 - 12 * ((m - 14) / 12))) / 12 -
		(3 * ((y + 4900 + (m - 14) / 12) / 100)) / 4 +
		d - 32075;
	return jd;
}

char usWeekday(long J)
// Rückgabewert ist amerikanischer Wochentag mit Sun=0, Mon=1, Sat=6
{
	return(J + 1) % 7;
}

char isoWeekDay(long J)
// Parameter ist die Julianische Tagesnummer
// Rückgabewert ist europäischer ISO Wochentag mit  Mon=1, Tue=2,Sat=6, Sun=7
{
	return J % 7 + 1;
}

long kw1JD(int year)
{// liefert die Julianische Tagesnummer der ersten KW des Jahres
	long result = JD(year, 1, 4); // vierter Januar gehört immer zur 1.KW
	char weekday = isoWeekDay(result); // Wochentag nach ISO
	return result - weekday + 1;
}

int KW(int Year, char Month, char Day)
{
	long kwJD = JD(Year, Month, Day);
	if (Month == 12 && kwJD >= kw1JD(Year + 1)) return 1; // im Dezember bereits erste Woche des Folgejahres
	else if (Month == 1 && kwJD < kw1JD(Year)) return KW(Year - 1, 12, 31);//im Januar noch letzte KW  des Vorjahrs
	else return 1 + (kwJD - kw1JD(Year)) / 7; // Normalfall
}

LaserData::LaserData(char product_name, char location, long long lot_num)
{
	Mark.clear();

	Mark = "--";	// init
	Mark[0] = product_name;
	Mark[1] = location;

	std::string lot_num_str = std::to_string(lot_num);
	Mark += lot_num_str.substr(lot_num_str.length() - 2);

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	int kw = KW(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

	std::string year = std::to_string(tm.tm_year);
	Mark += year.substr(year.length() - 1);

	char buffer[4];
	sprintf(buffer, "%02d%1d", kw, tm.tm_wday);

	Mark += buffer;
}
