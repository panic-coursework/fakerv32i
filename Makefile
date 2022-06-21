CC = gcc
CFLAGS = -g
CXX = g++
CXXFLAGS = -g

EXE = rv32i

SRCS = lib/vector.cpp clk.c reg.c main.c
OBJS_C = $(SRCS:.c=.o)
OBJS = $(OBJS_C:.cpp=.o)

.PHONY: all clean

all: $(EXE)
clean:
	rm -f $(EXE) $(OBJS)

$(EXE): $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) $(LOADLIBES) $(LDLIBS) -o $(EXE)
