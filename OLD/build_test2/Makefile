CC = gcc
CXX = g++

QT_CFLAGS := $(shell pkg-config --cflags Qt5Widgets Qt5Gui Qt5Core)
QT_LIBS := $(shell pkg-config --libs Qt5Widgets Qt5Gui Qt5Core)

CFLAGS = -fPIC -O2 -I..
CXXFLAGS = -fPIC -O2 -std=c++11 $(QT_CFLAGS)

LDFLAGS = -lstdc++ $(QT_LIBS)

SRCS_C = coordinates.c
SRCS_CXX = test.cpp

OBJS_C = coordinates.o
OBJS_CXX = test.o

TARGET = test

all: $(TARGET)

$(TARGET): $(OBJS_C) $(OBJS_CXX)
	$(CXX) -o $(TARGET) $(OBJS_C) $(OBJS_CXX) $(LDFLAGS)

coordinates.o: coordinates.c
	$(CC) -c $(CFLAGS) coordinates.c -o coordinates.o

test.o: test.cpp coordinates.h
	$(CXX) -c $(CXXFLAGS) -I.. test.cpp -o test.o

clean:
	rm -f *.o test