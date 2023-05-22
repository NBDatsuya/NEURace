/* Frequently used functions: myheader.c */
# include "myheader.h"
void errmsg(char* msg){
    perror(msg);
	exit(1);
}

void usage(char* msg){
    alert_exit(msg);
	exit(1);
}
void alert_exit(char* msg){
    puts(msg);
    exit(1);
}