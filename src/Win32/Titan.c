
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "types.h"
#include "file.h"
#include "CommandWrap.h"

#define TITAN_LOG_FILE_NAME "titanlog.txt"

//bool titanLogEnable = TRUE;

void titanDebug(char *format, ...)
{
    if (logEnable)
    {
#define DATA_SIZE 200 // space passed data is allowed to take up
#define TIME_SIZE 9   // space required by time string at front of each line
        char buffer[DATA_SIZE + TIME_SIZE + 2]; // +1 for the carriage-return and null terminator
        SYSTEMTIME systime;
        va_list argList;
        int aNumChars;

        GetSystemTime(&systime);
        _snprintf(buffer, TIME_SIZE, "%02d:%02d:%02d ", systime.wHour, systime.wMinute, systime.wSecond);

        va_start(argList, format);                            //get first arg
        aNumChars = _vsnprintf(buffer + TIME_SIZE, DATA_SIZE, format, argList); //prepare output string
        va_end(argList);

        if (aNumChars >= 0)
            memcpy((void*)(buffer + TIME_SIZE + aNumChars), "\n\0", 2);

        logfileLog(TITAN_LOG_FILE_NAME,buffer);
    }
}

void titanLogFileOpen(void)
{
	time_t now;
	char datestring[16];

    if (logEnable)
    {
        logfileClear(TITAN_LOG_FILE_NAME);
		now = time(NULL);
		strftime(datestring, 16, "%a %b %d %Y", gmtime(&now));
		titanDebug("Todays date is: %s", datestring);
    }
}

void titanLogFileClose(void)
{
    ;
}

