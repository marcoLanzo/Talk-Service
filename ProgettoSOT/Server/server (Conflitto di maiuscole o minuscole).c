#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>


#include "serverSocketOperation.h"
#include "ClienList.h"
#include "string.h"
#include "socketConnectList.h"
#include "Client.h"

#define BUFLEN 512
#define LENGTH_MSG 101
#define LENGTH_SEND 201
#define MAX_USERS 5

/*              ---->VARIABILI GLOBALI<----   */

int client_sockfd = 0;

ClientList *head; /* lista di tutti i client connessi al server */

Socket_list* head_socket ; /* lista delle socket dei client occupati in una conversazione */

pthread_mutex_t mutex;     /* mutex utile per la sincronizzazione dei thread associati ai vari client */

int client_socket_connect; /* variabile d'appoggio in cui memorizzare la socket del client
                              che ha effettuato la richiesta di connessione */
int client_socket_request; /* variabile d'appoggio in cui memorizzare la socket del client
                              che ha accettato la richiesta di connessione */
int Numero_utenti_connessi;/* Numero utenti connessi al Server */


struct thread_info{
    struct sockaddr_in *server_info;
    char* rcv_message;
    pthread_t tid;
    ClientList* list;
    message msg;
    int j;
};

/* catch_signal(int sig):
 * Qualora venga lanciato un segnale di SIGINT ctrl+C, Il Server lo catturerà e terminerà una ad una la connessione
 * con i vari client chiudendo e elimando dalla lista di gestione le socket di ciascuno di essi. Fatto ciò si
 * disconetterà tramite notifica a schermo. */
void catch_signal(int sig) {
    while (head != NULL) {
        printf("\n OSS:Chiusura socket: %d\n", head->data);
        close(head->data); // close all socket include server_sockfd
        head = delete_node_in_head(head);
    }
    printf("SERVER DISCONESSO.\n");
    printf("SERVZIO CHAT TERMINATO");
    exit(EXIT_SUCCESS);
}

void send_to_all_clients(ClientList *np, message msg) {
    ClientList *tmp = head->next;
    while ( tmp!= NULL) {

        /* OSS: Invio il messaggio pubblico a tutti gli utenti:
           1) Online 2) Non a me stesso che l'ho inviato */
        if (np->data != tmp->data && tmp->online == 1) {
            printf("Invio alla socket %d: \"%s\" \n", tmp->data, msg.data);
            send(tmp->data,&msg, LENGTH_SEND, 0);
        }
        tmp = tmp->next;
    }
}

void check_user_online(struct thread_info *t){
    printer_list(head, t->rcv_message);
    int len = (int)strlen(t->rcv_message);

    /* Se nella chat non ci sono utenti disponibili, il server mi avverte con un WARNING */
    if ((len - 1) == 0) {
        sprintf(t->rcv_message, "%s", KRED "\nATTENZIONE: "RESET);
        strcat(t->rcv_message, "Non ci sono utenti disponibili\n");
        strcpy(t->msg.data, t->rcv_message);
        free(t->rcv_message);
        send(t->list->data, &t->msg, LENGTH_MSG, 0);

     /* Il server manda al client richiedente una lista di utenti online */
    } else if (len != 0) {
        printf("t->rcv_message %s", t->rcv_message);
        strcpy(t->msg.data, t->rcv_message);
        printf("online: %s\n", t->msg.data);
        fflush(stdout);
        free(t->rcv_message);
        send(t->list->data, &t->msg, LENGTH_MSG, 0);
    }
}

