// my Arduino test Suite written by matthias hinrichs

// test.ino

int led = 13;
int menuState = 1;
int countdown = 10;


#include <LiquidCrystal.h>
#include "TimeTools.h"
#include "LapStorage.h"


TimeTools timetools;

// configure LCD-display
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

// define pins for buttons
#define KEY_OK_PIN 6
#define KEY_CANCEL_PIN 5
#define KEY_UP_PIN 4
#define KEY_DOWN_PIN 3

// bit flags used in key functions getKeys, waitForKeyPress, waitForKeyRelease
#define KEY_NONE     0
#define KEY_OK         1
#define KEY_CANCEL     2
#define KEY_UP         4
#define KEY_DOWN     8

#define KEYPRESS_ANY B11111111


// flags to manage access and pulse edges
volatile uint8_t bIRPulseFlags;

volatile unsigned long ulNewStartTime;

#define IR_PULSE_START_SET 1
#define IR_PULSE_END_SET 2


//*****************************************************************
// Global Instance of CLapTimes class (choosing Storage-Device)
//*****************************************************************
CLapTimes gLapTimes(new CEEPROMLapStore());


void setup() {
	pinMode(13, OUTPUT);

	lcd.begin(16,2);
	lcd.setCursor(0,0);
	lcd.print("Test App");
	delay(1000);

	Serial.begin(9600);
	Serial.println("Program initialized");

	pinMode(KEY_OK_PIN,INPUT);
	pinMode(KEY_CANCEL_PIN,INPUT);
	pinMode(KEY_UP_PIN,INPUT);
	pinMode(KEY_DOWN_PIN,INPUT);

}

void loop() {
	while(true){
		showMenu();

	    // statement
	    switch (waitForKeyPress(KEYPRESS_ANY)) {
	    	case KEY_OK:
	    	if(menuState == 1){
	    		countdownRun();
	    	} 
	    	else if(menuState == 2) {
	    		doCounter();
	    	} 
	    	else if(menuState == 3) {
	    		doShowSessionSummaries();
	    	}
	    	else {
	    		lcd.clear();
	    		lcd.print("OK");
	    	}
	    	lcd.clear();
	    	lcd.print("OK");
	    	break;
	    	case KEY_CANCEL:
	    	lcd.clear();
	    	lcd.print("CANCEL");
	    	menuState = 1;
	    	break;
	    	case KEY_UP:
	    	if(menuState < 4){
	    		menuState++;
	    	} 
	    	else {
	    		menuState = 1;
	    	}
	    	break;
	    	case KEY_DOWN:
	    	if(menuState > 1){
	    		menuState--;
	    	} else {
	    		menuState = 4;
	    	}
	    	break;
	    }


	    waitForKeyRelease();
	}

}

// MENU
void showMenu() {
	
	lcd.clear();
	if(menuState == 1){
		lcd.print("Run Countdown");
		//lcd.print(timetools.timeString(millis()));
	}
	if(menuState == 2){
		lcd.print("Counter");
	}
	if(menuState == 3){
		lcd.print("Sessions");
	}
	if(menuState == 4){
		showTotals();
	}
	
}


//Countdown app
void countdownRun() {
	unsigned long previousMillis = 0;
	long interval = 1000;
	lcd.clear();

	while(getKeys() != KEY_CANCEL){
		unsigned long currentMillis = millis();
		if(currentMillis - previousMillis >= interval) {
			// save the last time you blinked the LED 
			previousMillis = currentMillis; 
			if (countdown > 1) {
				countdown--;
				lcd.clear();
				lcd.print(countdown);
			} 
			else {
				lcd.clear();
				lcd.print("ENDE");
			}

		}

	}

}

