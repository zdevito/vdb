CXX = g++
FLAGS = -I..

EXAMPLES = $(basename $(wildcard *.cpp))
all:	$(EXAMPLES)
%:	%.cpp
	$(CXX) $(FLAGS) $< -o $@
clean:
	rm -f $(EXAMPLES)