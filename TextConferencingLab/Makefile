all: 
	clear && rm -f server deliver && gcc server.c -g -Wall -o server -lm && gcc -lm client.c -g -Wall -o client -lm && chmod +x server client

clean:
	clear && rm -f server deliver
