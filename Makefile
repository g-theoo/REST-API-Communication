CC = gcc
CFLAGS = -Wall -Wextra

client: client.c requests.c helpers.c buffer.c parson.c
	$(CC) $(CFLAGS) -o client client.c requests.c helpers.c buffer.c parson.c

run: client
	./client

clean:
	rm -f *.o client
