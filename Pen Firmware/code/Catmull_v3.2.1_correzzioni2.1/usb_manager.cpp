/*
* usb_manager.cpp
*
* Created on: August, 2024
*   Author: Michele Di Lucchio
*    
*/

#include "usb_manager.h"

// Inizializzo l'USBManager con un USBHost e MouseController
void initUSBManager(USBManager* usbManager){

  //usbManager->usbHost->begin(); // inizializza l'host USB

}

// Funzione per aggiornare l'interfaccia USB (esegue il polling del mouse)
void updateUSB(USBManager* usbManager){
  //Serial.println("prima di task");
  //usbManager->usbHost->Task(); 
  usbManager->usbHost.Task(); // Aggiorna lo stato del controller USB, rilevando eventi come il movimento del mouse o il clic dei pulsanti.
  //Serial.println("dopo di task");
  
  // Le funzioni weak definite nella libreria MouseController saranno chiamate automaticamente
}


