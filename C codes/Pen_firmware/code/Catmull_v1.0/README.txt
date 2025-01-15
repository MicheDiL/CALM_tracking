NUOVA VERSIONE DEL FIRMWARE PER IL PENNINO CON FILTRAGGIO DELLE VIBRAZIONI E IMPLMENTAZIONE DI UNA INTERPOLAZIONE CARMULL-ROM SULLA TRAIETTORIA REGISTRATA.

Funzione CatmullSpline
La funzione CatmullSpline() implementa l'interpolazione Catmull-Rom tra punti di dati. Questa funzione prende un insieme di valori xValues e yValues, e calcola il valore interpolato in una posizione intermedia specificata da pointX. Qui, xValues rappresenta la variabile indipendente (l'indice  che rappresenta la posizione nel vettore dei nodi registrati), mentre yValues è la variabile dipendente (i valori delle coordinate X e Y del manipolatore lungo la traiettoria).

Spiegazione passo passo:
Check iniziale (trim): La funzione inizia con un controllo per verificare se pointX si trova all'interno dei limiti degli xValues:

Se pointX è minore o uguale al primo valore in xValues, la funzione restituisce semplicemente il primo valore di yValues.
Se pointX è maggiore o uguale all'ultimo valore in xValues, restituisce l'ultimo valore di yValues.
Questo comportamento è attivato quando il parametro trim è true, e serve a evitare di interpolare punti al di fuori del dominio della traiettoria.

Determinazione dell'intervallo: Successivamente, la funzione determina l'intervallo in cui si trova pointX. Viene trovato l'indice i tale che:

xValues[i] <= pointX < xValues[i + 1]
Questo viene fatto con il ciclo while (pointX >= xValues[i + 1]), che continua ad avanzare fino a trovare l'intervallo corretto.
Interpolazione parametrica: Una volta trovato l'intervallo in cui si trova pointX, la funzione calcola il valore del parametro t, che rappresenta la posizione relativa di pointX tra i valori xValues[i] e xValues[i + 1]:

t = (pointX - xValues[i]) / (xValues[i + 1] - xValues[i]);

Questo t sarà compreso tra 0 e 1, dove:

t = 0 corrisponde a xValues[i].
t = 1 corrisponde a xValues[i + 1]

Calcolo dei polinomi di Hermite: La funzione poi usa i polinomi di Hermite per calcolare una curva interpolata:

auto h00 = 2 * t^3 - 3 * t^2 + 1;
auto h10 = t^3 - 2 * t^2 + t;
auto h01 = -2 * t^3 + 3 * t^2;
auto h11 = t^3 - t^2;

Calcolo delle pendenze (m0, m1): La Catmull-Rom Spline richiede anche la derivata o la pendenza ai punti 
𝑃1 e 𝑃2 , che sono calcolate come m0 e m1. 

Spiegazione del codice:
Variabile indipendente: l'array index_values[] contiene gli indici dei nodi della traiettoria. Questi indici sono usati come variabile indipendente nella funzione CatmullSpline(). Essi rappresentano la posizione relativa tra i nodi, e sono utilizzati per trovare i punti intermedi.

Variabile dipendente: Gli array pos_x_double[] e pos_y_double[] sono le variabili dipendenti, cioè i valori effettivi di X e Y che vuoi interpolare.

Interpolazione: La funzione CatmullSpline() prende come input:

index_values[]: l'array degli indici.
pos_x_double[] o pos_y_double[]: gli array dei valori di X o Y.
traj->current_index: l'indice corrente da interpolare.

Il numero minimo di punti necessari per eseguire un'interpolazione Catmull-Rom è 4 punti.

Motivo:
La Catmull-Rom Spline è una spline cubica che utilizza 4 punti per calcolare una curva che passa attraverso i due punti centrali, utilizzando i due punti esterni come "controllo" per la forma della curva. Ecco la disposizione tipica:

P0: il punto precedente (controllo).
𝑃1: il primo punto effettivo (dove inizia la curva).
𝑃2: il secondo punto effettivo (dove finisce la curva).
𝑃3: il punto successivo (controllo).

La curva viene interpolata tra 
𝑃1 e P2, ma per determinare la pendenza e la forma della curva, sono necessari anche 𝑃0 e 𝑃3.

Perché servono 4 punti?
Punto iniziale e punto finale della curva: La curva viene generata tra i punti 𝑃1 e 𝑃2, quindi abbiamo bisogno di almeno due punti tra cui interpolare.
Punti di controllo: La spline Catmull-Rom utilizza i punti 𝑃0 e 𝑃3 per determinare la pendenza (derivata) ai bordi della curva, fornendo un'influenza su come la curva entra e esce dai punti 𝑃1 e 𝑃2.

Caso di curve ai bordi:
Al bordo iniziale o finale della traiettoria (ad esempio quando si sta interpolando all'inizio o alla fine della sequenza di punti), la pendenza può essere stimata con metodi speciali (come la differenza in avanti o all'indietro) per approssimare i punti mancanti 𝑃0 o 𝑃3. Tuttavia, per l'interpolazione interna, sono necessari 4 punti. Il corpo della funzione CatmullSpline fornita è progettato per gestire l'interpolazione anche ai bordi della traiettoria, grazie ad alcuni controlli specifici che calcolano le pendenze in modo diverso quando si è all'inizio o alla fine del set di dati.

Come la funzione gestisce i bordi:
Gestione del parametro trim: All'inizio della funzione, c'è un controllo del parametro trim, che assicura che se il valore di pointX è fuori dai limiti del vettore, la funzione restituisce direttamente il primo o l'ultimo valore, senza tentare un'interpolazione:

if (trim)
{
    if (pointX <= xValues[0]) return yValues[0];
    if (pointX >= xValues[numValues - 1]) return yValues[numValues - 1];
}

Questo significa che, se stai cercando di interpolare un punto prima del primo o dopo l'ultimo nodo della traiettoria, la funzione non tenterà di interpolare oltre questi limiti, ma restituirà direttamente i valori associati al primo o all'ultimo nodo.

Calcolo delle pendenze ai bordi: Se ti trovi nel primo o ultimo intervallo della traiettoria (ossia stai interpolando tra il primo e il secondo nodo o tra il penultimo e l'ultimo nodo), la funzione CatmullSpline calcola le pendenze in modo speciale:
All'inizio della traiettoria (quando 𝑖=0):

if (i == 0)
{
    m0 = (yValues[1] - yValues[0]) / (xValues[1] - xValues[0]);
    m1 = (yValues[2] - yValues[0]) / (xValues[2] - xValues[0]);
}

Qui, la pendenza 𝑚0 è calcolata come la differenza tra il primo e il secondo punto, e 𝑚1 è calcolata usando il terzo punto come riferimento per la pendenza verso l'esterno.
Alla fine della traiettoria (quando 𝑖=numValues−2):

else if (i == numValues - 2)
{
    m0 = (yValues[numValues - 1] - yValues[numValues - 3]) / (xValues[numValues - 1] - xValues[numValues - 3]);
    m1 = (yValues[numValues - 1] - yValues[numValues - 2]) / (xValues[numValues - 1] - xValues[numValues - 2]);
}
Alla fine della traiettoria, la pendenza viene calcolata usando il penultimo e l'ultimo punto. Per 𝑚0, la funzione utilizza il terzultimo punto per calcolare la differenza tra 𝑃3 e 𝑃1, mentre m1 utilizza solo l'ultimo intervallo tra il penultimo e l'ultimo punto.
