/*
* pen_motion.h
*
* Created on: August, 2024
*   Author: Michele Di Lucchio
*   Description: General application functions 
*/

#ifndef APPLICATION_H
#define APPLICATION_H

/************************************************************** Define macros *********************************************************/
#define DEBUG
/* Macro for debug prints */
#ifdef DEBUG
#define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
#define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

/**************************************************************************************************************************************/

/**************************************************************************************************************************************/

/************************************************************* Include Files **********************************************************/
/**************************************************************************************************************************************/
/*
#include "pin_map.h"

#include "spi_manager.h"
#include "usb_manager.h"

#include "pen_spi.h"
#include "pen_usb.h"

#include "pen_zoom.h"  
#include "pen_motion.h"
#include "pen_button.h"
#include "led_blinker.h"
#include "trajectory.h"
*/
/**************************************************************************************************************************************/

/************************************************************* Type Definitions *******************************************************/
/**************************************************************************************************************************************/
/*
struct AppStruct // srtuttura complessa che contiene i puntatori alle altre strutture del sistema che rappresentano diverse funzionalità hardware e software del dispositivo
{

  SPIManager* spiManager;
  USBManager* usbManager;

  SPIStruct* penSpi;
  USBStruct* penUsb;

  PenMotionStruct* penMotion;
  ZoomStruct* zoom;
  trajectory_calm* traj_record;

  ButtonStruct* rightButton;
  ButtonStruct* leftButton;
  ButtonStruct* middleButton;

  LedBlinkerStruct* ledOk;
  LedBlinkerStruct* ledFault;
  LedBlinkerStruct* ledOnOff;
  LedBlinkerStruct* ledRed;
  LedBlinkerStruct* ledGreen;


}; //  Questa struttura complessa consente di accedere facilmente e gestire tutte le componenti dell'applicazione attraverso un'unica struttura.
*/

/*
typedef struct AppStruct // srtuttura complessa che contiene i puntatori alle altre strutture del sistema che rappresentano diverse funzionalità hardware e software del dispositivo
{

  SPIManager* spiManager;
  USBManager* usbManager;

  SPIStruct* penSpi;
  USBStruct* penUsb;

  PenMotionStruct* penMotion;
  ZoomStruct* zoom;
  trajectory_calm* traj_record;

  ButtonStruct* rightButton;
  ButtonStruct* leftButton;
  ButtonStruct* middleButton;

  LedBlinkerStruct* ledOk;
  LedBlinkerStruct* ledFault;
  LedBlinkerStruct* ledOnOff;
  LedBlinkerStruct* ledRed;
  LedBlinkerStruct* ledGreen;


} AppStruct; //  Questa struttura complessa consente di accedere facilmente e gestire tutte le componenti dell'applicazione attraverso un'unica struttura.
*/

/**************************************************************************************************************************************/

/************************************************************* Function Declarations **************************************************/
/**************************************************************************************************************************************/
/*void initAppStruct(AppStruct* app, SPIManager* app_spiManager, USBManager* app_usbManager, SPIStruct* app_penSpi, USBStruct* app_penUsb, 
PenMotionStruct* app_penMotion, ZoomStruct* app_zoom, ButtonStruct* app_rightButton, ButtonStruct* app_leftButton, ButtonStruct* app_middleButton, 
LedBlinkerStruct* app_ledOk, LedBlinkerStruct* app_ledFault, LedBlinkerStruct* app_ledOnOff, LedBlinkerStruct* app_ledRed, LedBlinkerStruct* app_ledGreen);
*/

/*
AppStruct* initAppStruct(SPIManager* app_spiManager, USBManager* app_usbManager, SPIStruct* app_penSpi, USBStruct* app_penUsb, 
PenMotionStruct* app_penMotion, ZoomStruct* app_zoom, ButtonStruct* app_rightButton, ButtonStruct* app_leftButton, ButtonStruct* app_middleButton, 
LedBlinkerStruct* app_ledOk, LedBlinkerStruct* app_ledFault, LedBlinkerStruct* app_ledOnOff, LedBlinkerStruct* app_ledRed, LedBlinkerStruct* app_ledGreen);
*/
//void processZoomState(AppStruct* app); // work in progress
                
/**************************************************************************************************************************************/

#endif /* APPLICATION_H */