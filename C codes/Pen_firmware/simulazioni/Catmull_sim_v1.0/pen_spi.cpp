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
      spiStruct->rollTX = spiStruct->delta_roll; // i dati registrati sono già ASSOLUTI
      spiStruct->pitchTX = spiStruct->delta_pitch; // i dati registrati sono già ASSOLUTI
      
      deleteOutliers(&penSpi);

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
      
      Serial.print("x:");
      Serial.print(spiStruct->rollTX);
      ////Serial.print("\t");  
      Serial.print(",y:");
      Serial.println(spiStruct->pitchTX);

      //Serial.print(120);
      //Serial.print(", ");
      //Serial.print(-120);
      //Serial.print(", ");
      //Serial.println(spiStruct->rollTX);

      //delay(250);

    break;

    case DRAW_RECORD:

      Serial.print("x_interp:");
      Serial.print(spiStruct->rollTX);
      ////Serial.print("\t");    
      Serial.print(",y_interp:");
      Serial.println(spiStruct->pitchTX);

      //Serial.print(120);
      //Serial.print(", ");
      //Serial.print(-120);
      //Serial.print(", ");
      //Serial.println(spiStruct->rollTX);

      //delay(100);

    break;

  }

  

}

void deleteOutliers(SPIStruct* spiStruct){
  // Aggiungi il nuovo campione al buffer, sovrascrivendo il campione più vecchio
  spiStruct->rollBuffer[spiStruct->bufferIndex] = spiStruct->rollTX;
  spiStruct->pitchBuffer[spiStruct->bufferIndex] = spiStruct->pitchTX;

  // Aggiorna il numero di campioni finché non raggiunge il buffer completo
  if(spiStruct->sampleCount < BUFFER_SIZE){
    spiStruct->sampleCount++;
  }

  // Calcola la media dei campioni solo se il buffer è pieno
  if(spiStruct->sampleCount == BUFFER_SIZE){
    int32_t rollSum = 0;
    int32_t pitchSum = 0;

    for(int i = 0; i < BUFFER_SIZE; i++){
      rollSum += spiStruct->rollBuffer[i];
      pitchSum += spiStruct->pitchBuffer[i];
    }

    // Imposta i valori di roll e pitch alla media dei campioni
    spiStruct->rollTX = (rollSum / BUFFER_SIZE);
    spiStruct->pitchTX = (pitchSum / BUFFER_SIZE);

  }

  // Aggiorna l'indice del buffer circolare
  spiStruct->bufferIndex = (spiStruct->bufferIndex + 1) % BUFFER_SIZE; // facciamo "ritornare" l'indice a zero quando raggiunge il valore massimo del buffer, cioè BUFFER_SIZE

}