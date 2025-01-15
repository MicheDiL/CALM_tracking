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

// Variabili globali per il movimento circolare
float angle = 0.0;           // Angolo iniziale in radianti
const float radius = 5.0;   // Raggio del cerchio
const float angularSpeed = 0.1; // Velocità angolare (radians per loop)
int currentIteration = 0;         // Contatore delle iterazioni

// Variabili per il movimento simulato
int simulatedXChange = 0;
int simulatedYChange = 0;

// Intervallo di rumore (puoi modificarlo in base alle tue esigenze)
const int NOISE_RANGE = 10; // Rumore massimo di +/- 10 unità

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
}

void loop() {

  checkCurrentState();

  //Serial.print("Stato attuale: ");
  //Serial.println(current_state); // Mostra lo stato corrente

  if(current_state == RECORDING){

    abort_sequence = false;
      
    // Controlla se abbiamo raggiunto il numero totale di iterazioni
    if (currentIteration < TRAJECTORY_MAX_SIZE) {

      // Calcola le variazioni dx e dy usando le funzioni trigonometriche
      simulatedXChange = (int)(radius * cos(angle)) + random(-NOISE_RANGE, NOISE_RANGE + 1); // Spostamento X con rumore; // Spostamento X
      simulatedYChange = (int)(radius * sin(angle)) + random(-NOISE_RANGE, NOISE_RANGE + 1); // Spostamento Y con rumore; // Spostamento Y

      // Incrementa l'angolo per il prossimo movimento
      angle += angularSpeed; // Modifica questa variabile per cambiare la velocità della traiettoria
      if (angle > TWO_PI) {  // Riporta l'angolo nell'intervallo [0, 2*PI]
        angle -= TWO_PI;
      }

      // Chiama la funzione mouseMoved con gli spostamenti simulati
      mouseMoved(simulatedXChange, simulatedYChange);
      
      // Incrementa il contatore delle iterazioni
      currentIteration++;

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
      //Serial.print("Initial_pointX: ");
      //Serial.println(traj_record->pointX);
      
    }

  }else if(current_state == DRAW_RECORD){

    /* In this case, the position is send at 1kHz ovvero ogni 1ms.*/
    //if (millis_prev + STREAMING_PERIOD_ms  <= millis()){
      
      //millis_prev = millis();

      handlePenMotionAndSendSPI(&penSpi);

    //}
  }

}
