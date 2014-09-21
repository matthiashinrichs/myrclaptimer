// Lapstorage.h

#ifndef Lapstorage_h
#define Lapstorage_h

#include "Arduino.h"
#include "TimeTools.h"


// If we assume that lap data will always be set to 0
// Session ends will always be 0
// we only ever return invalid handle

//*******************************************************************************************
// Lap storage and retreival definitions
//*******************************************************************************************
#define EMPTY_LAP_TIME 0
#define INVALID_LAP_HANDLE 0XFFFF
#define EEPROM_LAP_STORE_MAX_LAPS 500


typedef unsigned int lap_handle_t;


//*******************************************************************************************
// ILapStore 
//
// Defines a pure virtual class (C++ Terminology) or interface (Java Terminology)
// It simply defines the functions that can be used to get, set and clear laps
// in a lap store.
//
// The following lap store is provided
// 1) CEEPromLapStore - this one stores laps in the EEPROM 
// If you wanted to add SD Card Storage you could define a new class CSDCardLapStore
// 
//*******************************************************************************************
class ILapStore
{
public:
  virtual lap_time_t getLapTime(lap_handle_t lapHandle) = 0;
  virtual void setLapTime(lap_handle_t lapHandle,lap_time_t lapTime) = 0;
  virtual void clearAll() = 0;
  virtual unsigned int getMaxLaps() = 0;
  protected:
  ILapStore *m_pLapStore;

};

//*******************************************************************************************
// CEEPROMLapStore 
//
// Store laps in memory -
//
// For - simple, easy, its just using an array in memory
// Against - lose all laps if power is lost or Arduino is reset
//
//*******************************************************************************************

class CEEPROMLapStore : public ILapStore
{
public:
 virtual lap_time_t getLapTime(lap_handle_t lapHandle);
 virtual void setLapTime(lap_handle_t lapHandle,lap_time_t lapTime);
 virtual void clearAll();
 virtual unsigned int getMaxLaps();
};

//*******************************************************************************************
// CLapTimes 
//
// A lot of the work in this class is simply finding the start and end of sessions, and
// finding space to start a new session.
//
// With more memory I would have used headers to do a lot of the work inside CLapTimes
// A file system could also have done a lot of the work.
//
// It isn't pretty and could be refactored but it works.
//
//*******************************************************************************************
class CLapTimes
{
public:
  CLapTimes(ILapStore *pLapStore);
  lap_handle_t createNewSession();
  void setLapTime(lap_handle_t lapHandle,lap_time_t lapTime);
  lap_time_t getLapTime(lap_handle_t lapHandle);
  void getTotals(unsigned int &nSessions,unsigned int &nLapsRecorded,unsigned int &nLapsRemaining);
  void clearAll();
  lap_handle_t getSessionSummary(lap_handle_t lapHandle,unsigned int &nSessionAverage,unsigned int &nSessionBest,unsigned int &nSessionLapCount);
  lap_handle_t addLapTime(lap_handle_t lapHandle,lap_time_t lapTime);
  lap_handle_t moveNext(lap_handle_t lapHandle);
  lap_handle_t movePrevious(lap_handle_t lapHandle);
  lap_handle_t getSessionHandle(uint8_t nSession);
protected:
  ILapStore *m_pLapStore;
public:
  static char m_pTimeStringBuffer[9];/*m:ss:dd - dd represents hundredths of a second */
};



#endif