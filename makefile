#makefile for c++ project with libxml2
CFLAGS = `xml2-config --cflags` -g -Wall -Wextra -I/usr/include/libxml2 -L/usr/local/lib -static-libstdc++
LDFLAGS = `xml2-config --libs` -lxml2 -lssl -lcrypto
OBJS = feedreader.o parser.o downloader.o
TARGET = feedreader


CC=g++

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)
feedreader.o: feedreader.cpp
	$(CC) $(CFLAGS) -c feedreader.cpp $(LDFLAGS)
parser.o: parser.cpp
	$(CC) $(CFLAGS) -c parser.cpp $(LDFLAGS)
downloader.o: downloader.cpp
	$(CC) $(CFLAGS) -c downloader.cpp $(LDFLAGS)
test: $(TARGET)
	node test.js
clean:
	rm -f $(OBJS) $(TARGET)