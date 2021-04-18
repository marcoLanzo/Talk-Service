#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_COMMAND_PORT 7000

#include "serverSocketOperation.h"
#include "ClienList.h"
#include "string.h"

#define LENGTH_NAME 32
#define BUFLEN 512
#define LENGTH_MSG 101
#define LENGTH_SEND 201
#define N 10


// Global variables
int client_sockfd = 0;
ClientList *head;
char nickname[LENGTH_NAME] = {};
char recv_buffer[LENGTH_MSG] = {};
char send_buffer[LENGTH_SEND] = {};


struct thread_info{
    struct sockaddr_in *server_info;
    struct sockaddr_in *client_info;
    int sockfd;
    pthread_mutex_t mutex;
    char* rcv_message;
    char* send_message;
    pthread_t tid;
    ClientList* list;
};

void catch_ctrl_c_and_exit(int sig) {
    while (head != NULL) {
        printf("\nClose socketfd: %d\n", head->data);
        close(head->data); // close all socket include server_sockfd
        head = delete_node_in_head(head);
    }
    printf("Bye\n");
    exit(EXIT_SUCCESS);
}

void send_to_all_clients(ClientList *np, char tmp_buffer[]) {
    ClientList *tmp = head->next;
    while (tmp != NULL) {
        if (np->data != tmp->data) { // all clients except itself.
            printf("Send to sockfd %d: \"%s\" \n", tmp->data, tmp_buffer);
            send(tmp->data, tmp_buffer, LENGTH_SEND, 0);
        }
        tmp = tmp->next;
    }
}
/*
bool send_all(int socket, void *buffer, size_t length)
{
    char *ptr = (char*) buffer;
    while (length > 0)
    {
        ssize_t  i = send(socket, ptr, length,MSG_NOSIGNAL);
        if (i < 1) return false;
        ptr += i;
        length -= i;
    }
    return true;
}
*/
char ** tokenize_string(char * buffer,char * delimiter){
    char* token ;
    int i = 0;
    char **tokens = malloc(BUFLEN * sizeof(char));
    token = strtok(buffer,delimiter);
    while(token!= NULL) {
        tokens[i] = token;
        token = strtok(NULL,delimiter);
        i++;

    }
    //memset(token,'\0',sizeof(token));
    //printf("%s",token_vector[1]);
    return tokens;
}

