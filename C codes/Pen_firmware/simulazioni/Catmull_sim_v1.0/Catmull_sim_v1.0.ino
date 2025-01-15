/*
* main.ino
* Created on: August, 2024
* Author: Michele Di Lucchio
*/

/**********************************************************************************************************************************************************************************
******************************************************************************** INCLUDE *****************************************************************************************/
#include "spi_manager.h"
#include "usb_manager.h"
#include "pen_usb.h"
#include "pen_spi.h"
#include "fsm.h"
#include "pen_button.h"
#include "trajectory.h" 
#include "pin_map.h"

#include "data.h"
/*********************************************************************************************************************************************************************************/

/**********************************************************************************************************************************************************************************
********************************************************************************* MACRO ******************************************************************************************/
#define SPI_SPEED           100000 // Configura la velocità (1000000 = 1 MHz or 100000 = 100kHz)

/*********************************************************************************************************************************************************************************/

/**********************************************************************************************************************************************************************************
********************************************************************************* GLOBAL VARIABLES ********************************************************************************/

/*dichiarazione istanze delle strutture del sistema. Una istanza di una struttura è un OGGETTO specifico che utilizza il modello definito dalla struttura per contenere dati concreti.
* Una volta creata un'istanza di una struttura, è possibile accedere ai suoi membri e manipolarli come qualsiasi altra variabile.*/

SPIStruct penSpi; // oggetto per la comunicazione SPI master-sleave

// ***************************
// creazione di più istanze di una struttura, ognuna delle quali contiene i propri dati separati
LedBlinkerStruct ledOk; // led giallo frontale
LedBlinkerStruct ledFault; // led rosso frontale
LedBlinkerStruct ledOnOff; // led bottone
LedBlinkerStruct ledRed; // led rosso sotto
LedBlinkerStruct ledGreen; // led verde sotto


float SCALING_MOUSE = 1.0;              // Inizializzo SCALING_MOUSE: SCALING_MOUSE > 1 amplifica i movimenti del mouse, mentre SCALING_MOUSE < 1, i movimenti del laser sarebbero più piccoli per lo stesso movimento del mouse 
float* pScalingMouse = &SCALING_MOUSE;  // Definisco un puntatore a SCALING_MOUSE
unsigned long millis_prev = 0;          // per l'invio dei dati al manipolatore a 1kHz nello stato DRAW_RECORD

bool abort_sequence;

// variabili per il calcolo della ferquenza a cui gira il loop (9kHz quando il pennino non è collegato)
/*
unsigned long startMicros = 0;
unsigned long loopCounter = 0;*/


// Variabili per il movimento simulato
int simulatedXChange = 0;
int simulatedYChange = 0;
int currentIteration = 0;

int size = sizeof(simulatedDataX) /sizeof(simulatedDataX[0]);
int i = 0;
/*********************************************************************************************************************************************************************************/

void setup() {

  current_state = RECORDING; 

  Serial.begin(115200);

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  /******************************************************************************* INIZIALIZZAZIONE MODULI ***********************************************************************/

  initSPIStruct(&penSpi);

  initLedBlinkerStruct(&ledOk, LED_OK); // (led rosso frontale - serve per segnalere i movimenti lungo X)
  initLedBlinkerStruct(&ledFault, LED_FAULT); //  (led verde frontale - serve per segnalare i movimenti lungo Y)
  initLedBlinkerStruct(&ledOnOff, LED_ON_OFF_SWITCH);
  initLedBlinkerStruct(&ledRed, LED_USER_RED); // (led rosso sotto - serve per segnalare avviamento codice)
  initLedBlinkerStruct(&ledGreen, LED_USER_GREEN); // (led verde sotto - serve per segnalare l'invio di dati)
  
  traj_record = init_trajectory_struct();

  if (traj_record == NULL) {

    // Se l'allocazione della memoria fallisce, entra in un ciclo infinito
    Serial.println("Errore: allocazione della memoria fallita. Blocco esecuzione!.");

    while (true) {
      digitalWrite(BUZZER, !digitalRead(BUZZER));
      delay(500);
    }

  }

  delay(40000); // necessario per aprire un terminale Putty

}

void loop() {

  checkCurrentState();

  //Serial.print("Stato attuale: ");
  //Serial.println(current_state); // Mostra lo stato corrente

  if(current_state == RECORDING){

    abort_sequence = false;
      
    // Controlla se abbiamo raggiunto il numero totale di iterazioni
    if (currentIteration < TRAJECTORY_MAX_SIZE) {

      simulatedXChange = simulatedDataX[i];
      simulatedYChange = simulatedDataY[i];

      // Chiama la funzione mouseMoved con gli spostamenti simulati
      mouseMoved(simulatedXChange, simulatedYChange);
      
      // Incrementa il contatore delle iterazioni
      currentIteration++;
      i++;

      Serial.print("iteration:");
      Serial.print(currentIteration);
      Serial.print(",i:");
      Serial.println(i);

    } else {

      // Se la simulazione è finita, passa allo stato DRAW_RECORD
      current_state = DRAW_RECORD;

      //Serial.println("Contenuto del vettore pos_x_double:");
      //for (int i = 0; i < traj_record->current_index_record; i++) {
        //Serial.print("pos_x_double[");
        //Serial.print(i);
        //Serial.print("]: ");
        //Serial.println(traj_record->pos_x_double[i]);
      //}
      
      traj_record->pointX = traj_record->index_values[traj_record->current_size - 1];
      Serial.print("Initial_pointX: ");
      Serial.println(traj_record->pointX);
      delay(1000);
      
    }

  }else if(current_state == DRAW_RECORD){

    /* In this case, the position is send at 1kHz ovvero ogni 1ms.*/
    if (millis_prev + STREAMING_PERIOD_ms  <= millis()){
      
      millis_prev = millis();

      handlePenMotionAndSendSPI(&penSpi);

    }
  }

}