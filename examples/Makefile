CXX = g++
FLAGS = -I.. -g -Wall

EXAMPLES = $(basename $(wildcard *.cpp))
all:	$(EXAMPLES)
%:	%.cpp
	$(CXX) $(FLAGS) $< -o $@
clean:
	rm -f $(EXAMPLES)