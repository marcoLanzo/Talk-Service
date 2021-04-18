/*

 Il file clientsocketoperation.h contiene i metodi che permettono di gestire i socket

*/


/*
 nome funzione: socketInitialization

 parametro1: sockfd, tipo: int*
	
 valore restituito: void

 La funzione socketInitialization() attraverso il metodo socket() associa all'intero passato come parametro un riferimento ad un socket.
 In caso di errore della funzione socket() si ha la terminazione del programma e la stampa di un opportuno messaggio di errore.

*/
void socketInitialization(int *sockfd) {

    *sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(*sockfd < 0) {
        perror("Errore in socket()\n");
        exit(EXIT_FAILURE);
    }
}




/*
 nome funzione: socketConnection

 parametro1: sockfd, tipo: int

 parametro2: servaddr,  tipo: struct sockaddr_in
	
 valore restituito: void

 La funzione socketConnection() attraverso la chiamata alla connect() associa il socket riferito dal descrittore sockfd con la coppia
 indirizzo e numero di porta del server contenuta nella struttura servaddr.
 Nel caso di socket UDP questo comporterà la possibilità di rilevare errori che altrimenti sarebbe stato impossibile rilevare (vedi relazione).
 Inoltre si ha la possibilità di evitare di specificare le coordinate di rete del server ad ogni utilizzo delle funzioni sendto() e recvfrom()
 in quanto, i messaggi inviati dal sockfd su cui è stata effettuata la connect(), avranno come destinatario sempre la stessa coppia
 (indirizzo, n°porta) impostati nel momento della connect() e, quelli ricevuti da tale coppia, saranno recapitati al processo mentre quelli
 diversi verranno scartati automaticamente dal SO.
 In caso di errore della funzione connect() si ha la terminazione del programma e la stampa di un opportuno messaggio di errore.

*/       
void socketConnection(int sockfd, struct sockaddr_in servaddr) {

	if(connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) == -1){
		perror("Errore in connect()\n");
		exit(EXIT_FAILURE);
	}
}




/*
 nome funzione: structAddrInitialization

 parametro1: server_address, tipo: char*

 parametro2: server_numport, tipo: int

 valore restituito: struct sockaddr_in*
  
 La funzione structAddrInitialization(), riempe,  dopo averla allocata tramite malloc(), la struttura sockaddr_in servaddr tramite le funzioni
 htons() e inet_pton() che trasformano i dati passati come parametri (server_numport e server_address) in campi contenenti le informazioni di
 rete nel formato corretto.
 In caso di errore della funzione inet_pton() si ha la terminazione del programma e la stampa di un opportuno messaggio di errore altrimenti
 restituisce un puntatore alla struttura contenente le informazioni di rete.
 
*/
struct sockaddr_in* structAddrInitialization(char *server_address, int server_numport) {

	struct sockaddr_in *servaddr;			//alloco memoria per una struttura sockadrr_in
	servaddr = malloc(sizeof(struct sockaddr_in));
	if(!servaddr){
		fprintf(stderr,"Errore in malloc()\n");
		exit(EXIT_FAILURE);
	}

	memset((void *) servaddr, 0, sizeof(servaddr));
	servaddr->sin_family = AF_INET;                       
  	servaddr->sin_port = htons(server_numport);   	//assegno numero di porta   
                                                       
 	if (inet_pton(AF_INET, server_address, &(servaddr->sin_addr)) <= 0) {	//assegno indirizzo ip
	    	fprintf(stderr, "Errore in inet_pton() causato da %s\n", server_address);
	    	exit(EXIT_FAILURE);
	}
	
	return servaddr;
}




/*
 nome funzione: setRcvTimeout

 parametro1: sockfd, tipo: int

 parametro2: timeout, tipo: unsigned int

 valore restituito: void
  
 La funzione setRcvTimeout() imposta sul socket identificato dal file descriptor sockfd il timeout di ricezione (recvfrom()) tramite
 setsockopt() chiamata con i giusti flag. In caso di errore della funzione setsockopt() si ha la terminazione del programma e la stampa di un
 opportuno messaggio di errore.

*/
void setRcvTimeout(int sockfd, unsigned int timeout) {  
	
	struct timeval t; 

	if(timeout >= 1000000){			//per evitare errori di range nell'assegnazione
		t.tv_sec = timeout / 1000000;	
		t.tv_usec = timeout % 1000000;	
	}
	else{
		t.tv_sec = 0;
		t.tv_usec = timeout;
	}
	if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t)) == -1){
		perror("Errore in setsockopt()\n");
		exit(EXIT_FAILURE);
	}

}
