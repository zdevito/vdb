CXX = g++
CC = gcc
FLAGS = -I.. -g -Wall -lm

EXAMPLES = $(basename $(wildcard *.cpp) $(wildcard *.c))
all:	$(EXAMPLES)
%:	%.cpp
	$(CXX) $< $(FLAGS) -o $@
%:  %.c
	$(CC) $< $(FLAGS) -o $@
clean:
	rm -f $(EXAMPLES)
