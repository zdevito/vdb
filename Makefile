.SUFFIXES:
UNAME := $(shell uname)
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


ifeq ($(UNAME), Darwin)
ARCHIVE = vdb-osx
APPLICATION = $(EXECUTABLE).app
else
ARCHIVE = vdb-linux
APPLICATION = $(EXECUTABLE)
endif

.PHONY:	all purge clean examples release
all:	$(APPLICATION) examples
examples:
	make -C examples


build/$(FLTK_TAR):	
ifeq ($(UNAME), Darwin)
	curl $(FLTK_URL) -o build/$(FLTK_TAR)
else
	wget $(FLTK_URL) -O build/$(FLTK_TAR)
endif
	
$(FLTK_DIR):	build/$(FLTK_TAR)
	(cd build; tar -xf $(FLTK_TAR))
	
$(FLTK_CONFIG):	$(FLTK_DIR)
	(cd $(FLTK_DIR); ./configure --prefix=$(shell pwd)/local; make install)
	
build/%.o:	src/%.cpp $(FLTK_CONFIG)
	$(CXX) $(FLAGS) $< -c -o $@

$(EXECUTABLE):	$(addprefix build/, $(OBJS))
	$(CXX) $^ -o $@ $(LFLAGS)

$(EXECUTABLE).app:	$(EXECUTABLE)
	mkdir -p $@/Contents/Resources $@/Contents/MacOS
	echo APPLnone > $@/Contents/PkgInfo
	cp $(EXECUTABLE) $@/Contents/MacOS/$(EXECUTABLE)
	cp src/info.plist $@/Contents/info.plist
	
clean:
	make -C examples clean
	rm -rf build/*.o build/*.d build/$(ARCHIVE)
	rm -rf $(EXECUTABLE) $(ARCHIVE).tar.gz $(APPLICATION)

purge: clean
	rm -rf build/* local/*

$(ARCHIVE).tar.gz:	$(APPLICATION) vdb.h README.md
	mkdir -p build/$(ARCHIVE)
	cp -r $^ build/$(ARCHIVE)
	(cd build; tar czf ../$@ $(ARCHIVE))

release:	$(ARCHIVE).tar.gz

# dependency rules
DEPENDENCIES = $(patsubst %.cpp,build/%.d,$(SRC))
build/%.d:	src/%.cpp $(FLTK_CONFIG)
	@$(CXX) $(FLAGS) -MM -MT '$@ $(@:.d=.o)' $< -o $@
	
-include $(DEPENDENCIES)
