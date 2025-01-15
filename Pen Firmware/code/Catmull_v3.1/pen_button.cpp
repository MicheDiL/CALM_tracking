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

void initButtonStruct(ButtonStruct* buttonStruct){

  buttonStruct->button = NOT_PRESSED;
  buttonStruct->PressTime = 0; 
  buttonStruct->buttonClickCount = 0;
  
}

/************************************************************************* FUNZIONI WEAK DI MouseController **************************************************************/

extern USBManager   usbManager;
extern ButtonStruct rightButton;
extern ButtonStruct leftButton;
extern ButtonStruct middleButton;

extern float* pScalingMouse;  // Dichiara che pScalingMouse è definito altrove


void mousePressed(){

  if (leftButton.button == NOT_PRESSED){ // Controlla che il pulsante sinistro non sia premuto

    if(usbManager.mouse.getButton(RIGHT_BUTTON) && rightButton.button == NOT_PRESSED){
      rightButton.button = PRESSED;
      rightButton.PressTime = millis(); 
    }

    if(usbManager.mouse.getButton(MIDDLE_BUTTON) && middleButton.button == NOT_PRESSED){
      middleButton.button = PRESSED;
      middleButton.PressTime = millis();

    }
  }  
  
  if(rightButton.button == NOT_PRESSED && middleButton.button == NOT_PRESSED){
    
    if(usbManager.mouse.getButton(LEFT_BUTTON) && leftButton.button == NOT_PRESSED){
      leftButton.button = PRESSED;
      leftButton.PressTime = millis();

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

    } else {

      rightButton.button = LONG_PRESSED; // Pressione lunga

      if(current_state == FREE_HAND){
        *pScalingMouse = 1.0;               // Reset del fattore di scala
      }

    }

    rightButton.button = NOT_PRESSED; // Reset stato alla fine
    
  }

  // Gestione del pulsante centrale
  if (!usbManager.mouse.getButton(MIDDLE_BUTTON) && middleButton.button == PRESSED) {

    unsigned long elapsedTime = millis() - middleButton.PressTime;

    if (elapsedTime <= DEBOUNCE_DELAY) {
      
      middleButton.button = CLICKED; // Click rapido
      *pScalingMouse *= 2;

      // Controlla che SCALING_MOUSE non superi il limite massimo
      if (*pScalingMouse > MAX_SCALING_MOUSE) *pScalingMouse = MAX_SCALING_MOUSE;

    } else {

      middleButton.button = LONG_PRESSED; // Pressione lunga

      if(current_state == FREE_HAND){
        *pScalingMouse = 1.0;               // Reset del fattore di scala
      }
      
    }

    middleButton.button = NOT_PRESSED; // Reset stato alla fine

  }

  // Gestione del pulsante sinistro
  if (!usbManager.mouse.getButton(LEFT_BUTTON) && leftButton.button == PRESSED) {

    unsigned long elapsedTime = millis() - leftButton.PressTime;

    if (elapsedTime <= DEBOUNCE_DELAY) {
      
      leftButton.button = CLICKED; // Click rapido

      if(current_state == DRAW_RECORD){

        //reinit_trajectory(traj_record);
        cleanup_trajectory();
        
        current_state = FREE_HAND;
        
        if (traj_record == NULL) {
          traj_record = init_trajectory_struct();  // Rialloca memoria se è stata liberata
        }
        
      }

      if(current_state == FREE_HAND){
        
        current_state = ZERO_POINT;
        
      }

    } else {

      leftButton.button = LONG_PRESSED; // Pressione lunga
      
      if(current_state == RECORDING) {
        
        traj_record->pointX = traj_record->index_values[traj_record->current_size - 1];

        current_state = DRAW_RECORD;

      }

    }

    leftButton.button = NOT_PRESSED; // Reset stato alla fine
    
  }

}