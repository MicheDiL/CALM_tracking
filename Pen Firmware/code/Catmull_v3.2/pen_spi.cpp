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
  
  // Inizializzazione a zero dei buffer del filtro mediano usando memset
  memset(spiStruct->rollMedianBuffer, 0, sizeof(spiStruct->rollMedianBuffer));
  memset(spiStruct->pitchMedianBuffer, 0, sizeof(spiStruct->pitchMedianBuffer)); 
  spiStruct->rollMedian = 0;
  spiStruct->pitchMedian = 0;
  spiStruct->medianIndex = 0;
  spiStruct->medianSampleCount = 0;
  // Inizializzazione a zero dei buffer del filtro a media mobile usando memset
  memset(spiStruct->rollMovingAvgBuffer, 0, sizeof(spiStruct->rollMovingAvgBuffer));
  memset(spiStruct->pitchMovingAvgBuffer, 0, sizeof(spiStruct->pitchMovingAvgBuffer)); 
  spiStruct->movingAvgIndex = 0;
  spiStruct->movingAvgSampleCount = 0;
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
extern double query_points_dist;

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
      
      optimizeData(&penSpi);
      
      if(current_state == RECORDING){
        // qui il flag send_coordinate non ha alcuna utilità rispetto alla versione simulata del codice.
        send_coordinate = record_trajectory(traj_record, spiStruct->rollTX, spiStruct->pitchTX, MIN_EUCLID_DIST);     
      }
    
    break;

    case DRAW_RECORD:

      abort_sequence = !read_and_interp_trajectory(traj_record, &spiStruct->rollTX, &spiStruct->pitchTX, query_points_dist);
      
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

  spiStruct->roll_micro = spiStruct->rollTX * 1000/38; // converti in mm le posizioni assolute (roll)
  spiStruct->pitch_micro = spiStruct->pitchTX * 1000/38; // converti in mm le posizioni assolute (pitch)
  prepareSPIData(spiStruct->roll_micro, spiStruct->pitch_micro, tx);

  //prepareSPIData(spiStruct->rollTX, spiStruct->pitchTX, tx);

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

    case ZERO_POINT:
      setLed(&ledOnOff, false);
      setLed(&ledFault, false);
    break;

  }
  
}
// =====================================================================================================
// =====================================================================================================

void mouseMoved() {

  // Ottieni i cambiamenti di movimento dal MouseController
  int32_t xChange_micro = (usbManager.mouse.getXChange()); //*1000/30; // restituisce la variazione dell'asse X del mouse rispetto alla sua posizione precedente
  int32_t yChange_micro = (usbManager.mouse.getYChange()); //*1000/30; // restituisce la variazione dell'asse Y del mouse rispetto alla sua posizione precedente

  //Aggiorna le variabili delta_roll e delta_pitch
  penSpi.delta_roll = -(xChange_micro * SCALING_MOUSE); // bisognerebbe invertire il segno della coordinata
  penSpi.delta_pitch = -(yChange_micro * SCALING_MOUSE); // bisognerebbe invertire il segno della coordinata

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
      //SPI1.endTransaction();

      //
      delayMicroseconds(1);

      for(int i = 0; i < sizeof(tx) / sizeof(tx[0]); i++){ // Itera sul numero di elementi del vettore tx

        digitalWrite(spiManager->csPin, LOW);     // Seleziona lo slave
        SPI1.transfer16(tx[i]);           // Transfer block:  Trasmette un blocco di 16 bit e riceve un blocco di 16 bit simultaneamente
        digitalWrite(spiManager->csPin, HIGH);    // Deseleziona lo slave
        delayMicroseconds(1);
      }

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

      /*digitalWrite(spiManager->csPin, LOW);     // Seleziona lo slave
      rx[0] = SPI1.transfer16(tx[0]);           // Transfer block 1:  Trasmette un blocco di 16 bit e riceve un blocco di 16 bit simultaneamente
      digitalWrite(spiManager->csPin, HIGH);    // Deseleziona lo slave
      delayMicroseconds(1);

      digitalWrite(spiManager->csPin, LOW);
      rx[1] = SPI1.transfer16(tx[1]);           // Transfer block 2
      digitalWrite(spiManager->csPin, HIGH);
      delayMicroseconds(1);

      digitalWrite(spiManager->csPin, LOW);
      rx[2] = SPI1.transfer16(tx[2]);           // Transfer block 3
      digitalWrite(spiManager->csPin, HIGH);
      delayMicroseconds(1);

      digitalWrite(spiManager->csPin, LOW);
      rx[3] = SPI1.transfer16(tx[3]);           // Transfer block 4
      digitalWrite(spiManager->csPin, HIGH);*/

      SPI1.endTransaction();
  }

}

