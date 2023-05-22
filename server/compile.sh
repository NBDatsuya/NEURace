cc -c ../myheader.c
cc game_server.c myheader.o -o server
rm myheader.o