

FLTK_CONFIG = local/bin/fltk-config
FLTK_TAR = fltk-1.3.0-source.tar.gz
FLTK_URL = http://ftp.easysw.com/pub/fltk/1.3.0/$(FLTK_TAR)
FLTK_DIR = build/fltk-1.3.0

CXX = clang++
FLAGS = -g $(shell $(FLTK_CONFIG) --use-gl --cxxflags)
LFLAGS = $(shell $(FLTK_CONFIG) --use-gl --ldstaticflags)

SRC = main.cpp VDBWindow.cpp SocketManager.cpp Frame.cpp trackball.cpp GLWindow.cpp
OBJS = $(SRC:.cpp=.o)
EXECUTABLE = vdb

all:	$(EXECUTABLE)

build/$(FLTK_TAR):
	curl $(FLTK_URL) -o build/$(FLTK_TAR)
	
$(FLTK_DIR):	build/$(FLTK_TAR)
	(cd build; tar -xf $(FLTK_TAR))
	
$(FLTK_CONFIG):	$(FLTK_DIR)
	(cd $(FLTK_DIR); ./configure --prefix=$(shell pwd)/local; make install)
	
build/%.o:	src/%.cpp $(FLTK_CONFIG)
	$(CXX) $(FLAGS) $< -c -o $@

$(EXECUTABLE):	$(addprefix build/, $(OBJS))
	$(CXX) $(LFLAGS) $^ -o $@
	
clean:	
	rm -rf build/*.o build/*.d
	rm -f $(EXECUTABLE)

purge: clean
	rm -rf build/*

# dependency rules
DEPENDENCIES = $(patsubst %.cpp,build/%.d,$(SRC))
build/%.d:	src/%.cpp $(FLTK_CONFIG)
	@$(CXX) $(FLAGS) -MM -MT '$@ $(@:.d=.o)' $< -o $@
	
-include $(DEPENDENCIES)
