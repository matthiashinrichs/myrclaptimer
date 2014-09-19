// my Arduino test Suite written by matthias hinrichs

// test.ino

#include <LiquidCrystal.h>

void setup() {
Serial.begin(9600);
Serial.println("Program initialized");

}

void loop() {
Serial.println("Hallo Welt");
delay(5000);

}