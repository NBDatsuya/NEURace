# include "game_client.h"
PLAYER *player;

PSTAT pstat[MAX_PLAYER];
PSTAT *me;

int main(int argc, char** argv){
    initscr();
    noecho();

    load_plr();
    init_plist();

    init_home(create_menu_box());

    endwin();
}

MENU create_menu_box(){
    int scr_y,scr_x;

    getmaxyx(stdscr,scr_y,scr_x);
    
    return newwin(6,scr_x-12,scr_y-8,5);
}

WINDOW* create_room(){
    int scr_y,scr_x;

    getmaxyx(stdscr,scr_y,scr_x);
    
    return newwin(0,0,0,0);
}

void init_home(MENU menu){
    clear();
    if(menu == NULL) menu = create_menu_box();

    welcome();

    box(menu,0,0);
    refresh();
    wrefresh(menu);
    keypad(menu, true);
    mvwprintw(menu, 0, 2, "Menu");
    
    menu_select(menu);
}

void welcome(){
    addstr("The Frontroom!\nV.0.0.0\n");
    addstr("Be the last one leaving the room\n\n");
    
    if(!strcmp(player->name, "default"))
        addstr("You havn't setup your player profile!\n");
    else
        printw("Hello, %s", player->name);
}

void menu_select(MENU menu){
    
    int highlight = 0;

    init_menu_list();
    refresh();
    while(1){
        int choice;
        /* Menu painting */
        int i;
        for(i = 0;i<MENU_SIZE;i++){
            if(i == highlight) 
                wattron(menu, A_REVERSE);

            mvwprintw(menu, i+1 , 1, MENU_LIST[i]);
            wattroff(menu,A_REVERSE);
        }

        /* Input choice */
        choice = wgetch(menu);

        /* Handle choices */
        switch(choice){
            case KEY_UP:
                highlight--;
                if(highlight<0) highlight=0;
                break;
            case KEY_DOWN:
                highlight++;
                if(highlight>MENU_SIZE-1) highlight=MENU_SIZE-1;
            default:
                break;
        }
        if(choice == 10)
            break;
    }

    switch(highlight){
        case 0:
            check_user_profile();

            gaming(menu);
            break;
        case 1:
            setting(menu);
            init_home(menu);
            break;
        case 2:
            about(menu);
            init_home(menu);
            break;
        case 3:
            exit_game(0);
            break;
    }
}

void gaming(MENU menu){
    int svrfd;
    if((!svr_connect(&svrfd)) 
        // ||(!wait_to_join(&svrfd)) 
        || (!draw_scene(&svrfd)) 
        // || (!countdown_beforestart(&svrfd)) 
        || (!playing(&svrfd)) 
        //|| (!result(&svrfd)) 
        )

        init_home(menu);

    init_home(menu);
}



int svr_connect(int *svrfd){
    int svr_fd;
    int n;

    struct sockaddr_in addr;

    svr_fd = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family=AF_INET;
    addr.sin_port = htons(atoi(player->server_port));
    addr.sin_addr.s_addr=inet_addr(player->server_ip);

    clear();
    refresh();

    if(connect(svr_fd, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in))==-1){
        addstr("Cannot connect to server\nMaybe the server is closed.\n");
        getch();
        return 0;
    }

    (*svrfd) = svr_fd;

    MSG msg;
    strcpy(msg.cmd, "EN");
    strcpy(msg.content, player->name);
    strcat(msg.content, " %c");
    sprintf(msg.content,msg.content,player->sign);

    write(*svrfd, &msg, sizeof(MSG));

    read(*svrfd, &msg, sizeof(MSG));

    if(!strcmp(msg.cmd, "F")){
        addstr("The room is full.\n");
        getch();
        close(*svrfd);
        return 0;
    }else if(!strcmp(msg.cmd,"RE")){
        addstr("You've entered the same server.\n");
        getch();
        close(*svrfd);
        return 0;
    }else if(!strcmp(msg.cmd,"C")){
        return 1;
    }
}

int load_players(int *svrfd){
    MSG msg;
    int i;

    while(1){
        while(read(*svrfd, &msg, MSG_LEN)<0);

        if(!strcmp(msg.cmd, "P")){
            
            PSTAT *p = NULL;
            for(i=0;i<MAX_PLAYER;i++){
                if(pstat[i].is_alive=-1){
                    p = &(pstat[i]);
                    break;
                }
            }

            if(p==NULL){
                addstr("The room is full.\n");
                getch();
                close(*svrfd);
                return 0;
            }

            sscanf(msg.content, "%s%s", p->name, p->sign);

            if(!strcmp(p->name,player->name))
                me = p;

        }else if(!strcmp(msg.cmd, "PE"))
            break;
    }
}
/*
int wait_to_join(int *svrfd){
    int n;
    int count_plr = 0;
    int st_plst = 0;
    char buf[64], cmd[4];

    while(1){
        strcpy(buf, "");
        if((n=read(*svrfd, buf, sizeof(buf)))<0){
            clear();
            addstr("Wait...\n");
            refresh();
            continue;
        }
        
        printw("buf: %s[%d]\n", buf, sizeof(buf));
        sscanf(buf, "%s", cmd);
        printw("cmd: %s\n", cmd);
        refresh();
        

        if(!strcmp(cmd, "P")){
            char pname[17];
            char psign;
            if(!st_plst) {
                // Begin of player list
                count_plr = 0;
                st_plst = 1;
                addstr("Player List\n");
                addstr("----------------------\n");
            }
            count_plr++;
            sscanf("%s%s%s",pname,pname,psign);
            printw("%d. %s[%s]\n", count_plr, pname,psign);
            refresh();
        }else if(!strcmp(cmd, "PE")){
            st_plst = 0;
            printw("Count: %d\n", count_plr);
            refresh();
        }else{
            continue;
        }
    }
}
*/

