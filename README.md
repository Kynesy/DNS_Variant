# Progetto di Moltiplicazione di Matrici

Il progetto è un'implementazione di una versione modificata dell'algoritmo DNS di Dekel, Nassimi and Sahni che richiede $n^3$ processori (un elemento per processore)

La variante implementata associa più di un elemento per processore, in particolare richiedendo  $n^2r$ processori con $1 \le r \le n$. 

## Struttura del Progetto

Il progetto è organizzato nei seguenti file:

- `dnsVariant.c`: contengono l'implementazione dell'algoritmo di moltiplicazione di matrici.
- `generateMatrix.c`: contiene una funzione per generare matrici casuali.
- `inOutUtils.c` e `inOutUtils.h`: contengono funzioni per leggere e scrivere matrici da e su file.
- `seqMatrixMultiply.c`: contiene l'implementazione sequenziale dell'algoritmo di moltiplicazione di matrici.
- `dns.c`: implementazione del DNS a scopo autodidattico.
- `Makefile`: file per la compilazione del progetto.
- `script.sh`: script per eseguire misurazioni di performance e verificare la correttezza dell'algoritmo.

## Compilazione del progetto

Per compilare il progetto, eseguire il comando `./script.sh -b` nella directory principale del progetto, oppure usare direttamente il Makefile con il comando `make all`.


## Script

Lo shell script può essere utilizzato sia per effettuare le misurazioni dei tempi che per verificare se il risultato ottenuto da una moltiplicazione matriciale corrisponde allo stesso ottenuto dalla versione sequenziale. Per ulteriori dettagli controllare la sezione usage dello script con il comando `./script.sh -h`

## Esecuzione manuale dnsVersion

L'algoritmo va eseguito dopo la creazione delle matrici tramite il comando `./generateMatrix [SIZE]`. Le matrici generate sono logicamente quadrate e sono memorizzate nei file *matrixA.bin* e *matrixB.bin* come array unidimensionali.
La generazione delle matrici utilizza il seed impostato nel file `inOutUtils.h`.

Una volta generate le matrici, si può procedere con il calcolo utilizzando il comando `mpirun --oversubscribe -n [PROC] ./dnsVariant [SIZE]`.
E' importante che il parametro `[PROC]` sia $[SIZE]^2 \le [PROC] \le [SIZE]^3$.

Inoltre $[PROC]=n^2r$. Dato che il funzionamento si basa sulla suddivisione in sottomatrici quadrate, è necessario valga la relazione $n\text{ mod }r= 0$

Per esempio per una matrice con $[SIZE]=10$, i processori validi sono $[PROC]=\{10*10*1, 10*10*2, 10*10*5, 10*10*10\}$

Il risultato è memorizzato in *matrixC_dnsVariant.bin* ed è accessibile all'utente tramite il comando `./printMatrix [FILENAME]`.