void client_handler(void *p_client) {
    struct thread_info* t = (struct thread_info*)p_client;
    int leave_flag = 0;
    t->rcv_message = recv_buffer;
    t->send_message = send_buffer;
    int receive;
    char* buffer_list = malloc(sizeof(char) * 512);
    char** tokens;
    tokens = malloc(sizeof(char*) * BUFLEN);
    if (tokens == NULL){
        perror("Error function malloc");
        exit(EXIT_FAILURE);
    }

    if (buffer_list == NULL){
        perror("Error function malloc");
        exit(EXIT_FAILURE);
    }
    //ClientList *np = (ClientList *)p_client;

        if (msg.type == SET_USERNAME()
        //memset(t->rcv_message,'0',LENGTH_MSG);
        receive = recv(t->list->data, t->rcv_message, LENGTH_MSG, 0);
        //int receive = recv(t->sockfd, t->rcv_message, LENGTH_SEND, 0);
        printf("t->rcv_message: %s\n",t->rcv_message);
        char*string = strdup(t->rcv_message);
        tokens= tokenize_string(string," ");
        //printf("%d\n", receive);
        if (strcmp(tokens[0], "online") == 0) {
            //memset(t->rcv_message, '0', LENGTH_MSG);
            printer_list(head, t->rcv_message);
            printf("%s\n", t->rcv_message);
            send(t->list->data, t->rcv_message, LENGTH_MSG, 0);
        }
        if (strcmp(tokens[0],"connect") == 0){
            //t->list->online = false;
            //puts(tokens[1]);
            int client_socket = scan_list(head,tokens[1]);
            strcpy(t->send_message,"connect");
            strcat(t->send_message,t->name);
            printf("t->send_message: %s\n",t->send_message);
            int inviati = send(client_socket,t->send_message,LENGTH_SEND,0);
            printf("inviati ->%d\n",inviati);
            receive = recv(t->sockfd, t->rcv_message, LENGTH_SEND, 0);
            while (receive == -1){
                receive = recv(t->sockfd, t->rcv_message, LENGTH_SEND, 0);
            }
            if (strcmp(t->rcv_message,"SI") == 0){
                memset(t->send_message,'\0',LENGTH_SEND);
                sprintf(t->send_message,"%s","Connesso con ");
                strcat(t->send_message,tokens[1]);
                send(t->list->data,t->send_message,LENGTH_SEND,0);
                CONVERSATION:
                goto CONVERSATION;

            }
            //CONVERSATION:

        }
        goto STEP1;

        int read_size;
        message msg;

        if((read_size = recv(t->list->data, t->rcv_message, LENGTH_MSG, 0)) == 0)
        {
            printf("User disconnected: %s.\n", clients[sender].username);
            close(clients[sender].socket);
            clients[sender].socket = 0;
            send_disconnect_message(clients, clients[sender].username);

        } else {

            switch(msg.type)
            {
                case GET_USERS:
                    printer_list(head, t->rcv_message);
                    printf("%s\n", t->rcv_message);
                    send(t->list->data, t->rcv_message, LENGTH_MSG, 0);
                    break;

                case SET_USERNAME: ;
                    int i;
                    for(i = 0; i < MAX_CLIENTS; i++)
                    {
                        if(clients[i].socket != 0 && strcmp(clients[i].username, msg.username) == 0)
                        {
                            close(clients[sender].socket);
                            clients[sender].socket = 0;
                            return;
                        }
                    }

                    strcpy(t->list->name, msg.username);
                    printf("User connected: %s\n", t->list->name);
                    send_connect_message(t->list->data, sender);
                    break;

                case PUBLIC_MESSAGE:
                    send_public_message(clients, sender, msg.data);
                    break;

                case PRIVATE_MESSAGE:
                    //TODO SCAN_LIST
                    send_private_message(clients, sender, msg.username, msg.data);
                    break;

                default:
                    fprintf(stderr, "Unknown message type received.\n");
                    break;
            }
        }
    }
    }
     /*
    // Conversation
    while (1) {
        if (leave_flag) {
            break;
        }
        int receive = recv(t->list->data, t->rcv_message, LENGTH_MSG, 0);
        if (receive > 0) {
            if (strlen(recv_buffer) == 0) {
                continue;
            }
            sprintf(t->send_message, "%s：%s from %s", t->list->name, t->rcv_message, t->list->ip);
        } else if (receive == 0 || strcmp(t->rcv_message, "exit") == 0) {
            printf("%s(%s)(%d) leave the chatroom.\n", t->list->name, t->list->ip, t->list->data);
            sprintf(t->send_message, "%s(%s) leave the chatroom.", t->list->name, t->list->ip);
            leave_flag = 1;
        } else {
            printf("Fatal Error: -1\n");
            leave_flag = 1;
        }
        send_to_all_clients(t->list, t->send_message);
    }
    */
    // Remove Node
    close(t->list->data);
    head = delete_nodo(head,t->list->data);
    /*
    if (np == now) { // remove an edge node
        now = np->prev;
        now->link = NULL;
    } else { // remove a middle node
        np->prev->link = np->link;
        np->link->prev = np->prev;
    }
    free(np);
     */
}

int main(int argc,char* argv[])
{
    printf("\n--------------------------------------------------------------------------------");
    printf(  "--------------------------------------------------------------------------------");
    printf(  "--------------------------------------------------------------------------------\n");



    printf("\nProgetto di Sistemi Operativi A.A 2018/2019\nTesinaTalk: Realizzazione di un servizio"
           "talk gestito tramite server\n\nSERVER\n");
  
    puts("Starting server.");

    fd_set file_descriptors;



    //Socket information
    struct sockaddr_in server_info;
    struct sockaddr_in client_info;
    int s_addrlen = sizeof(server_info);
    int c_addrlen = sizeof(client_info);
    int server_sockfd;
    struct thread_info *tlist;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    argv++;

    socketInitialization(&server_sockfd);
    memset((void *)&client_info, 0, c_addrlen);
    structsocketInitialization(&server_info); // inizializzo struct per il server
    connection_socket(server_sockfd,&server_info); // bind e listen

    // Print Server IP
    getsockname(server_sockfd, (struct sockaddr*) &server_info, (socklen_t*) &c_addrlen);
    printf("Start Server on: %s:  %d\n", inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));
    signal(SIGINT, catch_ctrl_c_and_exit);


    // Initial linked list for clients
    head = initialize_node(server_sockfd, inet_ntoa(server_info.sin_addr));
    while (1) {
        client_sockfd = accept(server_sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);

        // Print Client IP
        getpeername(client_sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
        printf("Client %s:  %d come in.\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

        // Append linked list for clients
        ClientList *c = initialize_node(client_sockfd, inet_ntoa(client_info.sin_addr));
        tlist = malloc(sizeof(struct thread_info));
        if (tlist == NULL){
            perror("Error function malloc");
            exit(EXIT_FAILURE);
        }


        tlist->client_info=&client_info;
        tlist->server_info=&server_info;
        tlist->sockfd=server_sockfd;
        tlist->list = c;
        tlist->rcv_message = malloc(BUFLEN*sizeof(char));
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
