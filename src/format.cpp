#include <string>

#include "format.h"

using std::string;

#define SECONDS_IN_HOUR 3600
#define SECONDS_IN_MINUTE 60

string Format::ElapsedTime(long seconds) { 
    int hours;
    int minutes;

    std::string sHours;
    std::string sMinutes;
    std::string sSeconds;

    hours = seconds / SECONDS_IN_HOUR;
    seconds -= hours*SECONDS_IN_HOUR;
    sHours = std::to_string(hours);

    minutes = seconds / SECONDS_IN_MINUTE;
    seconds -= minutes*SECONDS_IN_MINUTE;
    sMinutes = std::to_string(minutes);
    sMinutes.insert(0, 2 - sMinutes.length(), '0');

    sSeconds = std::to_string(seconds);
    sSeconds.insert(0, 2 - sSeconds.length(), '0');

    std::string formattedString = sHours + ":" + sMinutes + ":" + sSeconds;

    return formattedString;
}