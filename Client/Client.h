//color codes
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RESET "\033[0m"


/* Struttura per tenere informazioni del client. Usata in Client.c */
typedef struct connection_info
{
    int socket;
    struct sockaddr_in *address;
    char username[20];
} connection_info;

/* Ogni parametro di questa enum rappresenta un differente case in cui il mio Client potrebbe trovarsi*/
typedef enum {
    CONNECT,
    DISCONNECT,
    GET_USERS,
    SET_USERNAME,
    PUBLIC_MESSAGE,
    PRIVATE_MESSAGE,
    TOO_FULL,
    CONVERSATION,
    CONVERSATION_END,
    NO_CONVERSATION,
    USER_NOT_ONLINE,
    SET_ONLINE,
} message_type;

/* PARAMETRI:
 * type: tipo del messaggio da inviare
 * username: Nome dell'utente che invia il messaggio
 * username_connect: Nome dell'utente connesso ad username
 * data: Contenuto del messaggio da inviare
 */
typedef struct
{
    message_type type;
    char username[21];
    char username_connect[21];
    char data[256];

} message;