all: TCP_Sender TCP_Receiver

TCP_Sender: TCP_Sender.o
	gcc -Wall -g TCP_Sender.o -o TCP_Sender

TCP_Receiver: TCP_Receiver.o
	gcc -Wall -g TCP_Receiver.o -o TCP_Receiver

TCP_Receiver.o: TCP_Receiver.c
	gcc -Wall -g -c TCP_Receiver.c

TCP_Sender.o: TCP_Sender.c Head.h
	gcc -Wall -g -c TCP_Sender.c

clean:
	rm -f *.o TCP_Sender TCP_Receiver