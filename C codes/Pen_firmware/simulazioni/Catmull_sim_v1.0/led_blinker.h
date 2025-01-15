/*
* led_blinker.h
*
* Created on: August, 2024
*   Author: Michele Di Lucchio
*   Description: Led pins Board SAMD21  
*/

#ifndef LED_H
#define LED_H

/************************************************************** Define macros *********************************************************/

/**************************************************************************************************************************************/

/**************************************************************************************************************************************/

/************************************************************* Include Files **********************************************************/
/**************************************************************************************************************************************/
#include <Arduino.h>

/**************************************************************************************************************************************/

/************************************************************* Type Definitions *******************************************************/
/**************************************************************************************************************************************/

struct LedBlinkerStruct
{
  int pin;                // pin collegato al LED
  unsigned long lastTime; // ultima volta che il LED è stato acceso/spento
  bool isOn;              // stato attuale del LED
};


/**************************************************************************************************************************************/

/************************************************************* Function Declarations **************************************************/
/**************************************************************************************************************************************/
void initLedBlinkerStruct(LedBlinkerStruct* ledBlinkerStruct, int pin);
void setLed(LedBlinkerStruct* ledBlinkerStruct, bool value);
void ledBlink(LedBlinkerStruct* ledBlinkerStruct1, LedBlinkerStruct* ledBlinkerStruct2, unsigned long period);
//void enableLedBlink(LedBlinkerStruct* ledBlinkerStruct, unsigned long blinkPeriod, int blinkCount);
//void doubleLedBlink(LedBlinkerStruct* ledBlinkerStruct1, LedBlinkerStruct* ledBlinkerStruct2, unsigned long blinkPeriod, int blinkCount);



/**************************************************************************************************************************************/

#endif /* LED_H */