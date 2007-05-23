I comandi sono per Linux, senza modifiche non riuscirete a far partire sotto
Windows il BARD.

Per farlo partire compilate andando nella directory /src
make clean
make all

Aprire il file Makefile in /src se volete commentare o decommentare alcune opzioni
per la compilazione

Una volta compilato per farlo partire potete usare il comando
./bard &
oppure
./startup &
oppure ancora
gdb ./bard
se lo volete debuggare

Per modificare la porta di gioco modificate il file sysdata.dat nella cartella
/system, attualmente è la 5002

Per connettervi tramite console di Linux digitate:
telnet 127.0.0.1 5002

Per creravi un Admin createvi prima un pg, poi andate nella cartella /player e
lì troverete il vostro file di personaggio con il nome del pg.
Apritelo testualmente, andate alla voce Trust (non alla voce Level, non dovete
modificare il livello, il livello non centra nulla con i poteri d'amministrazione
del pg) arrivati lì potete modificarla TRUST_PLAYER con una delle seguenti voci:
TRUST_NEOMASTER
TRUST_MASTER
TRUST_NEOBUILDER
TRUST_BUILDER
TRUST_IMPLE
in ordine da quello che ha meno permessi a quello che li ha tutti.
Oppure potete utilizzare il personaggio di nome Imple con password telnet.
Avendo un admin poi potete digitare wizhelp e cominciare a provare i vari comandi.

E' difficile prevedere a priori cosa vi possa servire sapere, purtroppo avendo
confidenza con il codice BARD molte cose potrei darle per scontate, quindi
scrivetemi se avete dubbi, indicarmi correzioni, materiale etc etc..
onirik79 [at] yahoo [dot] it
Con tutto quello che ho dato a quel codice sarò di certo felice che delle persone
se ne interessino :)

Alcune cose da notare del codice:
- Le animazioni ascii di introduzione, anche se molti client non supportano la
    cancellazione dello schermo.
- Il sistema delle banche, ne sono abbastanza soddisfatto, anche se è risultato
    più difficoltoso da gestire di quel che pensassi.
- I bitvector del Bard sono dinamici, possono essere teoricamente infiniti, molto
    potenti peccato che l'integrazione con il vecchio codice dei bit normali e
    gli ext bit abbia causato alcuni bachi che ci sono ancora nel motore.
- Il sistema del calendario, stagioni e anniversari è cosa carina.
- Il fatto che ogni send_log(), la funzione per l'invio dei messaggio di errore,
    supporti un argomento per il passaggio dell'eventuale file letto o scritto,
    così da ricavarne sempre la posizione di lettura o scrittura dell'errore.
    E' una mano santa per la lettura di aree e di file *.dat, ve lo assicuro.
- Il sistema di doppia lingua dei comandi, è un sistema molto complesso ma penso
    non si possa fare di meglio di così senza complicare maggiormente le cose.
- L'invio dei sogni, nulla di che ma sempre qualcosa di carino :)
- Alcune funzioni in gramm.c semplificano la vita, specie per la ricerca
    dell'articolo giusto, imparate ad utilizzarle.
- Il gioco degli scacchi, anche se manca la parte per rilevare se sia scacco
    matto e quindi fine partita.
- Il mapper, quello che si vede di fianco la descrizione della stanza.
- mccp è il protocollo di compressione dei dati inviati, non funziona, non ho
    mai capito perchè..
- memory.c contiene alcune funzioni per la ricerca dei flaw di memoria, non
    funziona :P ma l'idea era buona.. se qualcuno sa come farlo andare.
- Il protocollo msp funziona :) dovete però cambiare l'indirizzo da cui scaricare
    i suoni che sennò punta ancora a quello di terra secunda.
- Il protocollo mxp funziona a metà... non ricordo se per colpa di conflitti di
    tag <> e & (quelli per i colori dello smaug) o per il client zmud.. è un po'
    da rimaneggiare.
- Sistema di inserimento delle news.
- Note diversificate in lettere(rpg), bacheche(rpg per tutti) e note(private offrpg)
- Sistema delle question, da darci una occhiata e rimetterlo a posto se vi interessa
    non ci ho più messo mano da secoli..
- Social razziali, ovvero la possibilità di creare dei social personalizzati per
    ogni razza.
- Cielo stellato visualizzabile con il comando sky.
- Sistema di generazione pagine html tra cui il who, trovate tutto in web.c


Onirik 11/07/2006
