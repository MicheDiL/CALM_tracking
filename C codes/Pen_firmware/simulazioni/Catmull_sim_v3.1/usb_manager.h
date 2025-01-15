/*
* usb_manager.h
*
* Created on: August, 2024
*   Author: Michele Di Lucchio
*   Description: Gestione protocollo USB 
*/

#ifndef USB_MANAGER_H
#define USB_MANAGER_H

/************************************************************* Define Macros **********************************************************/
/**************************************************************************************************************************************/

/**************************************************************************************************************************************/

/************************************************************* Include Files **********************************************************/
/**************************************************************************************************************************************/
#include "Arduino.h"
#include <MouseController.h> // Includi la libreria per il controllo del mouse

/**************************************************************************************************************************************/

/************************************************************* Type Definitions *******************************************************/
/**************************************************************************************************************************************/

struct USBManager { // alloco direttamente gli oggetti USBHost e MouseController all'interno della struttura USBManager. Questo significa che non è necessario gestire manualmente l'allocazione e la deallocazione della memoria (come sarebbe necessario con new e delete). La durata di questi oggetti è legata alla durata dell'istanza di USBManager, il che semplifica la gestione della memoria e riduce il rischio di memory leaks.
  
  USBHost usbHost;        /* usbHost è un oggetto che rappresenta l'host USB, il componente che gestisce la comunicazione con i dispositivi USB collegati e della gestione
                          *  al sistema. In pratica, l'oggetto usbHost è responsabile dell'inizializzazione del controller USB, dell'individuazione dei dispositivi collegati, 
                          *  della comunicazione con essi. Viene utilizzato per eseguire operazioni come il polling dei dispositivi USB per verificare se ci sono nuovi dati 
                          *  da elaborare. Viene utilizzato per eseguire operazioni come il polling dei dispositivi USB per verificare se ci sono nuovi dati da elaborare.
                          *  L'oggetto usbHost è allocato automaticamente all'interno della struttura USBManager, il che significa che la sua durata è legata alla durata dell'istanza di USBManager */
    
  MouseController mouse;  /* MouseController è una classe utilizzata per interfacciarsi con un dispositivo mouse USB collegato. Questa classe fornisce i metodi e le callback 
                          *  necessari per gestire gli eventi del mouse, come i movimenti, i clic e i rilasci dei pulsanti. Il costruttore di MouseController richiede un riferimento 
                          *  a un oggetto USBHost. L'oggetto mouse è anche allocato automaticamente, il che significa che la gestione della memoria è semplice e sicura.*/

    // Costruttore della struttura
    USBManager() : mouse(usbHost) {}  /* Il costruttore USBManager() inizializza il membro mouse, associandolo all'oggetto usbHost. Questa inizializzazione garantisce che il 
                                      *  MouseController sia correttamente legato all'host USB al momento della creazione dell'istanza USBManager. USBManager() è un costruttore 
                                      *  predefinito che viene eseguito automaticamente quando viene creata un'istanza di USBManager. Questo costruttore nizializzail membro mouse 
                                      *  con l'oggetto usbHost. Questo è essenziale perché MouseController richiede un riferimento a USBHost al momento della costruzione. */
};

/**************************************************************************************************************************************/

/************************************************************* Function Declarations **************************************************/
/**************************************************************************************************************************************/
void initUSBManager(USBManager* usbManager);
void updateUSB(USBManager* usbManager);

// Funzioni di callback per gestire gli eventi del mouse. Sarebbero le funzioni "weak" della libreria MouseController. Dichiarandole qui le rendo visibili agli altri file che includono usb_manager.h (tipo pen_motion.h)
void mouseMoved(int xChange, int yChange); // definita in pen_spi.cpp
void mousePressed(); // definita in pen_button.cpp
void mouseReleased(); // definita in pen_button.cpp
void mouseDragged(); // definita in pen_spi.cpp
/**************************************************************************************************************************************/

#endif /* USB_MANAGER_H */