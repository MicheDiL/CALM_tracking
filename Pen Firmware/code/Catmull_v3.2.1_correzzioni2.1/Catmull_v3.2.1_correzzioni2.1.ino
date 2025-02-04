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

// ***************************
// ******** PROTOCOLLI *******
// ***************************
SPIManager spiManager; // oggetto per la configurazione del protocollo SPI
USBManager usbManager; // oggetto per la configurazione del protocollo USB

// ***************************
// ******** PENNINO **********
// ***************************
SPIStruct penSpi; // oggetto per la comunicazione SPI master-sleave
USBStruct penUsb; // oggetto per l' abilitazione dell' host USB (pennino)

ButtonStruct rightButton;
ButtonStruct leftButton;
ButtonStruct middleButton;

// ***************************
// ******** PCB **************
// ***************************
// creazione di più istanze di una struttura, ognuna delle quali contiene i propri dati separati
LedBlinkerStruct ledOk; // led giallo frontale
LedBlinkerStruct ledFault; // led rosso frontale
LedBlinkerStruct ledOnOff; // led bottone
LedBlinkerStruct ledRed; // led rosso sotto
LedBlinkerStruct ledGreen; // led verde sotto

// ***************************
// ******** APPLICAZIONE *****
// ***************************
//AppStruct* myapp; // puntatore alla struttura AppStruct, rappresenta l'applicazione nel suo complesso raccogliendo tutte le strutture precedenti per gestire l'intero sistema

// motion related variables
uint16_t rx[4];
uint16_t tx[4];

float SCALING_MOUSE = 1.0;              // Inizializzo SCALING_MOUSE: SCALING_MOUSE > 1 amplifica i movimenti del mouse, mentre SCALING_MOUSE < 1, i movimenti del laser sarebbero più piccoli per lo stesso movimento del mouse 
float* pScalingMouse = &SCALING_MOUSE;  // Definisco un puntatore a SCALING_MOUSE
unsigned long millis_prev = 0;          // per l'invio dei dati al manipolatore a 1kHz nello stato DRAW_RECORD

bool abort_sequence;
bool send_coordinate;
double query_points_dist;

// variabili per il calcolo della ferquenza a cui gira il loop (9kHz quando il pennino non è collegato)
/*
unsigned long startMicros = 0;
unsigned long loopCounter = 0;*/

uint8_t flag = 6; // valore di default
uint8_t* ptr_flag = &flag;  

// variabili per risolvere il problema dello switch da DRAW_RECORD a FREE_HAND
int32_t last_interp_roll = 0;
int32_t* ptr_last_interp_roll = &last_interp_roll;

int32_t last_interp_pitch = 0;
int32_t* ptr_last_interp_pitch = &last_interp_pitch;

// variaili per risolvere il problema dello switch da OVERFLOW_TARJ a FREE_HAND
int32_t last_recorded_roll = 0;
int32_t* ptr_last_recorded_roll = &last_recorded_roll;

int32_t last_recorded_pitch = 0;
int32_t* ptr_last_recorded_pitch = &last_recorded_pitch;

// Variabili per controllare la velocità del DRAW_RECORD
float TIME_DIST = 4000.0;
float* pScalingStreamPeriod = &TIME_DIST;
/*********************************************************************************************************************************************************************************/

void setup() {

  Serial.begin(115200);

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  /******************************************************************************* INIZIALIZZAZIONE MODULI ***********************************************************************/
  initSPIManager(&spiManager, IPC_SPI_CS, SPI_SPEED); // inizializzazione del modulo SPI
  initUSBManager(&usbManager); // inizializzazione del modulo USB

  initSPIStruct(&penSpi);
  initUSBStruct(&penUsb, nENUSBV);

  initButtonStruct(&rightButton);
  initButtonStruct(&leftButton);
  initButtonStruct(&middleButton);
  

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

  /*********************************************************************************************************************************************************************************/

  testAndMonitorSPICommunication(&penSpi); // identificazione degli stati dello slave mandando un messaggio di test
  
  Serial.end();
}

void loop() {

  checkCurrentState();

  if(current_state != ERROR){ // Manipolatore pronto a processare gli input del pennino

    switch(current_state){

      case FREE_HAND:

        updateUSB(&usbManager);

      break;

      case RECORDING:
        /* Richiamo la funzione per eseguire il polling e gestire il dispositivo USB. In this case, 
        the position is send only when there is an input from the mouse, not at 1kHz.*/
        updateUSB(&usbManager);
        
      break;

      case ZERO_POINT:
        /* Richiamo la funzione per eseguire il polling e gestire il dispositivo USB. In this case, 
        the position is send only when there is an input from the mouse, not at 1kHz.*/
        updateUSB(&usbManager);
      break;

      case OVERFLOW_TRAJ: // QUESTA SOLUZIONE DEVO ANCORA VERIFICARLA
        
        //current_state = DRAW_RECORD;
        updateUSB(&usbManager);
        //handlePenMotionAndSendSPI(&penSpi);
        //return;

      break;

      case DRAW_RECORD:
        /* In this case, the position is send at 1kHz ovvero ogni 1ms.*/
        if (millis_prev + STREAMING_PERIOD_ms <= millis()){
          
          millis_prev = millis();

          updateUSB(&usbManager); // senza questo non potrei fermare la ripetizione della traiettoria poichè avrei disabilitato le callback del mouse
          handlePenMotionAndSendSPI(&penSpi);
          //setLed(&ledGreen, !ledGreen.isOn); // qui il codice ci arriva? SI

          if (current_state != SEND_LAST_POINT && !abort_sequence){
            current_state = FREE_HAND;
            cleanup_trajectory();
          }else if(current_state == SEND_LAST_POINT && !abort_sequence){
            return;
          }
        }
      break;

      case SEND_LAST_POINT:  //  ENTRA QUI!

        //setLed(&ledGreen, !ledGreen.isOn); // qui il codice ci arriva? SI
        updateUSB(&usbManager);
        //handlePenMotionAndSendSPI(&penSpi);  //  questa chiamata la facciamo fare non appena lascio il pulsante sinistro

      break;

    }

  } else {                    // Manipolatore non pronto a processare gli input del pennino

    while(true){

      /*digitalWrite(BUZZER, HIGH);
      delayMicroseconds(2000);
      digitalWrite(BUZZER, LOW);
      delayMicroseconds(2000);*/
      Serial.println("Manipolatore non risponde! \nRiavvio necessario!");
      disableUSBStruct(&penUsb, nENUSBV);

    }

  }
}