void set_username(struct thread_info* t,char*buffer_list)
{

    /* Se La chat è momentaneamente al completo, ossia il numero di utenti connessi == MAX_USERS -> Allora
     * avvertirò l'utente i-esimo che sta tentando di entrare in chat invitandolo a riprovare più in là. */
    if (Numero_utenti_connessi > MAX_USERS) {
        t->msg.type = TOO_FULL;
        printf("Sono il thread con tid %ld\n", t->tid);
        sprintf(t->msg.data,"%s","Il server non può gestire ulteriori richieste. La chat è al completo");
        int inviati = (int)send(t->list->data, &t->msg, LENGTH_SEND, 0);
        printf(KRED "ATTENZIONE: " RESET);
        fprintf(stdout,"%s\n\n",t->msg.data);
        close(t->list->data);
        Numero_utenti_connessi--;
        pthread_exit(NULL);
        return;
    }

    strcpy(t->list->name, t->msg.username);

    /* Scelgo di utilizzare il nome come chiave univoca dell'utente. Qualora un utente che voglia entrare
     * nella chat abbia lo stesso nome di un'altro utente al fine di evitare ambiguità lo avverto con un
     * WARNING. L'utente a questo punto sarà libero di scegliere un altro nome fino a quando non ne troverà
     * uno libero per lui.*/
    if (check_name_in_list(head, t->list->name)) {
        printf(KRED "ATTENZIONE: " RESET);
        printf("Il nome %s è stato già inserito \n",t->list->name);
        close(t->list->data);
        Numero_utenti_connessi--;
        //pthread_exit(NULL);
        return;
    }

    /* Se L'i-esimo utente che richiede di entrare in chat ha un nome univoco e non è il numero MAX-USERS il
     * server lo accoglierà con un messaggio di benvenuto.*/
    if (Numero_utenti_connessi <= MAX_USERS && check_name_in_list(head,t->list->name) == false) {
        head = insert_in_queue(head, t->list);
        printf(KRED"%s(%s)(%d) Nuovo utente della chat: \n"RESET, t->list->name, t->list->ip, t->list->data);
        printer_list(head, buffer_list);
        //printf("%s", buffer_list);
        sprintf(t->msg.data, "%s", "Benvenuto nella chat ");
        strcat(t->msg.data, t->list->name);
        strcat(t->msg.data, "\nDigita /help per vedere i comandi disponibili\n");
        int inviati = (int)send(t->list->data, &t->msg, LENGTH_SEND, 0);
        if (inviati < 0){
            perror("Error function send");
        }
        return;
    }
}

void private_message(struct thread_info* t)
{
    /* Se provo ad instaurare una connessione con me stesso, il server mi avverte tramite un messaggio di
     * errore. */
    if (strcmp(t->list->name,t->msg.username_connect) == 0){
        t->msg.type = NO_CONVERSATION;
        fprintf(stdout,KRED "%s: "RESET, "ERRORE");
        fprintf(stdout,KYEL "%s " RESET, t->list->name);
        fprintf(stdout,"%s","vuole connettersi con ");
        fprintf(stdout,KYEL "%s.\n" RESET,t->msg.username_connect);
        sprintf(t->msg.data,"%s\n","ERRORE: Non puoi connetterti con te stesso");
        send(t->list->data,&t->msg, sizeof(t->msg), 0);
        return;
    }

    /*Se L'utente con il quale si vuole instaurare una connessione non è online oppure non è presente all'
     * interno della chat, il server mi avverte e stampa un messaggio di Warning */
    if (check_name_in_list(head,t->msg.username_connect) == false){
        t->msg.type = USER_NOT_ONLINE;
        sprintf(t->msg.data,"%s",KRED "ATTENZIONE: " RESET);
        strcat(t->msg.data,"L'utente con cui si vuole instaurare una connessione non è al momento online.\n");
        fprintf(stderr,"%s\n",t->msg.data);
        send(t->list->data,&t->msg, sizeof(t->msg), 0);
        return;
    }

    printf("t->list->data %d\n",t->list->data);
    client_socket_request = t->list->data;
    client_socket_connect = scan_list(head, t->msg.username_connect);

    /* L'utente avrà la possibità di instaurare una connessione con un altro utente solo se entrambi
     * sono online. Effettuato il check l'altro utente avrà l'opportunita di accettare o rifiutare l'invito
     * a conversare. */
    if (check_online(head,t->list->data) && (check_online(head,client_socket_connect))){
        sprintf(t->msg.data,"%s",t->list->name);
        strcat(t->msg.data," vuole comunicare con te\nAccetti la conversazione?\n");
        strcat(t->msg.data,"Digitare ");
        strcat(t->msg.data,KRED"SI"RESET);
        strcat(t->msg.data," o");
        strcat(t->msg.data,KRED" NO\n"RESET);
        strcpy(t->msg.username,t->list->name);
        send(client_socket_connect, &t->msg, sizeof(t->msg), 0);
        puts(t->msg.data);
    }else {

        /* Se un utente generico tenta di effettuare una connessione con un'altro utente mentre è gia conesso
         * con un altro utente, il server lo avvisa --> INUTILE. */
        sprintf(t->msg.data,"%s",KRED"ATTENZIONE: "RESET);
        strcat(t->msg.data,t->list->name);
        strcat(t->msg.data," sei già connesso con ");
        strcat(t->msg.data,t->msg.username_connect);
        send(t->list->data, &t->msg, sizeof(t->msg), 0);
    }
    return;
}

