#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include "../Client/string.h"
#include "clientsocketoperation.h"
#include "Client.h"

#define BUFLEN 512
#define LENGTH_MSG 101
#define LENGTH_SEND 201


/*              ---->VARIABILI GLOBALI<----   */

volatile sig_atomic_t flag = 0;
volatile sig_atomic_t mem = 0;
pthread_mutex_t mutex_1;
pthread_mutex_t mutex_2;

/*SPIEGAZIONE: Struct che utilizzo nel momento in cui instauro una connessione privata */
struct conversation{
    connection_info* connection;
    message msg;
};

void stop_client(connection_info *connection)
{
    close(connection->socket);
    printf("\nArrivederci ");
    printf(KYEL "%s\n" RESET, connection->username);
    exit(0);
}

void handle_user_input(connection_info *connection)
{
    char input[255];
    fscanf(stdin," %[^\n]",input);
    trim_newline(input);

    if(strcmp(input, "/q") == 0 || strcmp(input, "/quit") == 0)
    {
        message msg;
        memset(msg.data,'\0',sizeof(msg.data));
        strcpy(msg.username,connection->username);
        msg.type = DISCONNECT;

        if(send(connection->socket, &msg, sizeof(message), 0) < 0)
        {
            perror("Send failed");
            exit(1);
        }
        stop_client(connection);
        memset(input,0,sizeof(input));
    }
    else if(strcmp(input,"/online") == 0 || strcmp(input, "/o") == 0)
    {
        message msg;
        memset(msg.data,'\0',sizeof(msg.data));
        msg.type = GET_USERS;

        if(send(connection->socket, &msg, sizeof(message), 0) < 0)
        {
            perror("Send failed");
            exit(1);
        }
        memset(input,0,sizeof(input));
    }
    else if(strcmp(input, "/h") == 0 || strcmp(input,"/help") == 0)
    {
        printf("\n");
        printf(KYEL "ELENCO COMANDI" RESET);
        printf("\n----------------------------------------------------------------------------------|\n"
               " 1) /help or /h: Elenco comandi disponibili;                                      |\n"
               " 2) /online or /o : Elenco utenti disponibili nell'applicazione                   |\n"
               " 3) /connect <username> : Richiesta connessione con username                      |\n"
               " 4) /public_message : Messaggio inoltrato a tutti gli utenti                      |\n"
               " 5) /quit or /q: Uscita dal programma                                             |\n"
               "----------------------------------------------------------------------------------|\n\n");

        memset(input,0,sizeof(input));
    }
    else if(strncmp(input,"/connect",8) == 0 )
    {
        message msg;
        msg.type = PRIVATE_MESSAGE;

        char *toUsername;

        toUsername = strtok(input+8, " ");

        if(toUsername == NULL)
        {
            puts(KRED "Il formato per un messaggio privato è: / <username> <message>\n" RESET);
            return;
        }

        if(strlen(toUsername) == 0)
        {
            puts(KRED "RICORDA: Devi inserire un nome per inviare un messaggio privato.\n" RESET);
            return;
        }

        if(strlen(toUsername) > 20)
        {
            puts(KRED "OSSERVA: Il nome deve avere tra 1 e 20 caratteri al massimo.\n" RESET);
            return;
        }

        strncpy(msg.username_connect, toUsername, 20);
        printf("msg.username -> %s\n",msg.username_connect);

        if(send(connection->socket, &msg, sizeof(message), 0) < 0) {
            perror("Send failed");
            exit(1);
        }
        memset(input,0,sizeof(input));

    }
    else if (strncmp(input,"/public_message",15) == 0){
        message msg;
        msg.type = PUBLIC_MESSAGE;
        fscanf(stdin," %[^\n]",msg.data);
        printf("msg.data -> %s\n",msg.data);
        if(send(connection->socket, &msg, sizeof(message), 0) < 0) {
            perror("Send failed");
            exit(1);
        }
        memset(input,0,sizeof(input));


    }
    else if (strcmp(input,"/quit") != 0 || strcmp(input,"/online") != 0 || strcmp(input,"/connect") != 0
                || strcmp(input,"/disconnect") != 0 || strcmp(input,"/public_message") != 0
                || strcmp(input,"/help") == 0)
    {
            printf(KRED "ATTENZIONE: " RESET "Hai inserito un comando non valido. Riprova \n\n");
            memset(input,0,255);
    }
}


