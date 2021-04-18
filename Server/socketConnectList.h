//color codes
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RESET "\033[0m"

/* Operazioni sulla lista delle socket connesse */
/* TODO FINIRE DI COMMENTARE */

typedef struct SocketConnect {
    int value;
    struct SocketConnect* next;
} Socket_list;

Socket_list* alloc_node_socket(void)
{
    Socket_list* new;
    new = malloc(sizeof(ClientList));
    if (new == NULL){
        perror("Error function malloc");
        exit(EXIT_FAILURE);
    }

    return new;
}

Socket_list *initialize_node_socket(int sockfd) {
    Socket_list *n = alloc_node_socket();
    n->value = sockfd;
    return n;
}

bool check_socket_in_list(Socket_list* head_socket,int sockfd) {

    Socket_list* p;
    for (p=head_socket;p != NULL; p = p->next){
        if (p->value == sockfd){
            printf("socket %d già connessa con qualcuno \n",sockfd);
            return false;
        }
    }
    return true;
}

Socket_list* insert_in_queue_socket(Socket_list* head_socket,Socket_list* c)
{

    if (head_socket == NULL){
        return c;
    }

    Socket_list* p;
    for (p=head_socket;p->next != NULL; p = p->next);

    p->next = c;
    c->next = NULL;

    return head_socket;
}

bool scan_list_socket(Socket_list*head_socket,Socket_list* req,int value)
{
    Socket_list * p;
    for (p = head_socket; p != NULL; p = p->next) { // attento è p!=null
        if (req->value == value) {
            return true;
        }
    }
    return false;

}
int obtain_index_socket_list(Socket_list* head_socket,int value)
{
    Socket_list * p;
    int i = 1;
    for (p = head_socket; p != NULL; p = p->next) { // attento è p!=null
        if (p->value == value) {
            return i;
        }else {
            i++;
        }
    }
}

Socket_list* obtain_node_socket(Socket_list* head_socket,int value)
{
    Socket_list * p;
    Socket_list * prev;
    int i = 1;
    for (p = head_socket; p != NULL; p = p->next) { // attento è p!=null
        if (p->value == value){
            if (i % 2 == 0){
                int h = 1;
                for (prev = head_socket; prev != NULL; prev = prev->next){
                    if (h == i-1){
                        return prev;
                    }
                    h++;
                }
            }else {
                return p;
            }
        }
        i++;
    }
}

void printer_list_socket(Socket_list* head_socket) {
    Socket_list *p;
    //free(buffer_rcv);
    for (p = head_socket; p != NULL; p = p->next) { // attento è p!=null
        fprintf(stdout,KYEL "LISTA SOCKET UTENTI\n" RESET);
        fprintf(stdout, "%d->", p->value);
        //fprintf(stdout,"%d-> ",p->next->seq_num);
    }
    fprintf(stdout, "\n");
}

Socket_list* delete_nodo_socket(Socket_list * head_socket,int delete_value){

    Socket_list * prev;
    Socket_list* curr;


    for(curr= head_socket,prev = NULL;curr!=NULL;prev = curr,curr=curr->next){
        if(head_socket->value==delete_value){ //se il valore da eliminare è il primo della lista
            head_socket=head_socket->next;
            break;
        }
        if(curr->value == delete_value){ //se il valore da eliminare non è il primo della ista
            prev->next=curr->next;
            break;

        }
    }
    free(curr);
    return head_socket;
}