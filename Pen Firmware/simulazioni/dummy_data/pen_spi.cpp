/*
* pen_spi.cpp
*
* Created on: August, 2024
*   Author: Michele Di Lucchio
*    
*/

#include "pen_spi.h"
#include "fsm.h" // per accedere a curremt_state
#include "trajectory.h" // per la gestione della traiettoria

void initSPIStruct(SPIStruct* spiStruct){
  
  // variabili per l'inizializzazione
  spiStruct->startTime = 0;
  spiStruct->rx_test[0] = {0};
  spiStruct->slaveReady = false;

  // variabili per la fase operativa
  spiStruct->rollTX = 0;
  spiStruct->pitchTX = 0;
  //spiStruct->rollRX = 0;
  //spiStruct->pitchRX = 0;
  spiStruct->delta_roll = 0;
  spiStruct->delta_pitch = 0;
  
  // Inizializzazione dei buffer a zero usando memset
  memset(spiStruct->rollBuffer, 0, sizeof(spiStruct->rollBuffer));
  memset(spiStruct->pitchBuffer, 0, sizeof(spiStruct->pitchBuffer)); 
  spiStruct->bufferIndex = 0;
  spiStruct->sampleCount = 0;
}

// ==============================================================================================================================================0
// ==============================================================================================================================================

extern LedBlinkerStruct ledOk; // led giallo frontale
extern LedBlinkerStruct ledFault; // led rosso frontale
extern LedBlinkerStruct ledOnOff; // led bottone 
extern LedBlinkerStruct ledGreen; // led verde sotto 
extern LedBlinkerStruct ledRed; // led rosso sotto 


extern SPIStruct        penSpi;


extern float SCALING_MOUSE;

extern bool abort_sequence;


void handlePenMotionAndSendSPI(SPIStruct* spiStruct){

  switch(current_state){

    case FREE_HAND:
    case RECORDING:

      // accumulo gli spostamneti micrometrici dx e dy catturati dalle chiamate di getXChange() e getYChange()
      spiStruct->rollTX += spiStruct->delta_roll;
      spiStruct->pitchTX += spiStruct->delta_pitch;
      
      //deleteOutliers(&penSpi);

      if(current_state == RECORDING){
        record_trajectory(traj_record, spiStruct->rollTX, spiStruct->pitchTX);    
      }
    
    break;

    case DRAW_RECORD:

      abort_sequence = !read_and_interp_trajectory(traj_record, &spiStruct->rollTX, &spiStruct->pitchTX, INTERPOLATION_DENSITY);
      
    break;  

    case ZERO_POINT: // per ora non utilizziamo mai questo stato

      spiStruct->rollTX = 0;
      spiStruct->pitchTX = 0;
      
      current_state = FREE_HAND;

    break;

  }

  send_to_manipulator(&penSpi);
}

void checkCurrentState(){

  switch(current_state){

    case FREE_HAND:
      setLed(&ledOnOff, true);
      setLed(&ledFault, false);
    break;

    case RECORDING:
      setLed(&ledOnOff, false);
      setLed(&ledFault, true);
    break;

    case DRAW_RECORD:
      ledBlink(&ledOnOff, &ledFault, BLINK_PERIOD);
    break;

  }
  
}
// =====================================================================================================
// =====================================================================================================

void mouseMoved(int xChange, int yChange) {

  //Aggiorna le variabili delta_roll e delta_pitch
  penSpi.delta_roll = (xChange * SCALING_MOUSE); // bisognerebbe invertire il segno della coordinata
  penSpi.delta_pitch = (yChange * SCALING_MOUSE); // bisognerebbe invertire il segno della coordinata

  handlePenMotionAndSendSPI(&penSpi);
}


void send_to_manipulator(SPIStruct* spiStruct){

  switch(current_state){

    case INITIALIZATION:
      
    break;

    case ZERO_POINT:
      
    break;

    case RECORDING:

      //Serial.print("dx: " );
      //Serial.print(penSpi.delta_roll);
      //Serial.print("\t");
      //Serial.print("dy: " );
      //Serial.println(penSpi.delta_pitch);
      
      //Serial.print("x:");
      //Serial.print(spiStruct->rollTX);
      ////Serial.print("\t");  
      //Serial.print(",y:");
      //Serial.println(spiStruct->pitchTX);

      Serial.print(120);
      Serial.print(", ");
      Serial.print(-120);
      Serial.print(", ");
      Serial.println(spiStruct->rollTX);

      delay(500);

    break;

    case DRAW_RECORD:

      //Serial.print("x_interp:");
      //Serial.print(spiStruct->rollTX);
      ////Serial.print("\t");    
      //Serial.print(",y_interp:");
      //Serial.println(spiStruct->pitchTX);

      Serial.print(120);
      Serial.print(", ");
      Serial.print(-120);
      Serial.print(", ");
      Serial.println(spiStruct->rollTX);

      delay(100);

    break;

  }

  

}

