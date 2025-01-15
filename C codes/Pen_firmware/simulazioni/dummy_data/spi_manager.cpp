/*
* spi_manager.cpp
*
* Created on: August, 2024
*   Author: Michele Di Lucchio
*    
*/

#include "spi_manager.h"

void initSPIManager(SPIManager* spiManager, int csPin, uint32_t spiSpeed){
  
  spiManager->csPin = csPin;
  spiManager->spiSpeed = spiSpeed;
  spiManager->spiSettings = SPISettings(spiSpeed, MSBFIRST, SPI_MODE1); // Configura la velocità , il formato dei dati (MSBFIRST - Most Significant Bit First) e la modalità SPI (SPI_MODE1).

  pinMode(spiManager->csPin, OUTPUT); // SPI Chip Select (SAMD21 funge da master)
  digitalWrite(spiManager->csPin, HIGH); // deseleziona lo slave
  SPI1.begin(); // avvia il modulo SPI1 del micro

}
