CC = gcc
CXX = g++

QT_CFLAGS := $(shell pkg-config --cflags Qt5Core)
QT_LIBS := $(shell pkg-config --libs Qt5Core)

CFLAGS = -fPIC -O2
CXXFLAGS = -fPIC -O2 -std=c++11 $(QT_CFLAGS)

LDFLAGS = -lstdc++ $(QT_LIBS)

SRCS_C = foo.c
SRCS_CXX = test.cpp

OBJS_C = foo.o
OBJS_CXX = test.o

TARGET = test

all: $(TARGET)

$(TARGET): $(OBJS_C) $(OBJS_CXX)
	$(CXX) -o $(TARGET) $(OBJS_C) $(OBJS_CXX) $(LDFLAGS)

foo.o: foo.c foo.h
	$(CC) -c $(CFLAGS) foo.c -o foo.o

test.o: test.cpp foo.h
	$(CXX) -c $(CXXFLAGS) test.cpp -o test.o

clean:
	rm -f *.o test