CC = gcc
CFLAGS = -g
CXX = g++
CXXFLAGS = -g

EXE = rv32i

SRCS = $(shell find . -name '*.cpp' -or  -name '*.c' | grep -v tests/ | grep -v res/)
OBJS_C = $(SRCS:.c=.o)
OBJS = $(OBJS_C:.cpp=.o)

.PHONY: all clean

all: $(EXE)
clean:
	rm -f $(EXE) $(OBJS)

$(EXE): $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) $(LOADLIBES) $(LDLIBS) -o $(EXE)