//////////////////////////////////////////////////////////////////////////////////
//
// doCounter
//
// start recording new sessions, update screen every second
// check for new laps
// record new laps
// show lap time for a few seconds at the end of a lap
// update and show new best lap if its a new session best
//
//////////////////////////////////////////////////////////////////////////////////
void doCounter() {

	lap_handle_t currentLapHandle = gLapTimes.createNewSession();

	lap_time_t bestLapTime = 0XFFFF;
	unsigned long ulOldStartTime = millis();

	unsigned long ulLastTimeRefresh = millis();
    char *pStringTimeBuffer = NULL;

	long interval = 100;

	attachInterrupt(0,interruptMe,CHANGE);

	while((getKeys() != KEY_CANCEL) && (currentLapHandle != INVALID_LAP_HANDLE))
	{
		if((IR_PULSE_END_SET|IR_PULSE_START_SET) == bIRPulseFlags)
    	{
    		unsigned long ulLastLapTime = ulNewStartTime - ulOldStartTime;
    		ulOldStartTime = ulNewStartTime;
    		lap_time_t lapTime = timetools.convertMillisToLapTime(ulLastLapTime);

    		gLapTimes.addLapTime(currentLapHandle, lapTime);
    		currentLapHandle = gLapTimes.moveNext(currentLapHandle);

    		//check for new best lap
    		if ( lapTime < bestLapTime ) {
    			bestLapTime = lapTime;
    		}
    		
    		lcd.clear();
    		lcd.print("Best: ");
    		lcd.print(timetools.timeString(bestLapTime));
    		lcd.setCursor(0,1);
    		lcd.print("Last: ");
    		lcd.print(timetools.timeString(lapTime));


    		delay(2000);
    		// give ownership of the shared variables back to the ISR
      		bIRPulseFlags = 0;

    	}
		unsigned long ulCurrentLapTime = millis();
		if(ulCurrentLapTime - ulLastTimeRefresh >= interval) {
 			// save the last time you blinked the LED 
 			ulLastTimeRefresh = ulCurrentLapTime; 
 			lcd.clear();
      
      if(bestLapTime != 0XFFFF)
      {
       lcd.print("Best Lap ");
       lcd.print(timetools.timeString(bestLapTime));
      }
      else
      {
       lcd.print("Recording"); 
      }
      
      pStringTimeBuffer = timetools.timeString(timetools.convertMillisToLapTime(ulCurrentLapTime - ulOldStartTime));
      if(pStringTimeBuffer != NULL)
      {
        lcd.setCursor(0,1);
        lcd.print(pStringTimeBuffer);
      }
      else
      {
        // If we do not complete a lap for 9m59s display an idle message until a key is pressed
        lcd.setCursor(0,1);
        lcd.print("Idle");
        waitForKeyPress(KEYPRESS_ANY);
        ulOldStartTime = millis();
      }
 		}
 	}
  if(currentLapHandle == INVALID_LAP_HANDLE)
  {
    lcd.setCursor(0,1);
    lcd.print("Speicher voll!");
  }
}

//////////////////////////////////////////////////////////////////////////////////
//
// showTotals shows the number of sessions, laps and laps left
// 
//
//////////////////////////////////////////////////////////////////////////////////
void showTotals()
{
 unsigned int nSessions = 0;
 unsigned int nLapsRecorded = 0;
 unsigned int nLapsRemaining = 0;

 gLapTimes.getTotals(nSessions,nLapsRecorded,nLapsRemaining);

 lcd.clear();
 lcd.print("Sessions:");lcd.print(nSessions);
 lcd.setCursor(0, 1);
 lcd.print("Laps:");lcd.print(nLapsRecorded);
 lcd.print(" (");lcd.print(nLapsRemaining);lcd.print(")");
}


 void interruptMe(){
  uint8_t bLapCaptureState = digitalRead(2);
  //digitalWrite(LAP_CAPTURE_LED,bLapCaptureState);
  if(bLapCaptureState == LOW)
  {
     bIRPulseFlags = (IR_PULSE_END_SET|IR_PULSE_START_SET);
     ulNewStartTime = millis();
  } 
  else
  {
    bIRPulseFlags = 0;
  }
 }

 //////////////////////////////////////////////////////////////////////////////////
//
// doShowSessionSummaries
//
// implements the show session summary menu
// allows the user to scroll up and down through summaries of the recorded sessions
// user can press ok to enter the session and scroll through the session laps
//
//////////////////////////////////////////////////////////////////////////////////
void doShowSessionSummaries()
{
 boolean bFinished = false;
 uint8_t nSession = 0;

 do
 {
  lap_handle_t lapHandle = 0;
  uint16_t nSessionAverage = 0;
  uint16_t nSessionBest = 0;
  uint16_t nSessionLapCount = 0;

  Serial.println(nSession);
   
  lapHandle = gLapTimes.getSessionHandle(nSession);  
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SNo:");
  lcd.print(nSession);
  
  // if theres no laps for this session or its the first session but it doesnt contain any laps
  if(lapHandle == INVALID_LAP_HANDLE || (lapHandle == 0 && gLapTimes.getLapTime(lapHandle)==0))
  {
    lcd.setCursor(0,1);
    lcd.print("Empty Session");
  }
  else
  {
    Serial.println(lapHandle);
    
    gLapTimes.getSessionSummary(lapHandle,nSessionAverage,nSessionBest,nSessionLapCount);
    
    lcd.print(" Laps:");
    lcd.print(nSessionLapCount);
    lcd.setCursor(0,1);
// Best Lap Time
    lcd.print(timetools.timeString(nSessionBest));
// Average Lap Time
    lcd.print(" ");
    lcd.print(timetools.timeString(nSessionAverage));
  }

  waitForKeyRelease();

  switch(waitForKeyPress(KEYPRESS_ANY))
  {
   case KEY_UP:
    nSession++;
    break;
   case KEY_DOWN:
    nSession--;
    break;
   case KEY_CANCEL:
    bFinished = true;
    break;
   case KEY_OK:
    if(nSessionLapCount != 0)
    {  
      doLapScroll(gLapTimes.getSessionHandle(nSession));
    }
    break;
  }
 }while(!bFinished);
}

