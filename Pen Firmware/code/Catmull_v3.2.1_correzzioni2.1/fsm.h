/*
* fsm.h
*
* Created on: August, 2024
*   Author: Michele Di Lucchio
*   Description: macchina a stati finiti del sistema
*/

#ifndef FSM_H
#define FSM_H

/************************************************************** Define macros *********************************************************/
/**************************************************************************************************************************************/

/**************************************************************************************************************************************/

/************************************************************* Include Files **********************************************************/
/**************************************************************************************************************************************/

/**************************************************************************************************************************************/

/************************************************************* Type Definitions *******************************************************/
/**************************************************************************************************************************************/

enum states {

  INITIALIZATION,       // stato 0: fase di inizializzazione del calm
  ZERO_POINT,           // stato 1: Setta la posizione (0,0) nel punto del workspace scelto dall'operatore
  FREE_HAND,            // stato 2: Controllo manuale del manipolatore
  RECORDING,            // stato 3: Registrazione della traiettoria
  DRAW_RECORD,          // stato 4: Riproduzione di una traiettoria registrata
  OVERFLOW_TRAJ,        // stato 5: Saturazione della memoria per lo storage della traiettoria
  ERROR,                // stato 6
  SEND_LAST_POINT,      // stato 7: Dopo la fine della registrazione della traiettoria riprendi dal punto in cui si trova il laser e non dall'ultimo punto registrato
               
};      

/**************************************************************************************************************************************/

/************************************************************* Function Declarations **************************************************/
/**************************************************************************************************************************************/
// Dichiarazione esterna della variabile globale `current_state`
extern enum states current_state; //  dico al compilatore che la variabile esiste da qualche altra parte (in un file .cpp), ma può essere utilizzata in tutti i file che includono questo header.
/**************************************************************************************************************************************/

#endif /* FSM_H */