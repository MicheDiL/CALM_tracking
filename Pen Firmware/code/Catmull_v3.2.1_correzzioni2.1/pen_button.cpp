/*
* pen_button.cpp
*
* Created on: August, 2024
*   Author: Michele Di Lucchio
*    
*/

#include "pen_button.h"
#include "fsm.h"
#include "trajectory.h"

#include "pin_map.h" // per DEBUG
#include "pen_spi.h"

void initButtonStruct(ButtonStruct* buttonStruct){

  buttonStruct->button = NOT_PRESSED;
  buttonStruct->PressTime = 0; 
  buttonStruct->buttonClickCount = 0;
  
}

/************************************************************************************************************************************************************************/

extern USBManager   usbManager;
extern ButtonStruct rightButton;
extern ButtonStruct leftButton;
extern ButtonStruct middleButton;

extern float* pScalingMouse;  // Dichiara che pScalingMouse è definito altrove

extern SPIStruct        penSpi;
extern uint8_t* ptr_flag;

extern float* pScalingStreamPeriod;

/************************************************************************* FUNZIONI WEAK DI MouseController **************************************************************/


void mousePressed(){

  if (leftButton.button == NOT_PRESSED){ // Controlla che il pulsante sinistro non sia premuto

    if(usbManager.mouse.getButton(LEFT_BUTTON) && leftButton.button == NOT_PRESSED){

      if (current_state == DRAW_RECORD || current_state == OVERFLOW_TRAJ) {
        *ptr_flag = 1;  // mi dice che i dati dell'ultima posizione sono in attesa di essere inviati al manipolatore
      }else{
        *ptr_flag = 3; // pressione sul pulsante sinistro generica
      }
      leftButton.button = PRESSED;
      leftButton.PressTime = millis();
    }

  }  

  if (middleButton.button == NOT_PRESSED){

    if(usbManager.mouse.getButton(MIDDLE_BUTTON) && middleButton.button == NOT_PRESSED){
      middleButton.button = PRESSED;
      middleButton.PressTime = millis();
      *ptr_flag = 5; // pressione per regolazione dinamica dello zoom
    }
  }

  
  if(rightButton.button == NOT_PRESSED){
    
    if(usbManager.mouse.getButton(RIGHT_BUTTON) && rightButton.button == NOT_PRESSED){
      rightButton.button = PRESSED;
      rightButton.PressTime = millis(); 
      *ptr_flag = 5; // pressione per regolazione dinamica dello zoom
    }
    
  }
  
}

void mouseReleased() {

  // Gestione del pulsante destro 
  if (!usbManager.mouse.getButton(RIGHT_BUTTON) && rightButton.button == PRESSED) {

    unsigned long elapsedTime = millis() - rightButton.PressTime;
    
    if (elapsedTime <= DEBOUNCE_DELAY) {

      rightButton.button = CLICKED; // Click rapido
      *pScalingMouse /= 2;
            
      // Controlla che SCALING_MOUSE non sia inferiore al limite minimo
      if (*pScalingMouse < MIN_SCALING_MOUSE) *pScalingMouse = MIN_SCALING_MOUSE;

      if(current_state == DRAW_RECORD){
        *pScalingStreamPeriod /= 2;
      }

    }else {

      rightButton.button = LONG_PRESSED; // Pressione lunga

      if(current_state == FREE_HAND){
        *pScalingMouse = 1.0;               // Reset del fattore di scala
      }

      if(current_state == DRAW_RECORD){
        *pScalingStreamPeriod = 4000.0;
      }

    }

    rightButton.button = NOT_PRESSED; // Reset stato alla fine
    *ptr_flag = 6; // ritorna allo stato di default
    
  }

  // Gestione del pulsante centrale
  if (!usbManager.mouse.getButton(MIDDLE_BUTTON) && middleButton.button == PRESSED) {

    unsigned long elapsedTime = millis() - middleButton.PressTime;

    if (elapsedTime <= DEBOUNCE_DELAY) {
      
      middleButton.button = CLICKED; // Click rapido
      *pScalingMouse *= 2;

      // Controlla che SCALING_MOUSE non superi il limite massimo
      if (*pScalingMouse > MAX_SCALING_MOUSE) *pScalingMouse = MAX_SCALING_MOUSE;

      if(current_state == DRAW_RECORD){
        *pScalingStreamPeriod *=2;
      }

    }else {

      middleButton.button = LONG_PRESSED; // Pressione lunga

      if(current_state == FREE_HAND){
        *pScalingMouse = 1.0;               // Reset del fattore di scala
      }

      if(current_state == DRAW_RECORD){
        *pScalingStreamPeriod = 4000.0;
      }
      
    }

    middleButton.button = NOT_PRESSED; // Reset stato alla fine
    *ptr_flag = 6; // ritorna allo stato di default

  }

  // Gestione del pulsante sinistro
  if (!usbManager.mouse.getButton(LEFT_BUTTON) && leftButton.button == PRESSED) {

    unsigned long elapsedTime = millis() - leftButton.PressTime;

    if (elapsedTime <= DEBOUNCE_DELAY) {
      
      leftButton.button = CLICKED; // Click rapido per smettere di far ripetere la triaettoria

      /*if(current_state == DRAW_RECORD || current_state == OVERFLOW_TRAJ){

        current_state = SEND_LAST_POINT;
        *ptr_flag = 1; // mi dice che i dati dell'ultima posizione sono in attesa di essere inviati al manipolatore

        
      }else if(current_state == FREE_HAND){
        
        current_state = ZERO_POINT;

      }*/

      if(current_state == FREE_HAND){
        
        current_state = ZERO_POINT;

      }

    }else{

      leftButton.button = LONG_PRESSED; // Pressione lunga: vuol dire che abbiamo registrato
      
      /*if(current_state == RECORDING) {
        
        traj_record->pointX = traj_record->index_values[traj_record->current_size - 1];

        current_state = DRAW_RECORD;
        
      }*/

    }

    if(current_state == RECORDING) { // risolvo il problema di accendere ledfault se schiacchio accidentalmente pulsante sinistro a contatto con la tavoletta
        
      traj_record->pointX = traj_record->index_values[traj_record->current_size - 1];

      current_state = DRAW_RECORD;
        
    }

    if(current_state == OVERFLOW_TRAJ){
      //cleanup_trajectory();
      //current_state = FREE_HAND;
      handlePenMotionAndSendSPI(&penSpi);
      current_state = FREE_HAND;
    }

    // soluzione 1
    if(current_state == SEND_LAST_POINT){
      handlePenMotionAndSendSPI(&penSpi); // manda l'ultimo dato interpolato solo al rilascio del pulsante
      current_state = FREE_HAND;
    }

    leftButton.button = NOT_PRESSED; // Reset stato alla fine
    
  }

}