//////////////////////////////////////////////////////////////////////////////////
//
// doLapScroll
//
// scroll through the laps within a session, startLapHandle points to the start
//
//////////////////////////////////////////////////////////////////////////////////
void doLapScroll(lap_handle_t startLapHandle)
{
 boolean bFinished = false;
 lap_handle_t currentLapHandle = startLapHandle;
 lap_handle_t tmpLap = currentLapHandle;
 uint8_t nLapNumber = 0;

 do
 {
   lcd.clear();
   lcd.setCursor(0,0);
   
   if(tmpLap == INVALID_LAP_HANDLE)
   {
     lcd.print("No More Laps");
     delay(2000);
     lcd.clear();
   }

   lcd.print("Lap No.");
   lcd.print(nLapNumber);
   lcd.setCursor(0,1);
   
   if(currentLapHandle != INVALID_LAP_HANDLE)
   {
     char *pTime = timetools.timeString(gLapTimes.getLapTime(currentLapHandle));
     lcd.setCursor(0,1);
     lcd.print(pTime);  
   }

   waitForKeyRelease();
   
   uint8_t sKey = waitForKeyPress(KEYPRESS_ANY);
   switch(sKey)
   {
     case KEY_DOWN:
     case KEY_UP:
      (sKey == KEY_UP) ? tmpLap = gLapTimes.moveNext(currentLapHandle) : tmpLap = gLapTimes.movePrevious(currentLapHandle);
      if(tmpLap != INVALID_LAP_HANDLE)
      {
        if(gLapTimes.getLapTime(tmpLap) != EMPTY_LAP_TIME)
        {
          currentLapHandle = tmpLap;
          (sKey == KEY_UP) ? nLapNumber++ : nLapNumber--;
        }
        else
        {
          tmpLap = INVALID_LAP_HANDLE;
        }
      }
      break;
     case KEY_OK:
      tmpLap = currentLapHandle;
      break;
     case KEY_CANCEL:
      bFinished = true;
      break;
   }
 }
 while(!bFinished);
}


//////////////////////////////////////////////////////////////////////////////////
//
// Key related helpers
//
// getKeys - pole keys
// waitForKeyPress - block waiting for keys based on a mask
// waitForKeyRelease - block waiting until no kets are pressed
//
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//
// getKeys
//
// read the inputs and create a bit mask based on the buttons pressed
// this does not block, need to review whether we should make this block, in most
// cases we loop waiting for a key, sometimes we also loop waiting for no key
// could put both options here with an input parameter.
//
//////////////////////////////////////////////////////////////////////////////////
short getKeys()
{
 // Use bit flags for keys, we may have a future use for
 // combined key presses

 short sKeys = KEY_NONE;
 if(digitalRead(KEY_UP_PIN) == LOW)
 {
 	sKeys |= KEY_UP; 
 }
 if(digitalRead(KEY_DOWN_PIN) == LOW)
 {
 	sKeys |= KEY_DOWN;
 }
 if(digitalRead(KEY_OK_PIN) == LOW)
 {
 	sKeys |= KEY_OK;
 }
 if(digitalRead(KEY_CANCEL_PIN) == LOW)
 {
 	sKeys |= KEY_CANCEL;
 }

 return sKeys;
}

//////////////////////////////////////////////////////////////////////////////////
//
// waitForKeyRelease
//
// we can enter a function while the activating key is still pressed, in the new
// context the key can have a different purpose, so lets wait until it is released
// before reading it as pressed in the new context
// 
//////////////////////////////////////////////////////////////////////////////////
void waitForKeyRelease()
{
	do
	{
    // do nothing
	}
	while(getKeys() != KEY_NONE);

  	// debounce
  	delay(20);
}

//////////////////////////////////////////////////////////////////////////////////
//
// waitForKeyPress
//
// convenience function, loop doing nothing until one of the sKeyMask keys is 
// pressed
// 
//////////////////////////////////////////////////////////////////////////////////
uint8_t waitForKeyPress(uint8_t sKeyMask)
{
	uint8_t sKey = KEY_NONE;

	do
	{
		sKey = getKeys() & sKeyMask;    
	}
	while(sKey == KEY_NONE);

  //digitalWrite(BUZZER_PIN,HIGH);
  delay(20);
  //digitalWrite(BUZZER_PIN,LOW);
  
  return sKey;
}