/*
* led_blinker.cpp
*
* Created on: August, 2024
*   Author: Michele Di Lucchio
*    
*/

#include "led_blinker.h"

// Inizializza LedBlinker
void initLedBlinkerStruct(LedBlinkerStruct* ledBlinkerStruct, int pin){
  ledBlinkerStruct->pin = pin;
  ledBlinkerStruct->lastTime = 0;
  ledBlinkerStruct->isOn = false;

  pinMode(ledBlinkerStruct->pin, OUTPUT);
  digitalWrite(ledBlinkerStruct->pin, LOW);
}

void setLed(LedBlinkerStruct* ledBlinkerStruct, bool value){

  digitalWrite(ledBlinkerStruct->pin, value);

  value ? ledBlinkerStruct->isOn = true : ledBlinkerStruct->isOn = false; // (condizione) ? (valore_se_vero) : (valore_se_falso);

}

// Blink del LED per un numero specifico di volte con un periodo di lampeggio
/*void enableLedBlink(LedBlinkerStruct* ledBlinkerStruct, unsigned long blinkPeriod, int blinkCount){

  for (int i = 0; i < blinkCount; i++) {

    unsigned long currentTime = millis();

    // Cambia lo stato del primo LED
    ledBlinkerStruct->isOn = !ledBlinkerStruct->isOn;
    digitalWrite(ledBlinkerStruct->pin, ledBlinkerStruct->isOn ? HIGH : LOW);

    // Attendi per il periodo di lampeggio prima di continuare
    while (millis() - currentTime < blinkPeriod) {
      // Do nothing, just wait
    }
    
  }

  // Spegni il LED alla fine del ciclo
  digitalWrite(ledBlinkerStruct->pin, LOW);

}*/

/*void doubleLedBlink(LedBlinkerStruct* ledBlinkerStruct1, LedBlinkerStruct* ledBlinkerStruct2, unsigned long blinkPeriod, int blinkCount) {

  for (int i = 0; i < blinkCount; i++) {
    unsigned long currentTime = millis();

    // Cambia lo stato del primo LED
    ledBlinkerStruct1->isOn = !ledBlinkerStruct1->isOn;
    digitalWrite(ledBlinkerStruct1->pin, ledBlinkerStruct1->isOn ? HIGH : LOW);

    // Cambia lo stato del secondo LED
    ledBlinkerStruct2->isOn = !ledBlinkerStruct2->isOn;
    digitalWrite(ledBlinkerStruct2->pin, ledBlinkerStruct2->isOn ? HIGH : LOW);

    // Attendi per il periodo di lampeggio prima di continuare
    while (millis() - currentTime < blinkPeriod) {
      // Do nothing, just wait
    }
  }

  // Spegni i LED alla fine del ciclo
  digitalWrite(ledBlinkerStruct1->pin, LOW);
  digitalWrite(ledBlinkerStruct2->pin, LOW);
}*/

void ledBlink(LedBlinkerStruct* ledBlinkerStruct1, LedBlinkerStruct* ledBlinkerStruct2, unsigned long period){
  unsigned long currentTime = millis(); // Ottieni il tempo attuale una sola volta

  // Aggiorna lo stato di entrambi i LED
  if(currentTime - ledBlinkerStruct1->lastTime >= period) {

    // Cambia lo stato del primo LED
    ledBlinkerStruct1->lastTime = currentTime;
    ledBlinkerStruct1->isOn = !ledBlinkerStruct1->isOn;
    digitalWrite(ledBlinkerStruct1->pin, ledBlinkerStruct1->isOn ? HIGH : LOW);

    // Cambia lo stato del secondo LED
    ledBlinkerStruct2->lastTime = currentTime;
    ledBlinkerStruct2->isOn = !ledBlinkerStruct2->isOn;
    digitalWrite(ledBlinkerStruct2->pin, ledBlinkerStruct2->isOn ? HIGH : LOW);

  }
}