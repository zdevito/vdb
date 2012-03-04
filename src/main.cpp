#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <assert.h>
#include <stdio.h>

#include "VDBWindow.h"
#include "SocketManager.h"



bool read_line(const char * line, void * box) {
	printf("callback: %s\n",line);
	((Fl_Box*)box)->copy_label(line);
	return true;
}

int application_main(int argc, char ** argv) {
  Fl_Window *window = new Fl_Window(300,180);
  Fl_Box * box = new Fl_Box(20,40,260,100,"Hello, World!");
  box->box(FL_UP_BOX);
  box->labelsize(36);
  window->end();
  window->show();
  VDBWindow * win = new VDBWindow();
  win->show();
  SocketManager_init(read_line,box);
  return Fl::run();
}

#ifndef _WIN32

int main(int argc, char ** argv) {
	return application_main(argc, argv);
}

void report_error(const char * name) {
	perror(name); //a little simpler on anything but windows, don't you think
}

#else
int main(int argc, char ** argv) {
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2,2), &wsaData)) {
        return 1;
    }
	return application_main(argc, argv);
}
int CALLBACK WinMain(
  __in  HINSTANCE hInstance,
  __in  HINSTANCE hPrevInstance,
  __in  LPSTR lpCmdLine,
  __in  int nCmdShow
) {
	return main(0,NULL);
}
void report_error(const char * name) {
    int errCode = WSAGetLastError();
    LPSTR errString = NULL;
    int size = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                 FORMAT_MESSAGE_FROM_SYSTEM,
                 0,
                 errCode,
                 0,
                 (LPSTR)&errString,
                 0,          
                 0 );             
     printf( "%s: %s (%d)\n", name, errString, errCode);
     LocalFree( errString );
}
#endif