/*Disconnette il client dall'applicazione */
void disconnect_client(struct thread_info* t)
{
    printf(KRED "      ----> ATTENZIONE <----     \n"RESET);
    printf("UTENTE DISCONNESSO:" KYEL"%s.\n" RESET,t->list->name);
    int elimination_socket = scan_list(head,t->list->name);
    head = delete_nodo(head,elimination_socket);
    printer_all_list(head);
    Numero_utenti_connessi--;
    printf(KYEL"Numero utenti connessi %d\n"RESET, Numero_utenti_connessi);
    pthread_exit(NULL);
}

void conversation(struct thread_info*t,Socket_list*sock_req,Socket_list*sock_conn,Socket_list*sock_x)
{
    client_offline(head,t->list->data);
    client_offline(head,obtain_socket(head,t->msg.username));
    if (check_socket_in_list(head_socket,client_socket_request) && strcmp(t->msg.data,"bye") != 0){
        sock_req = initialize_node_socket(client_socket_request);
        head_socket = insert_in_queue_socket(head_socket,sock_req);
    }

    if (check_socket_in_list(head_socket,client_socket_connect) && strcmp(t->msg.data,"bye") != 0){
        sock_conn = initialize_node_socket(client_socket_connect);
        head_socket = insert_in_queue_socket(head_socket,sock_conn);
    }
    printf("t->list.name sender %s\n",t->list->name);
    printf("msg.username %s\n",t->msg.username);
    printf("client_socket_request -> %d\n",client_socket_request);
    printf("client_socket_connect -> %d\n",client_socket_connect);
    printf("t->list->name %s\n",t->list->name);
    puts(t->msg.data);
    printf("msg->username -> %s\n",t->msg.username_connect);
    printf("t->list->data %d\n",t->list->data);
    printer_list_socket(head_socket);
    // TODO PUT MUTEX FOR CONCURRENCY
    if (scan_list_socket(head_socket,sock_req,t->list->data)) {
        t->msg.type = CONVERSATION;
        send(sock_conn->value, &t->msg, sizeof(t->msg), 0);
        //t->socket_request = t->socket_connect;
        return;
    }
    if (scan_list_socket(head_socket,sock_conn,t->list->data)){
        t->msg.type = CONVERSATION;
        send(sock_req->value,&t->msg,sizeof(t->msg),0);
        return;
    }
    pthread_mutex_lock(&mutex);
    t->j = obtain_index_socket_list(head_socket,t->list->data);
    if (t->j % 2 != 0) {
        if (scan_list_socket(head_socket, sock_req, t->list->data) == false) {
            if (strcmp(t->msg.data, "bye") == 0) {
                t->msg.type = CONVERSATION;
                printf("msg.data %s\n", t->msg.data);
                come_back_online(head, t->list->data);
                sock_x = obtain_node_socket(head_socket, t->list->data);
                send(sock_x->next->value, &t->msg, sizeof(t->msg), 0);
                head_socket = delete_nodo_socket(head_socket, sock_x->next->value);
                printer_list_socket(head_socket);
                //accesso = 0;
                pthread_mutex_unlock(&mutex);
                return;
            } else {
                t->msg.type = CONVERSATION;
                strcpy(t->msg.username_connect,t->msg.username);
                sock_x = obtain_node_socket(head_socket, t->list->data);
                send(sock_x->next->value, &t->msg, sizeof(t->msg), 0);
                //accesso = 0;
                pthread_mutex_unlock(&mutex);
                return;
            }
        }
    } else if (t->j % 2 == 0){
        if (scan_list_socket(head_socket,sock_conn,t->list->data) == false)
        {
            if (strcmp(t->msg.data, "bye") == 0) {
                t->msg.type = CONVERSATION;
                printf("msg.data %s\n",t->msg.data);
                come_back_online(head,t->list->data);
                sock_x = obtain_node_socket(head_socket, t->list->data);
                //come_back_online(head_socket,sock_x->)
                send(sock_x->value, &t->msg, sizeof(t->msg), 0);
                head_socket=delete_nodo_socket(head_socket,sock_x->value);
                printer_list_socket(head_socket);
                //accesso = 1;
                pthread_mutex_unlock(&mutex);
                return;
            }else{
                t->msg.type = CONVERSATION;
                strcpy(t->msg.username_connect,t->list->name);
                sock_x= obtain_node_socket(head_socket,t->list->data);
                send(sock_x->value, &t->msg, sizeof(t->msg), 0);
                //accesso = 1;
                pthread_mutex_unlock(&mutex);
                return;
            }
        }
    }
    return;
}

