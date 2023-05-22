cc -c ../myheader.c 
cc game_client.c myheader.o -o client -lcurses -lpthread
rm myheader.o