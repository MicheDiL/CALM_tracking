/*
* trajectory.cpp
*
* Created on: September, 2024
*   Author: Michele Di Lucchio
*    
*/

#include "trajectory.h"

extern LedBlinkerStruct ledGreen; // led verde sotto


// Definizione del puntatore globale alla struttura trajectory_calm
trajectory_calm* traj_record = NULL;  // Inizializza il puntatore globale a NULL: non punta a nessun blocco di memoria

// ***********************************************************
// *********** INZIALIZZAZIONE DELLA STRUTTURA  **************
// ***********************************************************
trajectory_calm* init_trajectory_struct()
{
  trajectory_calm* traj_record = (trajectory_calm*) malloc(sizeof(trajectory_calm)); // Allocazione di memoria

  if (traj_record != NULL){ // aggiorno il valore del puntatore traj_record con un nuovo indirizzo di memoria che ora punta al blocco allocato.

    // Inizializza i vettori delle posizioni assolute (int32_t)
    //memset(traj_record->pos_x, 0, sizeof(traj_record->pos_x));
    //memset(traj_record->pos_y, 0, sizeof(traj_record->pos_y));

    traj_record->current_index_record = 0;
    traj_record->current_size = 0;
    traj_record->current_direction = 1;

    traj_record->total_distance = 0;

    traj_record->pointX = 0;

    // Inizializza i vettori delle coordinate double (pos_x_double, pos_y_double)
    memset(traj_record->pos_x_double, 0, sizeof(traj_record->pos_x_double));
    memset(traj_record->pos_y_double, 0, sizeof(traj_record->pos_y_double));

    // Inizializza i valori degli indici (index_values)
    memset(traj_record->index_values, 0, sizeof(traj_record->index_values));
    
    return traj_record; // Restituisce il puntatore alla struttura inizializzata

  } else {

    return NULL;
  }
  
}

// ***********************************************************
// *********** REGISTRAZIONE DELLA TRAIETTORIA  **************
// ***********************************************************
/*
Record the trajectory. Stops recording when the structure is full.
Also computes the distance covered, for not displaying the result if this distance is too small.
*/
void record_trajectory(trajectory_calm* traj, int32_t x_to_be_recorded, int32_t y_to_be_recorded)
{
  if (traj->current_index_record < TRAJECTORY_MAX_SIZE){ // controllo curciale per evitare overflow 
  
    //traj->pos_x[traj->current_index_record] = x_to_be_recorded;
    //traj->pos_y[traj->current_index_record] = y_to_be_recorded;

    traj->pos_x_double[traj->current_index_record] = static_cast<double>(x_to_be_recorded); // Conversione dei valori di x in double
    traj->pos_y_double[traj->current_index_record] = static_cast<double>(y_to_be_recorded); // Conversione dei valori di y in double

    if (traj->current_index_record > 0){ // se ci sono già punti registrati
      // sommo la distanza euclidea fra due punti registrati successivi: somma dei quadrati delle differenze tra le coordinate x e y del punto corrente e del punto precedente
     // traj->total_distance += sqrt(pow2(x_to_be_recorded - traj->pos_x[traj->current_index_record - 1]) + pow2(y_to_be_recorded - traj->pos_y[traj->current_index_record - 1]));
    }

    traj->index_values[traj->current_index_record] = traj->current_index_record; // Indici da 0 a length - 1

    traj->current_index_record += 1;
    traj->current_size += 1; // reppresenta la length del vettore
    
  }else{
    setLed(&ledGreen, !ledGreen.isOn);
  }
}

// ***********************************************************
// ********** REINIZIALIZZAZIONE DELLA TRAIETTORIA  **********
// ***********************************************************
void reinit_trajectory(trajectory_calm* traj)
{
  traj->current_index_record = 0;
  traj->current_size = 0;
  traj->current_direction = 1;
  traj->total_distance = 0;
}

void cleanup_trajectory() {

  if (traj_record != NULL) {
    free(traj_record);  // Libera la memoria dinamica allocata per la traiettoria
    traj_record = NULL; // Previene l'uso accidentale di un puntatore non valido
  }

}
// ***********************************************************
// *********** LETTURA DELLA TRAIETTORIA REGISTRATA **********
// ***********************************************************
/*
Reads the tabulars in one direction and switch direction when arrived at one extremity.
When go_back is asserted, it switches in the direction of the most recent point.
*/

/*void read_trajectory(trajectory_calm* traj, int32_t* x, int32_t* y, boolean go_back)
{
  if (traj->current_index >= traj->current_size) // se si è raggiunta la fine della traiettoria
  {
    traj->current_direction = - 1;
    traj->current_index += - 1;
    traj->current_distance = 0;
  }
  else if (traj->current_index == 0 || go_back) // cambia direnzione di scorrimento del vettore in cui è salvata la traiettoria registrata
  {
    traj->current_direction = 1;
    traj->current_distance = 0;
  }
  
  // assegno le coordinate del punto corrente alle variabili puntate da x e y
  *x = traj->pos_x[traj->current_index];
  *y = traj->pos_y[traj->current_index];

  traj->current_index += traj->current_direction; // aggiorno l'indice corrente in base alla direzione attuale
}*/

// ***********************************************************
// *********** INTERPOLA LA TRAIETTORIA REGISTRATA ***********
// ***********************************************************
boolean read_and_interp_trajectory(trajectory_calm* traj, int32_t* x, int32_t* y, double increment)
{

  if (traj->current_size > 4) 
  {
    
    if(traj->pointX >= traj->index_values[traj->current_size - 1]){

      //traj->pointX = traj->index_values[traj->current_size - 1];  // Inizia dall'ultimo nodo
      traj->current_direction = -1; // Direzione inversa
                                    
    } else if (traj->pointX <= 0){

      traj->pointX = 0;                       // Inizia dal primo nodo
      traj->current_direction = 1;            // Direzione normale
                                    
    }

    // Interpolazione Catmull-Rom per la coordinata X
    double interpolated_x = Interpolation::CatmullSpline(traj->index_values, // Variabile indipendente
    traj->pos_x_double,                                                      // Variabile dipendente: i valori di X
    traj->current_size,                                                      // Numero di nodi
    (traj->pointX),                                                          // Parametro pointX
    true);                                                                   // Limita ai bordi

    // Interpolazione Catmull-Rom per la coordinata Y
    double interpolated_y = Interpolation::CatmullSpline(traj->index_values, // Variabile indipendente
    traj->pos_y_double,                                                      // Variabile dipendente: i valori di Y
    traj->current_size,                                                      // Numero di nodi
    (traj->pointX),                                                          // Parametro pointX
    true);                                                                   // Limita ai bordi

    // Passaggio dei valori interpolati al manipolatore, convertiti in int32_t
    *x = static_cast<int32_t>(interpolated_x);
    *y = static_cast<int32_t>(interpolated_y);

    // Incrementa pointX per ottenere il prossimo valore alla chiamata successiva
    traj->pointX += increment * traj->current_direction;

  }
  else
  {
    return false; // Non ci sono abbastanza dati per procedere
  }
  return true;
}

int32_t pow2(int32_t input)
{
  return input * input;
}