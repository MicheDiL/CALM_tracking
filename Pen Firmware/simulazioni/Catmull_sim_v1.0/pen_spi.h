/*
* pen_spi.h
*
* Created on: August, 2024
*   Author: Michele Di Lucchio
*   Description: Gestione comunicazione di test SPI master-slave
*/

#ifndef PEN_SPI_H
#define PEN_SPI_H

/************************************************************** Define macros *********************************************************/
/**************************************************************************************************************************************/
#define DATA_INITIALIZATION   0xAAAA
#define DATA_ZERO_POINT       0xBBBB
#define TIMEOUT_DURATION      60000  // Tempo di timeout (in millisecondi) entro cui deve essere stabilita la comunicazione di test SPI
#define INIT_STATE            0x1111 // 4369:  codifica dello stato Init dello slave
#define PEN_INPUT_STATE       0x5555 // 21845: codifica dello stato PenInput dello slave
#define SPI_READY             0x2222 // 8738:  codifica stato SpiReady per iniziare la trasmissione dei dati pennino
#define BLINK_PERIOD          500    // ms
#define BUFFER_SIZE           10     // filtro a media mobile sugli ultimi 50 samples
//#define INTERP_STEP           50000
/**************************************************************************************************************************************/

/************************************************************* Include Files **********************************************************/
/**************************************************************************************************************************************/
#include <Arduino.h>
#include "pin_map.h"
#include "spi_manager.h"
#include "led_blinker.h"
#include "usb_manager.h"

/**************************************************************************************************************************************/

/************************************************************* Type Definitions *******************************************************/
/**************************************************************************************************************************************/

struct SPIStruct
{
  // variabili per la comunicazione di test
  unsigned long startTime;
  uint16_t rx_test[1];
  bool slaveReady;

  // variabili per la fase operativa
  int32_t rollTX;
  int32_t pitchTX;
  //int32_t rollRX;
  //int32_t pitchRX;
  int32_t delta_roll; // Spostamento del mouse lungo X
  int32_t delta_pitch; // Spostamento del mouse lungo Y
    // Buffer per i campioni
  int32_t rollBuffer[BUFFER_SIZE];
  int32_t pitchBuffer[BUFFER_SIZE];
    // Indice corrente nel buffer
  int bufferIndex;
  int sampleCount;
};

/**************************************************************************************************************************************/

/************************************************************* Function Declarations **************************************************/
/**************************************************************************************************************************************/
void initSPIStruct(SPIStruct* spiStruct); // alloco staticamente la memoria riservata alla struttura (questa è una alternativa alla allocazione dinamica della memoria riservata alla struttura che in C si fa con malloc)
void testAndMonitorSPICommunication(SPIStruct* spiStruct);
void handlePenMotionAndSendSPI(SPIStruct* spiStruct);
void checkCurrentState();
void prepareSPIData(int32_t roll, int32_t pitch, uint16_t* txData);
void send_to_manipulator(SPIStruct* spiStruct);
void deleteOutliers(SPIStruct* spiStruct);
/**************************************************************************************************************************************/

#endif /* PEN_SPI_H */
