//Libraries
#include "DS3231.h"//https://github.com/NorthernWidget/DS3231

//Variables
byte Year ;
byte Month ;
byte Date ;
byte DoW ;
byte Hour ;
byte Minute ;
byte Second ;
bool Century  = false;
bool h12 ;
bool PM ;

//Objects
DS3231 Clock;

void setup(){
//Init Serial USB
Serial.begin(9600);
Serial.println(F("Initialize System"));
//Wire.begin();

    /*Clock.setClockMode(false);  // set to 24h
    Clock.setSecond(0);
    Clock.setMinute(23);
    Clock.setHour(5);
    Clock.setDate(10);
    Clock.setMonth(10);
    Clock.setYear(20);
    Clock.setDoW(6);*/
}

void loop(){
readRTC();
}

void readRTC( ){/* function readRTC */ 
////Read Real Time Clock
Serial.print(Clock.getYear(), DEC);
        Serial.print("-");
        Serial.print(Clock.getMonth(Century), DEC);
        Serial.print("-");
        Serial.print(Clock.getDate(), DEC);
        Serial.print(" ");
        Serial.print(Clock.getHour(h12, PM), DEC); //24-hr
        Serial.print(":");
        Serial.print(Clock.getMinute(), DEC);
        Serial.print(":");
        Serial.println(Clock.getSecond(), DEC);
        delay(1000);
}

