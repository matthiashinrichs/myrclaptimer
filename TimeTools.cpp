/*
  TimeTools.cpp - Library for generating timestrings.
  Created by Matthias Hinrichs, September 20, 2014.
  Not yet released into the public domain.
*/

#include "Arduino.h"
#include "TimeTools.h"

char TimeTools::m_pTimeStringBuffer[9];/*m:ss:dd - dd represents hundredths of a second */


TimeTools::TimeTools(){

}

// 10 minutes is 600 seconds or 600,000 milli seconds, this is too big to fit into a uint32_t
// so we divide by 10 to convert the value into a lap_time_t which contains the lap time in 100's
// of seconds.
lap_time_t TimeTools::convertMillisToLapTime(unsigned long ulTime)
{
  return ulTime/10;
}

char* TimeTools::timeString(lap_time_t time)
{	
	char *pResult = NULL;
	unsigned long nSeconds = time/100;
	unsigned long nMinutes = nSeconds/60;
	unsigned long nHundredths = time - (nSeconds*100);

	if(nMinutes <= 9)
	{
		nSeconds -= (nMinutes * 60);

		m_pTimeStringBuffer[7] = 0;
    	m_pTimeStringBuffer[6] = (nHundredths%10)+'0';
    	m_pTimeStringBuffer[5] = (nHundredths/10)+'0';
    	m_pTimeStringBuffer[4] = '.';
    	m_pTimeStringBuffer[3] = (nSeconds%10)+'0';
    	m_pTimeStringBuffer[2] = (nSeconds/10)+'0';
    	m_pTimeStringBuffer[1] = ':';
    	m_pTimeStringBuffer[0] = nMinutes + '0';    
    
    	pResult = m_pTimeStringBuffer;
	}

	return pResult;
}

