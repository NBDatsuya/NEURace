#include "game_server.h"

PLAYER_FD players[MAX_PLAYERS];
int is_ongame = 0;

int main(int argc, char** argv){
	char buf[MSG_LEN];	/* Buffer */
	int lfd;
	int i,n;

	/* Epoll related vars*/
	struct epoll_event ev, events[MAX_EVENTS];
    int conn_sock, nfds, epollfd;

	IGNORE_PIPE();

	/*  Bad args */
	if(argc < 2)
		usage("USE: CMD PORT");

	/*  Cannot bind */
    if(init_socket(&lfd, argv[1])==-1)
        errmsg("Cannot bind");

	// Listen for the connection
    listen(lfd, 1);

	init_player();

	// -----------------------------------------
	// Epoll
	// -----------------------------------------
	
	epollfd = epoll_create(10);

    if (epollfd == -1)
        errmsg("epoll_create");

	ev.events = EPOLLIN;
    ev.data.fd = lfd;
    if (epoll_ctl(epollfd, 
				EPOLL_CTL_ADD, 
				lfd, 
				&ev) == -1) 
		errmsg("epoll_ctl: lfd");

	while(1){
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) 
            errmsg("epoll_pwait");

        for (n = 0; n < nfds; ++n) {
            if (events[n].data.fd == lfd) {
                conn_sock = accept(lfd, NULL, NULL);

                if (conn_sock == -1)
                    errmsg("accept");

                setnonblocking(conn_sock);

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_sock;

                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
                            &ev) == -1) 
                    errmsg("epoll_ctl: conn_sock");

        		}else {
					handler(events[n].data.fd);
					close(events[n].data.fd);
            	}
		}
	}	
}

int init_socket(int *sockfd, char *port){
	struct sockaddr_in addr;	// Address 2 bind
	
	(*sockfd) = socket(AF_INET, SOCK_STREAM, 0);
	// Set protocol, port and in-address
    addr.sin_family = AF_INET;
	// Using atoi() to convert ascii code into int
    addr.sin_port = htons(atoi(port));
	// Address to accept any incoming messages.
    addr.sin_addr.s_addr = INADDR_ANY;

	return bind(*sockfd, 
			(const struct sockaddr *)&addr, 
			sizeof(struct sockaddr_in));
}

void setnonblocking(int fd) {
	int opts;
	opts=fcntl(fd, F_GETFL);

	if(opts<0)
		errmsg("fcntl(sock,GETFL)");

	opts = opts|O_NONBLOCK;

	if(fcntl(fd, F_SETFL, opts)<0) 
		errmsg("fcntl(sock,SETFL,opts)");
}

void init_player(){
	int i;
	for(i=0;i<MAX_PLAYERS;i++)
		players[i].fd=-1;
}

void handler(int cfd){
	MSG msg;
	int i;

	while(1){
		while(read(cfd, &msg, MSG_LEN)<0);
	
		if(!strcmp(msg.cmd, "EN")){
			/* Client wanna Enter */
			handle_enter(cfd, &msg);

		}else if(!strcmp(msg.cmd, "EX")){
			/* Exit */
			handle_exit(cfd, &msg);
			break;

		}else if(!strcmp(msg.cmd, "M")){
			/* Move */
			handle_move(cfd, &msg);
			
		}else if(!strcmp(msg.cmd, "K")){
			/* Keep alive */
			continue;
		}

		strcpy(msg.cmd, "K");
	}
}

void handle_enter(int cfd, MSG *msg){
	PLAYER_FD *cur_p = NULL;
	int i;
	MSG resp;

	for(i=0;i < MAX_PLAYERS;i++){
		if(players[i].fd == cfd){
			/* Re-enter */
			strcpy(resp.cmd, "RE");

			write(cfd, &resp, MSG_LEN);
			return;
		}
		if(players[i].fd==-1){
			/* Place */
			cur_p=&(players[i]);
			break;
		}
	}
		
	
	if(cur_p == NULL){
		/* Full */
		strcpy(resp.cmd, "F");
		write(cfd, &resp, MSG_LEN);
		return;
	}

	/* Connected */
	sscanf(msg->content, 
			"%s%s",
			(cur_p->data).name, 
			&(cur_p->data.sign));

	cur_p->fd=cfd;

	printf("[INFO] %s[%c] Entered, fd=%d\n",
			cur_p->data.name,
			cur_p->data.sign, 
			cur_p->fd
		);
	
	strcpy(resp.cmd, "C");
	strcpy(resp.content, "");

	write(cfd, &resp, sizeof(resp));

	/* Broadcasting player-list */
	brc_plist();
}

void brc_plist(){
	int i,j;
	MSG msg;

	for(i=0;i<MAX_PLAYERS;i++){

		if((players[i].fd==-1) || 
			(players[j].fd==-1)) 
				continue;

		for(j=0;j<MAX_PLAYERS;j++){
			
			printf("now send %d to %d\n",players[j].fd, players[i].fd);

			if((players[i].fd==-1) || 
				(players[j].fd==-1)) 
				continue;
			
			pinfo_msg(&msg, &(players[j]));

			write(players[i].fd, &msg, MSG_LEN);
		}

		strcpy(msg.cmd, "PE");
		write(players[i].fd, &msg, MSG_LEN);
	}
}

void handle_exit(int cfd, MSG *msg){
	int i;
	for(i=0;i<MAX_PLAYERS;i++){
		if(players[i].fd==cfd || 
			players[i].fd == -1) 
			continue;

		write(players[i].fd, msg, sizeof(MSG));
		printf("[INFO] %s %s", msg->cmd, msg->content);
	}
}

void handle_move(int cfd, MSG *msg){
	int i;
	for(i=0;i<MAX_PLAYERS;i++){
		if(players[i].fd==cfd || 
			players[i].fd == -1) 
			continue;

		write(players[i].fd, msg, sizeof(MSG));
		printf("[INFO] %s %s", msg->cmd, msg->content);
	}
}

void pinfo_msg(MSG *msg, PLAYER_FD* plr){
	char fmt[32];
	strcpy(msg->cmd,"P");
	
	sprintf(fmt, "%s %c", plr->data.name, plr->data.sign);

	strcpy(msg->content, fmt);
}
void alrd_msg(MSG *msg){
	strcpy(msg->cmd, "A");
}
void exit_msg(MSG *msg, const char player_name[]){
	strcpy(msg->cmd, "EX"); 
	strcpy(msg->content, player_name);
}
void move_msg(MSG *msg, int x, int y){
	strcpy(msg->cmd, "M"); 
	char numstr[5];
	sprintf(numstr,"%d ",x);
	strcpy(msg->content, numstr); 
	sprintf(numstr,"%d",y);
	strcat(msg->content, numstr);
}
void result_msg(MSG *msg, 
			const char player_name[]){
	if(msg->cmd[0]!='R')
		strcpy(msg->cmd, "R ");
	else
		strcpy(msg->cmd, "");

	strcpy(msg->content, player_name);
}