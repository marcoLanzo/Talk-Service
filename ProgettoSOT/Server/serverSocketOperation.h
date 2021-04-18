/*

 Il file servsocketoperation.h contiene i metodi che agiscono sui socket

*/




/*
 nome funzione: socketInitialization

 parametro1: sockfd, tipo: int*

 valore restituito: void

 La funzione socketInitialization() attraverso il metodo socket() associa all'intero passato come parametro un
 riferimento ad un socket. In caso di errore della funzione socket() si ha la terminazione del programma e la stampa
 di un opportuno messaggio di errore.

*/

#define MAX_USERS 5



void socketInitialization(int *sockfd) {

    *sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(*sockfd < 0) {
        perror("Errore in socket()\n");
        exit(EXIT_FAILURE);
    }


}




/*
 nome funzione: cmdsocketInitialization

 parametro1: sockfd, tipo: int*

 parametro2: addr, tipo: struct sockaddr_in*

 valore restituito: void

 Tale funzione ha il compito di riempire la struttura addr tramite la funzione  htons() che trasforma la costante che
 identifica il numero di porta del server in un campo contenente tale informazione nel formato corretto. Inoltre qui,
 a differenza del client, non c'è stato bisogno di usare l'inet_pton che allo stesso modo dell'htons con la porta avrebbe
 trasformato l'indirizzo ip nel giusto formato poichè si è preferito impostare il server in ascolto su qualsiasi ip.

 htons: Converte un numero dal formato del computer locale a quello della rete (Big-Endian)
 */
void structsocketInitialization(struct sockaddr_in *addr,int port) {

    memset((void *)addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    addr->sin_addr.s_addr=INADDR_ANY;


    /*
    if(inet_pton(AF_INET,addr->sin_addr.s_addr,&addr->sin_addr) <= 0) {
        //fprintf(stderr, "Erorre in inet_pton() causato da %s\n", SERVER_ADDR);
        exit(EXIT_FAILURE);
    }
     */
}

/* La bind() associa le informazioni di rete presenti nella struttura addr al socket riferito dal descrittore sockfd in
 * modo che, note tali informazioni, un client possa contattare il server.*/

void connection_socket(int sockfd,struct sockaddr_in *addr) {

    if (bind(sockfd, (struct sockaddr *) addr, sizeof(*addr)) < 0) {
        //perror("Errore in bind()\n");
        exit(EXIT_FAILURE);
    }

    listen(sockfd,MAX_USERS);
}

/*
 nome funzione: datasocketInitialization

 parametro1: sockfd,  tipo: int*

 parametro2: addr,  tipo: struct sockaddr_in

 valore restituito: unsigned int

 La funzione datasocketInitialization() come prima operazione chiama il metodo socket() che ritorna un file descriptor come riferimento al
 socket. Dopo aver riempito la struttura addr tramite le funzioni htons() e inet_pton() che trasformano le costanti che identificano indirizzo
 e numero di porta del server in campi contenenti tali informazioni nel formato corretto, si effettua la chiamata alla bind().
 La bind() associa le informazioni di rete presenti nella struttura addr al socket riferito dal descrittore sockfd in modo che, note tali
 informazioni, un client possa contattare il server. La funzione termina restituendo il numero di porta, scelto in questo caso dal SO,
 convertito opportunamente in un unsigned int tramite ntohs() dopo aver caricato le informazioni associate al socket con la funzione
 getsockname().
 In caso di errore delle funzioni socket(), inet_pton() o bind() si ha la terminazione del programma e la stampa di un opportuno messaggio di
 errore.

*/
/*
unsigned int datasocketInitialization(int* sockfd, struct sockaddr_in addr) {

    if((*sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        //perror("Errore in socket()\n");
        exit(EXIT_FAILURE);
    }

    memset((void *)&addr, 0, sizeof(addr));		//alloco memoria per una struttura sockadrr_in
    addr.sin_family = AF_INET;
    addr.sin_port = htons(0);                  	//scelto in maniera casuale dal SO

    if(inet_pton(AF_INET, SERVER_ADDR, &addr.sin_addr) <= 0) {
        //fprintf(stderr, "Erorre in inet_pton() causato da %s\n", SERVER_ADDR);
        exit(EXIT_FAILURE);
    }

    if(bind(*sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        //perror("Error in bind()\n");
        exit(EXIT_FAILURE);
    }

    socklen_t addr_len = sizeof(&addr);
    getsockname(*sockfd, (struct sockaddr *) &addr, &addr_len);
    return ntohs(addr.sin_port);
}
*/



/*
 nome funzione: setRcvTimeout

 parametro1: sockfd, tipo: int

 parametro2: timeout, tipo: long unsigned

 valore restituito: void

 La funzione setRcvTimeout() imposta sul socket identificato dal file descriptor sockfd il timeout di ricezione (recvfrom()) tramite
 setsockopt() chiamata con i giusti flag. In caso di errore della funzione setsockopt() si ha la terminazione del programma e la stampa di un
 opportuno messaggio di errore.

*/
void setRcvTimeout(int sockfd, long unsigned timeout) {

    struct timeval t;

    if(timeout >= 1000000){			//divido il campo secondi da quello in microsecondi
        t.tv_sec = timeout / 1000000;
        t.tv_usec = timeout % 1000000;
    }
    else{
        t.tv_sec = 0;
        t.tv_usec = timeout;
    }
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t)) == -1){
        //perror("Errore in setsockopt()");
        exit(EXIT_FAILURE);
    }
}