void client_handler(void *p_client) {
    struct thread_info* t = (struct thread_info*)p_client;
    char* buffer_list = malloc(sizeof(char) * 512);
    char tmp_buffer[512];
    if (buffer_list == NULL){
        perror("Error function malloc");
        exit(EXIT_FAILURE);
    }
    int read_size;
    Socket_list* sock_conn ;
    Socket_list *sock_req ;
    Socket_list* sock_x;
    printf("Sono il thread con tid %ld\n",t->tid);

    LABEL:
    sock_conn = malloc(sizeof(Socket_list));
    sock_req = malloc(sizeof(Socket_list));
    sock_x = malloc(sizeof(Socket_list));
    memset(t->msg.data,'\0',sizeof(t->msg.data));
    memset(t->rcv_message,'\0',sizeof(t->rcv_message));
    memset(&t->msg.type,0,sizeof(t->msg.type));
    if((read_size = (int)recv(t->list->data, &t->msg, sizeof(message), 0)) == 0) {

        /* Se il server non riceve alcun messaggio oppure un messaggio vuoto dal client i-esimo si predisporrà a
         * disconnettere l'utente dalla chat e ad eliminarlo dalla lista degli utenti disponibili.*/
        disconnect_client(t);

    } else if (read_size > 0) {

        switch(t->msg.type) {

            case GET_USERS:
                t->rcv_message = malloc(sizeof(char) * BUFLEN);
                check_user_online(t);
                break;

            /* SET_USERNAME: Memorizzo e controllo il nome dell'utente entrato in chat*/
            case SET_USERNAME:
                set_username(t,buffer_list);
                break;

            /*SET_ONLINE: Rimposta ad online un utente alla fine di una conversazione con un altro utente. In questo
             * modo ritornerà disponibile per una conversazione per un altro utente */
            case SET_ONLINE:
                t->msg.type = SET_ONLINE;
                come_back_online(head,t->list->data);
                sprintf(t->msg.data,KRED "%s\n",t->list->name);
                strcat(t->msg.data," sei di nuovo online.\n");
                send(t->list->data,&t->msg,sizeof(t->msg),0);

            /* PUBLIC_MESSAGE: Se un utente decide di inviare un messaggio pubblico, tutti gli utenti online in quel
             * momento saranno in grado di visionare a schermo il messaggio inviato dall'utente.*/
            case PUBLIC_MESSAGE:
                strcpy(t->msg.username,t->list->name);
                t->msg.type = PUBLIC_MESSAGE;
                send_to_all_clients(t->list,t->msg);
                break;

            /* PRIVATE_MESSAGE: Un utente generico in questo caso invierà un messaggio privato per iniziare una
             * conversazione con un altro utente. Il destinatario potrà decidere se accettare o meno la conversazione.*/
            case PRIVATE_MESSAGE:
                printf("MITTENTE:" KYEL "%s\n" RESET,t->list->name);
                printf("DESTINATARIO" KYEL "%s\n" RESET,t->msg.username_connect);
                private_message(t);
                break;

            case CONVERSATION:
                conversation(t,sock_req,sock_conn,sock_x);
                break;

                /* Se il server non riceve alcun messaggio oppure un messaggio vuoto dal client i-esimo si predisporrà a
            * disconnettere l'utente dalla chat e ad eliminarlo dalla lista degli utenti disponibili.*/
            case DISCONNECT:
                disconnect_client(t);
                break;
            case CONVERSATION_END:
                    t->msg.type=CONVERSATION_END;
                    if (check_online(head,t->list->data)){
                        sprintf(t->msg.data,"%s","ERROR");
                        fprintf(stdout,"%s",t->msg.data);
                        fprintf(stdout,"%s ",t->list->name);
                        fprintf(stdout,"%s\n","non è connesso ad alcun utente.");
                        send(t->list->data,&t->msg, sizeof(t->msg), 0);
                        sleep(1);
                        break;
                    }
                    int h = obtain_index_socket_list(head_socket,t->list->data);
                    printf("msg.data %s\n",t->msg.data);
                    come_back_online(head,t->list->data);
                    sock_x= obtain_node_socket(head_socket,t->list->data);
                if (t->j % 2 != 0) {
                    send(sock_x->next->value, &t->msg, sizeof(t->msg), 0);
                    printer_list_socket(head_socket);
                    come_back_online(head, sock_x->next->value);
                    head_socket = delete_nodo_socket(head_socket, t->list->data);
                    printer_list_socket(head_socket);
                    if (head_socket->next == NULL) {
                        head_socket = NULL;
                        printer_list_socket(head_socket);
                    }
                    else if (head_socket->next != NULL) {
                        head_socket = delete_nodo_socket(head_socket, sock_x->next->value);
                        printer_list_socket(head_socket);
                    }
                }else if (t->j % 2 == 0){
                    send(sock_x->value, &t->msg, sizeof(t->msg), 0);
                    printer_list_socket(head_socket);
                    come_back_online(head, sock_x->value);
                    head_socket = delete_nodo_socket(head_socket, t->list->data);
                    printer_list_socket(head_socket);
                    if (head_socket->next == NULL) {
                        head_socket = NULL;
                        printer_list_socket(head_socket);
                    }
                    else if (head_socket->next != NULL){
                        head_socket = delete_nodo_socket(head_socket,sock_x->value);
                        printer_list_socket(head_socket);
                    }
                }
                break;

             /* NO_CONVERSATION: Se un utente vuole connettersi ad un altro utente ma tuttavia l'latro utente rifiuta di
             * instaurare una conversazione inviando un NO al mittente allora il server prenderà in carico la risposta e
             * lo notificherà al client richidente connessione. */
            case NO_CONVERSATION:
                    t->msg.type = NO_CONVERSATION;
                    printf("msg.data -> %s",t->msg.data);
                    come_back_online(head,t->list->data);
                    send(client_socket_request, &t->msg, sizeof(t->msg), 0);
                    break;
            default:
                fprintf(stderr, "Unknown message type received.\n");
                break;
            case CONNECT:break;
            case TOO_FULL:break;
            case USER_NOT_ONLINE:break;
        }
        pthread_join(t->tid,NULL);
        goto LABEL;

    }
}

