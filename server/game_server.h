#include "../myheader.h"

#include <arpa/inet.h>

#include <netinet/in.h>
#include <netinet/ip.h>

#include <signal.h>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/types.h>


#define MAX_PLAYERS 5
#define MAX_EVENTS 10

//Prtcl Message

#define MSG_FULL "F"
#define MSG_RE "RE"

#define IGNORE_PIPE() signal(SIGPIPE, SIG_IGN)

typedef struct player_info{
    char name[17];
    char sign;
    char server_ip[17];
}PLAYER;

typedef struct player_fd{
    PLAYER data;
    int fd;
}PLAYER_FD;

typedef struct msg{
    char cmd[4];
    char content[60];
}MSG;

#define MSG_LEN sizeof(MSG)

/* Epoll related  */

/// @brief Initialize the socket
/// @param *socket Socket
/// @param port Port number(string)
/// @return Bind status
int init_socket(int *, char *);

void setnonblocking(int fd);


/* App prtcl handling functions*/

void init_player();

/// @brief Msg handler
/// @param sockfd Socket fd 
/// @param buf Msg
/// @return 0=end
void handler(int);
void handle_enter(int, MSG *);
void handle_exit(int, MSG *);
void handle_move(int, MSG *);
void brc_plist();

/* App prtcl output functions*/

void alrd_msg(MSG *);
void exit_msg(MSG *, const char []);
void move_msg(MSG *, int , int );
void result_msg(MSG *, const char[]);
void pinfo_msg(MSG *, PLAYER_FD*);