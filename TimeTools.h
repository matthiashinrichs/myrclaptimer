/* 
	TimeTools.h 
	includes functions turn millis() into a formatted timestring e.g. mm:ss.dd 
*/

#ifndef TimeTools_h
#define TimeTools_h

#include "Arduino.h"

typedef unsigned int lap_time_t;




class TimeTools
{
public:
	TimeTools();
	static char* timeString(lap_time_t time);
	static lap_time_t convertMillisToLapTime(unsigned long ulTime);
	static char m_pTimeStringBuffer[9];/*m:ss:dd - dd represents hundredths of a second */
};




#endif