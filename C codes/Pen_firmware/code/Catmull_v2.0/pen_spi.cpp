/*
* pen_spi.cpp
*
* Created on: August, 2024
*   Author: Michele Di Lucchio
*    
*/

#include "pen_spi.h"
#include "fsm.h"        // per accedere a curremt_state
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

extern SPIManager       spiManager;
extern SPIStruct        penSpi;

extern USBManager       usbManager;

extern uint16_t rx[4];
extern uint16_t tx[4]; 

extern float SCALING_MOUSE;

extern bool abort_sequence;
extern bool send_coordinate;

void testAndMonitorSPICommunication(SPIStruct* spiStruct){ 
  
  current_state = INITIALIZATION;

  Serial.println("Inzializzazione");


  spiStruct->startTime = millis(); // imposta il tempo attuale in ms in cui inizia la comunicazione di test

  //delay(100); //delay(500);

  do {
    
    send_to_manipulator(&spiManager);    

    
    Serial.println("");
    Serial.println("Dati SPI (decimali):");
    Serial.print("tx: ");
    Serial.print(DATA_INITIALIZATION);
    Serial.print("\t");
    Serial.print("rx: ");
    Serial.print(spiStruct->rx_test[0]);
    Serial.print("\n");


    if (spiStruct->rx_test[0] == PEN_INPUT_STATE) {

      setLed(&ledFault, LOW);
      setLed(&ledOk, HIGH);

      Serial.println("");
      Serial.println("PenInput State.");
 

    } else if (spiStruct->rx_test[0] == INIT_STATE) {

      setLed(&ledFault, HIGH); // LED ROSSO
      setLed(&ledOk, LOW);

      Serial.println("");
      Serial.println("Init state");

    } else if (spiStruct->rx_test[0] == SPI_READY) {

      setLed(&ledFault, LOW);
      setLed(&ledOk, LOW);
      setLed(&ledOnOff, HIGH);

      Serial.println("");
      Serial.println("SPI ready");
 
      // iniziliazzazione terminata con successo
      spiStruct->slaveReady = true;
      current_state = FREE_HAND;

    }

    if (millis() - spiStruct->startTime > TIMEOUT_DURATION) { // controlla il tempo trascorso confrontando il valore attuale di millis() con startTime.
      
      // iniziliazzazione fallita
      spiStruct->slaveReady = false;
      
      break; // Se, per qualsiasi motivo, lo slave non risponde entro il tempo specificato da TIMEOUT_DURATION, si esce dal ciclo do-while e si esegue la riga 126
    }

  } while (current_state == INITIALIZATION);

  if (current_state == INITIALIZATION){

    current_state = ERROR;

    Serial.println("Slave not responding. Communication failed.");

  }
}

void handlePenMotionAndSendSPI(SPIStruct* spiStruct){

  switch(current_state){

    case FREE_HAND:
    case RECORDING:

      // accumulo gli spostamneti micrometrici dx e dy catturati dalle chiamate di getXChange() e getYChange()
      spiStruct->rollTX += spiStruct->delta_roll;
      spiStruct->pitchTX += spiStruct->delta_pitch;
      
      deleteOutliers(&penSpi);
      
      if(current_state == RECORDING){
        // qui il flag send_coordinate non ha alcuna utilità rispetto alla versione simulata del codice.
        send_coordinate = record_trajectory(traj_record, spiStruct->rollTX, spiStruct->pitchTX, MIN_EUCLID_DIST);     
      }
    
    break;

    case DRAW_RECORD:

      abort_sequence = !read_and_interp_trajectory(traj_record, &spiStruct->rollTX, &spiStruct->pitchTX, TRAJECTORY_MAX_SIZE/3);
      
    break;  

    case ZERO_POINT: // per ora non utilizziamo mai questo stato

      spiStruct->rollTX = 0;
      spiStruct->pitchTX = 0;
      
      current_state = FREE_HAND;

    break;

  }

  // Resetto i buffer per la SPI 
  rx[0] = 0; tx[0] = 0;
  rx[1] = 0; tx[1] = 0;
  rx[2] = 0; tx[2] = 0;
  rx[3] = 0; tx[3] = 0;

  prepareSPIData(spiStruct->rollTX, spiStruct->pitchTX, tx);

  send_to_manipulator(&spiManager);
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

void mouseMoved() {

  // Ottieni i cambiamenti di movimento dal MouseController
  int32_t xChange_micro = (usbManager.mouse.getXChange())*1000/30; // restituisce la variazione dell'asse X del mouse rispetto alla sua posizione precedente
  int32_t yChange_micro = (usbManager.mouse.getYChange())*1000/30; // restituisce la variazione dell'asse Y del mouse rispetto alla sua posizione precedente

  //Aggiorna le variabili delta_roll e delta_pitch
  penSpi.delta_roll = (xChange_micro * SCALING_MOUSE); // bisognerebbe invertire il segno della coordinata
  penSpi.delta_pitch = (yChange_micro * SCALING_MOUSE); // bisognerebbe invertire il segno della coordinata

  handlePenMotionAndSendSPI(&penSpi);
}

void mouseDragged(){

  current_state = RECORDING;
  
  mouseMoved();

}

/*Send commands to the manipulator, by SPI*/ 

void send_to_manipulator(SPIManager* spiManager){

  switch(current_state){

    case INITIALIZATION:
      SPI1.beginTransaction(spiManager->spiSettings);
      digitalWrite(spiManager->csPin, LOW);                                 // Seleziona lo slave
      penSpi.rx_test[0] = SPI1.transfer16(DATA_INITIALIZATION); 
      digitalWrite(spiManager->csPin, HIGH);                                // Deseleziona lo slave
      SPI1.endTransaction(); 
    break;

    case ZERO_POINT:
      SPI1.beginTransaction(spiManager->spiSettings);
      digitalWrite(spiManager->csPin, LOW);                                 // Seleziona lo slave
      SPI1.transfer16(DATA_ZERO_POINT);
      digitalWrite(spiManager->csPin, HIGH);                                // Deseleziona lo slave
      SPI1.endTransaction();
    break;

    default:

      SPI1.beginTransaction(spiManager->spiSettings);

      for(int i = 0; i < sizeof(tx) / sizeof(tx[0]); i++){ // Itera sul numero di elementi del vettore tx

        digitalWrite(spiManager->csPin, LOW);     // Seleziona lo slave
        rx[i] = SPI1.transfer16(tx[i]);           // Transfer block:  Trasmette un blocco di 16 bit e riceve un blocco di 16 bit simultaneamente
        digitalWrite(spiManager->csPin, HIGH);    // Deseleziona lo slave
        delayMicroseconds(1);
      }

      SPI1.endTransaction();
  }

}

void prepareSPIData(int32_t roll, int32_t pitch, uint16_t* txData){

  txData[0] = (int16_t) (0x8000 | (roll & 0xFFF));                  // ROLL LSB -> txData[0] = 0x8XXX
  txData[1] = (int16_t) (0xC000 | ((roll & 0xFFF000) >> 12));       // ROLL MSB -> txData[1] = 0xCXXX
  txData[2] = (int16_t) (0x0000 | (pitch & 0xFFF));                 // PITCH LSB -> txData[2] = 0x0XXX
  txData[3] = (int16_t) (0x4000 | ((pitch & 0xFFF000) >> 12));      // PITCH MSB -> txData[3] = 0x4XXX
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