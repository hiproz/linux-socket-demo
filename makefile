
#LDFLAGS = -l

all : client server

client : client.o
	gcc -g $(LDFLAGS) $^ -o $@
server : server.o
	gcc -g $(LDFLAGS) $^ -o $@ 


.PHONY : clean
clean :
	client server client.o server.o

