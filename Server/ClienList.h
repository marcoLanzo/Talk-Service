

//color codes
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RESET "\033[0m"


#include <stdbool.h>

#define BUFLEN 512

typedef struct ClientNode {
    int data;
    struct ClientNode* next;
    char ip[16];
    char name[31];
    bool online;
} ClientList;

/* valore di ritorno : struct ClientNode *
 * Descrizione: Tale funzione contiene al suo interno il procedimento per gestire l'allocazione di un nuovo nodo.*/
ClientList* alloc_node(void)
{
    ClientList* new;
    new = malloc(sizeof(ClientList));
    if (new == NULL){
        perror("Error function malloc");
        exit(EXIT_FAILURE);
    }

    return new;
}

/* valore di ritorno : struct CLientNode*
 * Descrizione: Tale funzione si occupa di allocare un nodo e di inzializzarne i vari campi nel modo corretto. */
ClientList *initialize_node(int sockfd, char* ip) {
    ClientList *np = alloc_node();
    struct timeval tv;

    tv.tv_sec = 30;  /* 30 Secs Timeout */

    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(struct timeval));
    np->data = sockfd;
    np->next= NULL;
    strncpy(np->ip, ip, 16);
    strncpy(np->name, "NULL", 5);
    np->online = 1;
    return np;
}


/* valore di ritorno : bool
 * Descrizione: Tale funzione verifica se la stringa passata da parametro è presente nella lista collegata. Utile per
 * verifacare se un dato nome utente (usata come id univoco del Client) è già stato usato o meno.*/
bool check_name_in_list(ClientList* head,char name[31]) {

    ClientList* p;
    for (p=head;p != NULL; p = p->next){
        if (strcmp(p->name,name) == 0){
            return true;
        }
    }
    return false;

}

/* valore di ritorno : void
 * Descrizione: Tale funzione si occupa di reimpostare su online il client associato alla socket passata da parametro */
void come_back_online(ClientList* head,int socket){
    ClientList* p;
    for (p=head;p != NULL; p = p->next){
        if (p->data == socket){
            p->online = 1;
        }
    }
}

/* valore di ritorno : int
 * Descrizione: Tale funzione ci permette di ottenere la socket associata al nome utente passato da parametro.*/
int obtain_socket(ClientList* head,char* name){
    ClientList* p;
    for (p=head;p != NULL; p = p->next){
        if (strncmp(p->name,name,21) == 0){
            return p->data;
        }
    }
}

/* valore di ritorno : struct ClientNode *
 * Descrizione: Tale funzione ci permette di inserire in coda della lista un nuovo nodo c */
ClientList* insert_in_queue(ClientList* head,ClientList* c)
{

    if (head == NULL){
        return c;
    }

    ClientList* p;
    for (p=head;p->next != NULL; p = p->next);

    p->next = c;
    c->next = NULL;

    return head;
}

/* valore di ritorno: void
 * Descrizione: Tale funzione stampa il contenuto della lista.
 * Utilizzata per stampare a schermo tutti gli utenti online.*/
void printer_list(ClientList*head,char* buffer_rcv)
{
    ClientList*p;
    int i = 1;
    char intero[4];
    strcat(buffer_rcv,"\n");
    for (p = head; p != NULL; p = p->next){
        if (strcmp(p->name, "NULL") != 0 && p->online == 1){
            sprintf(intero,"%d)",i);
            strcat(buffer_rcv,intero);
            strcat(buffer_rcv,p->name);
            strcat(buffer_rcv,"\n\n");
            printf(KYEL "ONLINE :\n");
            fprintf(stdout, "%s",buffer_rcv);
            i++;
        }
    }
    fprintf(stdout,"\n");

}

/*
 * valore di ritorno : struct ClientNode *
 * Descrizione: Tale funzione elimina l'elemento in testa.*/
ClientList * delete_node_in_head(ClientList * head){

    ClientList * p;
    if ( head !=NULL){
        p= head;
        head = head->next;
        free(p);
    }
    return head;
}

/* valore di ritorno: void
 * Descrizione: Tale funzione stampa il contenuto della lista.
 * Utilizzata per stampare a schermo tutti gli utenti online e non online.*/
void printer_all_list(ClientList*head){
    ClientList * p;
    for (p = head; p != NULL; p = p->next){
        printf("p->name %s\n",p->name);
    }
    fprintf(stdout,"\n");
}

/* valore di ritorno: int
 * Descrizione:  Tale funzione ci permette di ottenere la socket associata al nome utente passato da parametro.*/
int scan_list(ClientList* head,char* name){
    ClientList * p;
    for (p = head; p != NULL; p = p->next){
        if (strcmp(p->name,name) == 0){
            return p->data;
        }
    }

}

/* valore di ritorno: void
 * Descrizione:  Tale funzione ci permette di reimpostare offline l'utente in lista associato alla scoket passata per
 * parametro. */
void client_offline(ClientList* head,int data)
{
    ClientList * p;
    for (p = head; p != NULL; p = p->next){
        if (p->data == data){
            p->online = 0;
        }
    }

}

/* valore di ritorno: bool
 * Descrizione:  Tale funzione ci permette di verificare se l'utente i-esimo identificato tramite socket passata da
 * parametro sia al momento della chiamata di tale funzione online o offline */
bool check_online(ClientList* head,int data)
{
    ClientList * p;
    for (p = head; p != NULL; p = p->next){
        if (p->data == data){
            if (p->online == 1){
                return true;
            }
            else
                return false;
        }
    }
}

/*
 * valore di ritorno : struct ClientNode *
 * Descrizione: Tale funzione elimina il nodo della lista con socket di valore delete_value. Il nodo eliminato può
 * presentarsi in qualsiasi posizione della lista, in qualsiasi caso sia esso sarà eliminato. */
ClientList* delete_nodo(ClientList * head,int delete_value){

    ClientList * prev;
    ClientList * curr;


    for(curr= head,prev = NULL;curr!=NULL;prev = curr,curr=curr->next){
        if(head->data==delete_value){ //se il valore da eliminare è il primo della lista
            head=head->next;
            break;
        }
        if(curr->data == delete_value){ //se il valore da eliminare non è il primo della ista
            prev->next=curr->next;
            break;

        }
    }
    free(curr);
    return head;
}
