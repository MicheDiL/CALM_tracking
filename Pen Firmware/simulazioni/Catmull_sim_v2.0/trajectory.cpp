/*
* trajectory.cpp
*
* Created on: September, 2024
*   Author: Michele Di Lucchio
*    
*/

#include "trajectory.h"

extern LedBlinkerStruct ledGreen; // led verde sotto

extern int currentIteration;
extern int i;

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
    traj_record->current_index = 0;

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
boolean record_trajectory(trajectory_calm* traj, int32_t x_to_be_recorded, int32_t y_to_be_recorded, int32_t min_distance)
{
  if (traj->current_index_record < TRAJECTORY_MAX_SIZE) { // controllo per evitare overflow
      
    // Se ci sono già punti registrati, controlla la distanza rispetto all'ultimo nodo
    if (traj->current_index_record > 0) {

      // Calcola la distanza euclidea tra il nuovo punto e l'ultimo nodo registrato
      int32_t dx = x_to_be_recorded - traj->pos_x_double[traj->current_index_record - 1];
      int32_t dy = y_to_be_recorded - traj->pos_y_double[traj->current_index_record - 1];
      int32_t dist = sqrt(pow2(dx) + pow2(dy));

      // Se la distanza è inferiore a min_distance, non registrare il punto
      if (dist < min_distance) {
        return false;
      }
    }

    // Se la distanza è sufficiente, memorizza il nuovo nodo
    traj->pos_x_double[traj->current_index_record] = static_cast<double>(x_to_be_recorded); // Conversione dei valori di x in double
    traj->pos_y_double[traj->current_index_record] = static_cast<double>(y_to_be_recorded); // Conversione dei valori di y in double

    // Calcola la distanza totale (solo se ci sono già punti registrati)
    if (traj->current_index_record > 0) {
      traj->total_distance += sqrt(pow2(x_to_be_recorded - traj->pos_x_double[traj->current_index_record - 1]) + 
                                    pow2(y_to_be_recorded - traj->pos_y_double[traj->current_index_record - 1]));
    }

    // Registra l'indice
    traj->index_values[traj->current_index_record] = traj->current_index_record;

    // Aggiorna gli indici
    traj->current_index_record += 1;
    traj->current_size += 1; // aggiorna la dimensione del vettore

    // Incrementa il contatore delle iterazioni
    Serial.print("iteration:");
    Serial.print(currentIteration);
    Serial.println(", ");

    currentIteration += 1;

  } else {
    setLed(&ledGreen, !ledGreen.isOn); // Indicazione di overflow della traiettoria
  }

  return true;
  
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


// ***********************************************************
// *********** INTERPOLA LA TRAIETTORIA REGISTRATA ***********
// ***********************************************************
boolean read_and_interp_trajectory(trajectory_calm* traj, int32_t* x, int32_t* y, double num_query_points)
{

  double d = (traj->current_size - 1)/num_query_points;

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
    traj->pointX += (d * traj->current_direction);

    //Serial.print("pointX: ");
    //Serial.println(traj->pointX);

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