void handle_user_send(struct conversation* conv){
    fscanf(stdin," %[^\n]",conv->msg.data);
    conv->msg.type = CONVERSATION;
    if (strcmp(conv->msg.data,"exit")==0 || strcmp(conv->msg.data,"e")==0){
        conv->msg.type = CONVERSATION_END;
        sprintf(conv->msg.data,"Disconnesso\n");
        fflush(stdout);
        printf("Stop Comunicazione con " KRED "%s\n" RESET,conv->msg.username_connect);
        puts("Sei di nuovo online. Premere" KGRN " /help\n\n" RESET);
        sleep(1);
        send(conv->connection->socket, &conv->msg, sizeof(conv->msg), 0);
        fflush(stdout);
        pthread_mutex_unlock(&mutex_1);
        pthread_exit(NULL);
    }


    else if (strcmp(conv->msg.data,"exit") != 0 || strcmp(conv->msg.data,"e") != 0) {
        if (send(conv->connection->socket,&conv->msg,sizeof(conv->msg),0) < 0){
            perror("Error function send");
            exit(EXIT_FAILURE);
        }
    }
}

void handle_send_answer(struct conversation* chat)
{
    ssize_t recv_val = recv(chat->connection->socket, &chat->msg, sizeof(chat->msg), 0);
    if (recv_val < 0){
        perror("Send failed");
        exit(1);
    }
    else if(recv_val == 0)
    {
        chat->msg.type = SET_ONLINE;
        send(chat->connection->socket,&chat->msg,sizeof(chat->msg),0);
        printf(KYEL "%s " RESET, chat->msg.username_connect);
        printf("si è disconnesso. Premere /help per tornare al menù\n\n");
        pthread_mutex_unlock(&mutex_1);
        pthread_exit(NULL);
    }

    else if (recv_val > 0 && strncmp(chat->msg.data,"Disconnesso",11) != 0 ){
        printf(KWHT "From %s:" KCYN " %s\n"RESET, chat->msg.username_connect, chat->msg.data);
    }

    else if (strncmp(chat->msg.data,"Disconnesso",11) == 0){
        printf(KWHT "From %s:" KCYN " %s\n"RESET, chat->msg.username, chat->msg.data);
        printf("La conversazione con "KRED"%s" RESET,chat->msg.username);
        printf(" è terminata. \n");
        printf("Sei di nuovo online. Premere" KGRN " /help\n\n" RESET);
        fflush(stdout);
        pthread_mutex_unlock(&mutex_1);
        pthread_exit(NULL);
    }
}

