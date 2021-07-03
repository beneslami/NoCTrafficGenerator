
CC = g++
CFLAGS = -Wall -c
main.o: main.cpp
	$(CC) $(CFLAGS) -o main.o main.cpp
MessageType.o: MessageType.cpp
	 $(CC) $(CFLAGS) -o MessageType.o MessageType.cpp
PacketQueue.o: PacketQueue.cpp
	$(CC) $(CFLAGS) -o PacketQueue.o PacketQueue.cpp
RandomGenerator.o: RandomGenerator.cpp
	$(CC) $(CFLAGS) -o RandomGenerator.o RandomGenerator.cpp
TrafficGenerator.o: TrafficGenerator.cpp
	$(CC) $(CFLAGS) -o TrafficGenerator.o TrafficGenerator.cpp
all : main.o MessageType.o PacketQueue.o RandomGenerator.o TrafficGenerator.o
	$(CC) -o main main.o MessageType.o PacketQueue.o RandomGenerator.o TrafficGenerator.o

clean:
	rm -rf main main.o MessageType.o PacketQueue.o RandomGenerator.o TrafficGenerator.o