int draw_scene(int *svrfd){
    WINDOW *room = create_room();
    wrefresh(room);
}
int playing(int *svrfd){
    /* multi-threads 1 for me, other for others */
    pthread_t t_view;
    int input;
    int i;

    pthread_create(&t_view, NULL, view, svrfd);

    while(input = getchar()){
        if (input == 'w')      
            me->x--;
        else if (input == 'a') 
            me->y--;
        else if (input == 's') 
            me->x++;
        else if (input == 'd') 
            me->y++;

        if(me->x<0 || 
            me->y<0 || 
            me->x<LINES-1 || 
            me->y<COLS-1){

            me->is_alive = 0;
            break;
        }

        for(i = 0;i<MAX_PLAYER;i++){
            if(pstat[i].name == player->name)
                continue; 
                
            
            if(me->x==pstat[i].x &&
                me->x==pstat[i].y){

                me->is_alive = 0;
                break;
            }
        }

        move(me->x, me->y);
		addstr(&(me->sign));	
		move(LINES-1, COLS-1);
		refresh();
    }
}

void *view(void *svrfd){
    MSG msg;
    int i;
    char name[17];

    while(1){
        while((read((*(int*)svrfd), &msg, MSG_LEN))<0);

        if(!strcmp(msg.cmd,"M")){
            for(i=0;i<MAX_PLAYER;i++){

                if(pstat[i].name == player->name)
                    continue; 
                
                sscanf(msg.content, "%s %d %d", name, &(pstat[i].x), &(pstat[i].y));

                if(strcmp(pstat[i].name, name)) continue;
                move(pstat[i].x, pstat[i].y);
                addstr(&(pstat[i].sign));
            }

            move(LINES-1, COLS-1);
            refresh();
        }
    }
}

int result(int *svrfd){
    clear();

    move(LINES/2, COLS/3);

    if(!(me->is_alive)){
	    addstr("GAME OVER!\n");
	    
    }else{
        addstr("You win!\n");
    }

    refresh();

    getch();
    
}

void load_plr(){
    int fd, n;
    PLAYER *buf = malloc(sizeof(PLAYER));

    if ((fd = open("user_profile", O_RDONLY, DEFAULT_ROLE)) == -1){
        if((fd = open("user_profile", O_CREAT|O_WRONLY, DEFAULT_ROLE)) == -1){
            printw("Error occured while loading player profile!\n");
            getch();
            exit(1);
        }else{
            strcpy(buf->name, "default");
            buf->sign='0';
            strcpy(buf->server_ip, "0.0.0.1");
            write(fd, buf, sizeof(PLAYER));
            close(fd);
        }
    }else{
        while ((n = read(fd, buf, sizeof(PLAYER))) != 0){
            if (n == -1){
                printw("Runtime error!\n");
                getch();
                exit_game(1);
            }
        }
        close(fd);
    }
    player = buf;
}

void init_plist(){
    int i;
    for(i=0;i<MAX_PLAYER;i++)
        pstat[i].is_alive = -1;
}

void setting(MENU menu){
    int fd;
    while(1){
        int choice = 0;
        clear();
        echo();
        printw("SETTINGS\n");
        printw("Input a number to edit corresponding item\n");
        printw("----------------------------------------\n");
        printw("1 - Your name: %s\n", player->name);
        printw("2 - Your shown-in-game sign: %c\n",player->sign);
        printw("3 - Server IP: %s\n",player->server_ip);
        printw("4 - Port: %s\n", player->server_port);
        printw("0 or other - Back to main\n");
        scanw("%d", &choice);

        switch(choice){
            case 1:
                set_name();
                continue;
                break;
            case 2:
                set_sign();
                continue;
                break;
            case 3: 
                set_ip();
                continue;
                break;
            case 4:
                set_port();
                continue;
                break;
            case 0:
            default:
                break;
        }
        break;
    }
    
    if((fd=open("user_profile",O_WRONLY|O_CREAT,DEFAULT_ROLE))==-1){
        printw("Cannot write file!\n");
        getch();
        exit(1);
    }
    else{
        write(fd, player, sizeof(PLAYER));
        close(fd);
    }

    noecho();
}

void set_name(){
    clear();
    printw("Input your name in no more than 16 chars:\n");
    getstr(player->name);
}

void set_sign(){
    clear();
    printw("Input your sign shown in game in ONLY 1 char:\n");
    scanw("%c", &(player->sign));
}
void set_ip(){
    clear();
    printw("Input server IP address:\n");
    getstr(player->server_ip);
}

void set_port(){
    clear();
    printw("Input server port number:\n");
    getstr(player->server_port);
}

void about(MENU menu){
    clear();
    addstr("The Frontroom\nV.0.0.0\nProgrammed By Huang Xuda Samuel\n");
    addstr("System Programming Course\n");
    addstr("2023.5\n");
    
    getch();
}

void exit_game(int exit_code){
    endwin();
    echo();
    exit(exit_code);
}