void* conversation_send(void*c)
{
    struct conversation* conv2 = (struct conversation*) c;
    fd_set file_descriptors;

    LABEL:
    fflush(stdout);
    FD_ZERO(&file_descriptors);
    FD_SET(STDIN_FILENO, &file_descriptors);
    FD_SET(conv2->connection->socket, &file_descriptors);
    fflush(stdin);

    if(select(conv2->connection->socket+1, &file_descriptors, NULL, NULL, NULL) < 0)
    {
        perror("Select failed.");
        exit(1);
    }

    if(FD_ISSET(STDIN_FILENO, &file_descriptors))
    {
        handle_user_send(conv2);
    }

    if(FD_ISSET(conv2->connection->socket, &file_descriptors))
    {
        handle_send_answer(conv2);
    }

    if (flag == 1){
        puts("bye");
        pthread_mutex_unlock(&mutex_1);
        pthread_exit(NULL);
    }

    goto LABEL;

}
void handle_server_message(connection_info *connection)
{
    message msg;
    pthread_t tid;
    struct conversation *conv;
    //Ricevo una risposta dal Server
    ssize_t recv_val = recv(connection->socket, &msg, sizeof(message), 0);
    if(recv_val < 0)
    {
        perror("recv failed");
        exit(1);

    }
    else if(recv_val == 0)
    {
        close(connection->socket);
        puts(KRED "SERVER DISCONNESSO.\n" RESET);
        exit(0);
    }

    switch(msg.type)
    {

        case CONNECT:
            printf(KYEL "%s has connected." RESET "\n", msg.username);
            break;

        case DISCONNECT:
            printf(KYEL "%s has disconnected." RESET "\n" , msg.username);
            break;

        /* GET_USERS: L'utente ha la possibilità di visionare tutti gli utenti online */
        case GET_USERS:
            printf(KYEL "\nLISTA UTENTI ONLINE\n" RESET);
            printf("%s", msg.data);
            memset(msg.data,0,sizeof(msg.data));
            break;

        case SET_USERNAME:
            //TODO: implement: name changes in the future?
            break;

        case PUBLIC_MESSAGE:
            printf(KGRN "%s" RESET ": %s\n", msg.username, msg.data);
            break;
        case USER_NOT_ONLINE:
            printf(KYEL "\nMESSAGGIO DA SERVER:\n" RESET);
            fprintf(stdout,"%s",msg.data);
            printf("Premere /help per tornare al menù principale\n\n");
            break;
        case PRIVATE_MESSAGE:
            printf(KWHT "From %s:" KCYN " %s\n" RESET, msg.username, msg.data);
            char buf[BUFLEN];
        RESTART:
            fscanf(stdin,"%s",buf);
            strcpy(msg.data,buf);
            printf("msg.data -> %s\n",msg.data);
            if (strcmp(msg.data,"SI") == 0) {
                msg.type = CONVERSATION;
                sprintf(msg.data, "%s", "Inizia la conversazione privata. Ora puoi inviare un messaggio\n");
                printf(KYEL "%s\n" RESET, msg.data);
                if (send(connection->socket, &msg, sizeof(message), 0) < 0) {
                    perror("Send failed");
                    exit(1);
                }
            }else if (strcmp(msg.data,"NO") == 0){
                msg.type = NO_CONVERSATION;
                sprintf(msg.data,KYEL"%s"RESET,msg.username);
                strcat(msg.data,KCYN " non vuole comunicare con te. Digita /help per tornare al menù\n\n"RESET);
                if (send(connection->socket, &msg, sizeof(message), 0) < 0) {
                    perror("Send failed");
                    exit(1);
                }
            }

            /* Se inserisco un comando non valido -- NO || SI -- fornisco l'opportunità all'utente di reinserire
             * il comando */
            else if (strcmp(msg.data,"NO") !=0 || strcmp(msg.data,"SI") != 0){
                puts(KRED "ATTENZIONE: " RESET);
                puts("Mi dispiace hai inserito una risposta non corretta. Ricorda immetti "KRED"SI"RESET " o" KRED" NO\n" RESET);
                goto RESTART;
            }

            //printf(KWHT "From %s:" KCYN " %s\n" RESET, msg.username, msg.data);
            break;
        case CONVERSATION:
          printf(KWHT "From %s:" KCYN " %s\n"RESET, msg.username_connect, msg.data);
          if (mem <= 1){
              conv = malloc(sizeof(struct conversation));
              if (conv == NULL){
                  perror("Error function malloc");
                  exit(EXIT_FAILURE);
              }
          }
          conv->connection = connection;
          memcpy(&conv->msg,&msg,sizeof(message));
          pthread_mutex_lock(&mutex_1);
          if (pthread_create(&tid,NULL,conversation_send,(void*)conv) < 0){
              perror("Error function pthread");
              exit(EXIT_FAILURE);
          }
          pthread_mutex_lock(&mutex_1);
          pthread_mutex_unlock(&mutex_1);
          pthread_join(tid,NULL);
          memset(conv,0,sizeof(struct conversation));
          mem++;
          break;
        case CONVERSATION_END:
            if (strncmp(msg.data,"ERROR",6) == 0){
                printf(KYEL "\nMESSAGGIO DA SERVER:\n" RESET);
                sprintf(msg.data,"%s",KRED "\nATTENZIONE: " RESET);
                strcat(msg.data,msg.username);
                strcat(msg.data,"non sei connesso ad alcun utente. Premere /help e torna al menù.\n\n");
                fprintf(stdout,"%s\n\n",msg.data);
                break;
            }
            break;
        case NO_CONVERSATION:
            printf(KWHT "From %s:" KCYN " %s\n"RESET, msg.username_connect, msg.data);
            break;
        case TOO_FULL:
            break;
        case SET_ONLINE:
            fprintf(stdout,"%s\n",msg.data);
            break;
        default:
            fprintf(stderr, KRED "Tipo di messaggio non riconosciuto." RESET "\n");
            break;
    }
}

// get a username from the user.
void get_username(char *username)
{
    printf("Inserisci un username: ");
    fflush(stdout);
    memset(username, 0, 1000);
    fgets(username, 22, stdin);
    trim_newline(username);
    // TODO check name -> riuscito
    if(strlen(username) > 20)
    {
      //clear_stdin_buffer();
      puts("Username must be 20 characters or less.");

    }

}

//send local username to the server.
void set_username(connection_info *connection)
{
  message msg;
  msg.type = SET_USERNAME;
  strncpy(msg.username, connection->username, 20);

  if(send(connection->socket, (void*)&msg, sizeof(msg), 0) < 0)
  {
    perror("Send failed");
    exit(1);
  }
}