void prepareSPIData(int32_t roll, int32_t pitch, uint16_t* txData){

  txData[0] = (int16_t) (0x8000 | (roll & 0xFFF));                  // ROLL LSB -> txData[0] = 0x8XXX
  txData[1] = (int16_t) (0xC000 | ((roll & 0xFFF000) >> 12));       // ROLL MSB -> txData[1] = 0xCXXX
  txData[2] = (int16_t) (0x0000 | (pitch & 0xFFF));                 // PITCH LSB -> txData[2] = 0x0XXX
  txData[3] = (int16_t) (0x4000 | ((pitch & 0xFFF000) >> 12));      // PITCH MSB -> txData[3] = 0x4XXX
}

void optimizeData(SPIStruct* spiStruct){

  // Step 1: Aggiungere i nuovi campioni al buffer mediano
  spiStruct->rollMedianBuffer[spiStruct->medianIndex] = spiStruct->rollTX;
  spiStruct->pitchMedianBuffer[spiStruct->medianIndex] = spiStruct->pitchTX;

  // Incrementa il contatore per il buffer mediano
  if (spiStruct->medianSampleCount < MEDIAN_BUFFER_SIZE) {
    spiStruct->medianSampleCount++;
  }

  // Calcolo della mediana solo se il buffer mediano è pieno
  spiStruct->rollMedian = spiStruct->rollTX;
  spiStruct->pitchMedian = spiStruct->pitchTX;

  if (spiStruct->medianSampleCount == MEDIAN_BUFFER_SIZE) {
    // Copia e ordina i buffer mediani
    int32_t tempRoll[MEDIAN_BUFFER_SIZE];
    int32_t tempPitch[MEDIAN_BUFFER_SIZE];
    
    for (int i = 0; i < MEDIAN_BUFFER_SIZE; i++) {
      tempRoll[i] = spiStruct->rollMedianBuffer[i];
      tempPitch[i] = spiStruct->pitchMedianBuffer[i];
    }

    std::sort(tempRoll, tempRoll + MEDIAN_BUFFER_SIZE); // gli ingressi della funzione sort sono puntatori al primo e ultimo elemento di un array da ordinare
    std::sort(tempPitch, tempPitch + MEDIAN_BUFFER_SIZE);

    // Prendi la mediana
    spiStruct->rollMedian = tempRoll[MEDIAN_BUFFER_SIZE / 2];
    spiStruct->pitchMedian = tempPitch[MEDIAN_BUFFER_SIZE / 2];
  }

  // Aggiorna l'indice del buffer mediano
  spiStruct->medianIndex = (spiStruct->medianIndex + 1) % MEDIAN_BUFFER_SIZE;

  // Step 2: Applicare il filtro a media mobile sui dati "filtrati" con il filtro mediano
  deleteNoise(&penSpi);
}

void deleteNoise(SPIStruct* spiStruct){
  // Aggiungi il nuovo campione al buffer, sovrascrivendo il campione più vecchio
  spiStruct->rollMovingAvgBuffer[spiStruct->movingAvgIndex] = spiStruct->rollMedian;
  spiStruct->pitchMovingAvgBuffer[spiStruct->movingAvgIndex] = spiStruct->pitchMedian;

  // Aggiorna il numero di campioni finché non raggiunge il buffer completo
  if(spiStruct->movingAvgSampleCount < MOVING_AVG_BUFFER_SIZE){
    spiStruct->movingAvgSampleCount++;
  }

  // Calcola la media dei campioni solo se il buffer è pieno
  if(spiStruct->movingAvgSampleCount == MOVING_AVG_BUFFER_SIZE){
    int32_t rollSum = 0;
    int32_t pitchSum = 0;

    for(int i = 0; i < MOVING_AVG_BUFFER_SIZE; i++){
      rollSum += spiStruct->rollMovingAvgBuffer[i];
      pitchSum += spiStruct->pitchMovingAvgBuffer[i];
    }

    // Imposta i valori di roll e pitch alla media dei campioni
    spiStruct->rollTX = (rollSum / MOVING_AVG_BUFFER_SIZE);
    spiStruct->pitchTX = (pitchSum / MOVING_AVG_BUFFER_SIZE);

  }

  // Aggiorna l'indice del buffer circolare
  spiStruct->movingAvgIndex = (spiStruct->movingAvgIndex + 1) % MOVING_AVG_BUFFER_SIZE; // facciamo "ritornare" l'indice a zero quando raggiunge il valore massimo del buffer, cioè BUFFER_SIZE

}