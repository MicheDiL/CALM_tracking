/*
* pen_usb.cpp
*
* Created on: August, 2024
*   Author: Michele Di Lucchio
*    
*/

#include "pen_usb.h"

/*
// Initialize USB Controller
USBHost usb; // USBHost is the base class for all calls that rely on USB host communication. When invoked, it initializes a USB controller

// Attach mouse controller to USB
MouseController mouse(usb); // MouseController is the class for all calls to the USBHost relating to an attached mouse
*/

void initUSBStruct(USBStruct* usbStruct, int pin){

  usbStruct->usb_pin = pin;
  usbStruct->USBready = true;

  pinMode(usbStruct->usb_pin, OUTPUT); 
  digitalWrite(usbStruct->usb_pin, LOW); // Enable USB power (Active low)
 
}

void disableUSBStruct(USBStruct* usbStruct, int pin){
  usbStruct->usb_pin = pin;
  usbStruct->USBready = false;

  pinMode(usbStruct->usb_pin, OUTPUT); 
  digitalWrite(usbStruct->usb_pin, HIGH); // Enable USB power (Active low)
}