int main(int argc,char* argv[])
{
    printf("\n--------------------------------------------------------------------------------");
    printf(  "--------------------------------------------------------------------------------");
    printf(  "--------------------------------------------------------------------------------\n");



    printf("\nProgetto di Sistemi Operativi A.A 2018/2019\nTesinaTalk: Realizzazione di un servizio"
           "talk gestito tramite server\n\nSERVER\n");
  
    puts("Starting server.");



    /*                 --->Informazioni Socket<--- */
    struct sockaddr_in server_info;
    struct sockaddr_in client_info;
    int c_addrlen = sizeof(client_info);
    int server_sockfd;
    struct thread_info *tlist;

    if (argc != 2)
    {
     fprintf(stderr, "Usage: %s <port>\n", argv[0]);
     exit(1);
    }

    argv++;
    //Creo socket e la inizializzo
    socketInitialization(&server_sockfd);
    memset((void *)&client_info, 0, c_addrlen);
    unsigned int  server_cmd_port;
    char *p = NULL;

    errno = 0;
    server_cmd_port = (unsigned int) strtoul(argv[0], &p, 0);			//prendo numero di porta
    if(errno != 0 || *p != '\0'){
        fprintf(stderr, "Errore in strtoul() %s è numero di porta non valido!\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    structsocketInitialization(&server_info,server_cmd_port); // inizializzo struct per il server
    connection_socket(server_sockfd,&server_info); // bind e listen

    // Print Server IP
    getsockname(server_sockfd, (struct sockaddr*) &server_info, (socklen_t*) &c_addrlen);
    printf("Start Server on: %s:  %d\n", inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));

    signal(SIGINT, catch_signal);



    /*                                        ---->SPIEGAZIONE<----
     * Inizializzo la testa della mia lista collegata. In questa lista assegnerò ogni nodo della list ad un client
     * specifico cosi da avere nodi svincolati per ogni client. Successivamente per ogni nodo generato e di conseguenza
     * per ciascun client genererò un thread con annessa struttura privata con all'interno tale nodo.
     * Cosi facendo garantirò l'esclusività delle risorse per ciascun client.*/

    head = initialize_node(server_sockfd, inet_ntoa(server_info.sin_addr));
    pthread_mutex_init(&mutex,NULL);
    while (1) {
        client_sockfd = accept(server_sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
        Numero_utenti_connessi++;

        // Stampo Client IP
        getpeername(client_sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
        printf("Client %s:  %d come in.\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

        // Append linked list for clients
        ClientList *c = initialize_node(client_sockfd, inet_ntoa(client_info.sin_addr));
        tlist = malloc(sizeof(struct thread_info));
        if (tlist == NULL){
            perror("Error function malloc");
            exit(EXIT_FAILURE);
        }

        message msg;
        tlist->server_info=&server_info;
        tlist->list = c;
        tlist->rcv_message = malloc(BUFLEN*sizeof(char));
        tlist->msg = msg;
        if (tlist->rcv_message == NULL){
            perror("Error in function malloc");
            exit(EXIT_FAILURE);
        }

        if (pthread_create(&tlist->tid, NULL, (void *)client_handler, tlist) != 0) {
            perror("Create pthread error!\n");
            exit(EXIT_FAILURE);
        }

    }

    return 0;
}