void connect_to_server(connection_info* connection, char* ip_address,int port) {
    while (1) {
        get_username(connection->username);

        // CREATE SOCKET
        socketInitialization(&connection->socket);
        // INITIALIZE STRUCT SOCKET
        connection->address = structAddrInitialization(ip_address, port); //inizializza struttura con le info di rete
        // CREATE CONNECTION CLIENT-SERVER
        socketConnection(connection->socket, *connection->address);
        // INVIO USERNAME SERVER
        set_username(connection);
        message msg;
        // VERIFICO SE L'USERNAME È CORRETTO
        ssize_t recv_val = recv(connection->socket, &msg, sizeof(message), 0);
        //printf("%s\n",msg.data);
        if (recv_val < 0) {
            perror("recv failed");
            exit(1);
        } else if (recv_val == 0) {
            close(connection->socket);
            printf("\n");
            printf(KRED "ATTENZIONE: " RESET);
            printf("Il nome %s è già stato assegnato. Inserisci un nome differente\n", connection->username);
            printf("\n");
            continue;
        } else if (recv_val > 0){
            if (msg.type == TOO_FULL) {
                fflush(stderr);
                fprintf(stderr, "%s\n", msg.data);
                fprintf(stderr, "Ci dispiace ");
                fprintf(stderr,KRED "%s" RESET,msg.username);
                fprintf(stderr,". Riprova a conneterti dopo\n\n");
                sleep(1);
                exit(0);
            } else {
                // STAMPO IL SERVER A CUI SONO CONNESSO
                printf("Connesso al Server: IP %s;  PORTA %d\n", inet_ntoa(connection->address->sin_addr),
                       ntohs(connection->address->sin_port));
                printf(KWHT"\n%s\n" RESET,msg.data); // STAMPO MESSAGGIO DI BENVENUTO DAL SERVER
                break;
            }
        }

    }
}

void handle_signal_action(int sig_number)
{
    if (sig_number == SIGINT) {
        printf("SIGINT è stato catturato.\n\n");
        printf(KYEL "Arrivederci\n" RESET);
        exit(EXIT_SUCCESS);

    }
    else if (sig_number == SIGPIPE) {
        printf("SIGPIPE was catched!\n");
        exit(EXIT_SUCCESS);
    }
}

int setup_signals()
{
    struct sigaction sa;

    sa.sa_handler = handle_signal_action;
    sa.sa_flags=0;

    if (sigaction(SIGINT, &sa, 0) != 0) {
        perror("sigaction()");
        return -1;
    }
    if (sigaction(SIGPIPE, &sa, 0) != 0) {
        perror("sigaction()");
        return -1;
    }

    return 0;
}

int main(int argc,char* argv[])
{
    char message[BUFLEN];
    char** token_vector;
    struct thread_info *tlist;
    connection_info connection;
    fd_set file_descriptors;

    if (setup_signals() != 0)
        exit(EXIT_FAILURE);


    //signal(SIGINT, catch_ctrl_c_and_exit);
    if (argc != 3) {
        fprintf(stderr,"Utilizzo: ./client <server IP> <server numport>\n");
        exit(EXIT_FAILURE);
    }
    unsigned int  server_cmd_port;
    char *p = NULL;

    errno = 0;
    server_cmd_port = (unsigned int) strtoul(argv[2], &p, 0);			//prendo numero di porta
    if(errno != 0 || *p != '\0'){
        fprintf(stderr, "Errore in strtoul() %s è numero di porta non valido!\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    printf("\n--------------------------------------------------------------------------------"); //stampe grafiche
    printf(  "--------------------------------------------------------------------------------");
    printf(  "--------------------------------------------------------------------------------\n");


    printf("\nProgetto di Sistemi Operativi A.A 2018/2019\nTesinaTalk: Realizzazione di un servizio"
           " talk gestito tramite server\n");

    printf("\n--------------------------------------------------------------------------------"); //stampe grafiche
    printf(  "--------------------------------------------------------------------------------");
    printf(  "--------------------------------------------------------------------------------\n\n");

    printf(KYEL "CLIENT" RESET);
    printf("\n\n");



    // FUNZIONE CHE SI OCCUPA DELLA CONNESSIONE CON IL SERVER
    connect_to_server(&connection,argv[1],server_cmd_port);
    pthread_mutex_init(&mutex_1,NULL);
    pthread_mutex_init(&mutex_2,NULL);

    while(1)
    {
        FD_ZERO(&file_descriptors);
        FD_SET(STDIN_FILENO, &file_descriptors);
        FD_SET(connection.socket, &file_descriptors);
        fflush(stdin);
        errno = 0;

        if(select(connection.socket+1, &file_descriptors, NULL, NULL, NULL) < 0 && errno != EINTR)
        {
            perror("Select failed.");
            exit(1);
        }

        if(FD_ISSET(STDIN_FILENO, &file_descriptors))
        {
            handle_user_input(&connection);
        }

        if(FD_ISSET(connection.socket, &file_descriptors))
        {
            handle_server_message(&connection);
        }

        if (flag == 1){
            //send(connection.socket,&connection.socket,sizeof(int))
            puts("bye");
            exit(1);
        }
    }

    close(connection.socket);
    return 0;
}
