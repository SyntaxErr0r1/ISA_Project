#makefile for c++ project with libxml2
CC = g++
CFLAGS = -g -Wall -I/usr/include/libxml2 
LDFLAGS = -lxml2 -lssl -lcrypto
OBJS = main.o parser.cpp downloader.cpp
TARGET = main
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)
main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp
clean:
	rm -f $(OBJS) $(TARGET)