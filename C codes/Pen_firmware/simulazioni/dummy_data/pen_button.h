/*
* pen_button.h
*
* Created on: August, 2024
*   Author: Michele Di Lucchio
*   Description: Gestione bottoni pennino 
*/

#ifndef PEN_BUTTON_H
#define PEN_BUTTON_H

/************************************************************** Define macros *********************************************************/
#define DEBOUNCE_DELAY 500 // Tempo massimo (in ms) tra pressione e rilascio per considerare un "click" rapido

// Definisci i limiti minimo e massimo simmetrici rispetto a 1
#define MIN_SCALING_MOUSE 0.25  // Limite minimo (ad esempio, 1/4)
#define MAX_SCALING_MOUSE 4.0   // Limite massimo (ad esempio, 4)
/**************************************************************************************************************************************/

/**************************************************************************************************************************************/

/************************************************************* Include Files **********************************************************/
/**************************************************************************************************************************************/
#include <Arduino.h>

#include "led_blinker.h"
#include "usb_manager.h" // si porta dentro "MouseController.h"

/**************************************************************************************************************************************/

/************************************************************* Type Definitions *******************************************************/
/**************************************************************************************************************************************/
enum ButtonState {
  NOT_PRESSED,   // Pulsante non premuto
  PRESSED,       // Pulsante premuto (in attesa di rilascio)
  CLICKED,       // Pulsante rilasciato dopo un clic rapido
  LONG_PRESSED   // Pulsante premuto per lungo tempo
};

struct ButtonStruct
{
  ButtonState button;      // stato del pulsante
  unsigned long PressTime; // tempo di pressione del pulsante
  float buttonClickCount; // variabile per contare i click (questa variabile è usata dentro la funzione pow per questo il suo tipo deve essere float)

};
/**************************************************************************************************************************************/

/************************************************************* Function Declarations **************************************************/
/**************************************************************************************************************************************/
void initButtonStruct(ButtonStruct* buttonStruct); // alloco staticamente la memoria riservata alla struttura (questa è una alternativa alla allocazione dinamica della memoria riservata alla struttura che in C si fa con malloc)


/**************************************************************************************************************************************/

#endif /* PEN_BUTTON_H */