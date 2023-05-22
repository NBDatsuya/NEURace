#include "../myheader.h"

#include <curses.h>

#include <arpa/inet.h>

#include <netinet/in.h>
#include <netinet/ip.h>

#include <pthread.h>

#include <signal.h>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/types.h>

#define MENU_SIZE 4
#define DEFAULT_ROLE 0644
#define MAX_PLAYER 5
#define MSG_LEN 64

#define init_menu_list() char MENU_LIST[MENU_SIZE][16]={"START GAME","SETTINGS","ABOUT","EXIT"}
#define check_user_profile() if(!strcmp(player->name, "default")) setting(menu)

typedef WINDOW* MENU;

typedef struct player_info{
    char name[17];
    char sign;
    char server_ip[17];
    char server_port[5];
}PLAYER;

typedef struct msg{
    char cmd[4];
    char content[60];
}MSG;

typedef struct player_stat{
    char name[17];
    char sign;
    int is_alive;
    int x;
    int y;
}PSTAT;

int main();
MENU create_menu_box();
void init_home(MENU);
void welcome();
void menu_select(MENU);
void load_plr();
void init_plist();

void setting(MENU);

void set_name();
void set_sign();
void set_ip();
void set_port();

void about(MENU);
void exit_game(int);

void gaming(MENU);
int svr_connect(int *);

int draw_scene(int *);
int load_players(int *);
int playing(int *);
int result(int *);
void *view(void *s);