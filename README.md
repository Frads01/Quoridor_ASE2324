# QUORIDOR
Progetto per l'extrapoint del corso di Architetture dei Sistemi di Elaborazione, PoliTo, a.a. 23/24.

### Controlli
**Tasto INT0**: avvio gioco (tenere premuto)\
**Tasto INT1**: cambia modalità (da giocatore a muro e viceversa)\
**Tasto INT2**: ruota muro di 90°\
**Joystick**: sposta giocatore/muro\
**Select**: conferma scelta

### Regole
Ogni giocatore parte dal centro della sua linea perimetrale, e l'obiettivo è arrivare con la propria pedina alla linea perimetrale opposta. Vince chi consegue per primo l'obiettivo.

Al proprio turno il giocatore può scegliere se muovere orizzontalmente o verticalmente la propria pedina, oppure se inserire un muro. Il muro non può essere scavalcato ma deve essere aggirato.

Se due pedine si trovano faccia a faccia, colui che ha il tratto può scavalcare l'avversario e porsi alle sue spalle. Non è possibile "chiudere" un giocatore con l'aiuto dei muri; bisogna sempre lasciargli la possibilità di arrivare alla meta.

> _**UPDATE 31/01/2024**: aggiornato all'extrapoint 2_
### Caratteristiche principali
*	Rappresentazione della griglia di gioco come matrice 13×13. La matrice, che si compone di celle riserva-te per i muri alternate a celle riservate per il giocatore, permette di gestire lo spostamento delle pedine e il posizionamento dei muri in maniera semplice, verificando direttamente la presenza di zone “murate”, dei percorsi disponibili, e di mosse non valide nel caso del posizionamento dei muri (ad esempio, la sovrapposi-zione).
*	Controllo della “trappola”. Durante il posizionamento del muro, per evitare che uno dei 2 giocatori rimanga intrappolato, è stato utilizzato un algoritmo ricorsivo basato sul Depth-First Search per i grafi: vengono con-trollate le celle adiacenti ai giocatori, e il muro verrà piazzato se esiste almeno un percorso che permette al giocatore di vincere.
*	Salvataggio della mossa. Questa viene gestita tramite una funzione ASM che inserisce correttamente le in-formazioni in essa (giocatore, modalità, etc.); queste possono essere ricavati attraverso la funzione getMo-veInfo(enum en_info) che sfrutta operazioni bit-a-bit.
*	Utilizzo di strutture dati enum per indicare la posizione scelta (en_dir), l’informazione della mossa (en_info) e modalità di gioco (en_mod).
### Impiego risorse hardware
*	Dimensione dello stack: 0x1000 (4096), aumentato per permettere la ricorsione.

| Timer  | Descrizione |
| ------------- |:-------------:|
| Timer 0 (1s) | Usato per gestire il timer di gioco di 20 secondi, definito con una variabile intera unsigned. Alla scadenza di T0, la variabile viene decrementata. Viene resettata a fine mossa o quando si arriva a 0. |
| Timer 1 (3s) | Usato per la comparsa a schermo di scritte. |
| Timer 2 (4s) | Usato per la comunicazione tra CAN: in caso CAN1 non sia disponibile, dopo 4s viene con-trollato CAN2. |
| RIT (50ms) | Usato per il joystick e il debouncing dei tasti. |
### Implementazione delle nuove funzionalità
*	Movimenti diagonali. Sono stati inseriti dei casi aggiuntivi nella struttura dati enum en_dir e nella funzio-ne moveElement, che vengono chiamati solo se viene rilevata la pressione di 2 direzioni adiacenti del joy-stick (e.g., su-destra, giù-sinistra).
*	NPC. Per la scelta della modalità e il posizionamento del muro, è stata usata una funzione random con se-me uguale al valore del RIT quando questa viene chiamata (LPC_RIT->RICOUNTER). Per lo spostamento del giocatore, invece, viene cercato il percorso più breve tramite una versione dell’algoritmo di Dijkstra adattato al progetto. 
*	Modalità 2 schede. La comunicazione ha inizio dopo una fase di handshake, che controlla se le schede siano effettivamente collegate tramite l’invio di un messaggio contenente il numero 0xFE; se c’è il collega-mento, verrà inviato nel verso opposto un altro messaggio con il numero 0xFF che conferma la connessione. La prima scheda che seleziona il numero di schede, e quindi il tipo di avversario, sarà il giocatore 1. 

