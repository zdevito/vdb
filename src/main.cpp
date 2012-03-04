#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <assert.h>
#include <stdio.h>

#ifndef _WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#else
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")
typedef int socklen_t;
int close(int i) { return closesocket(i); }
#endif

Fl_Box *box;

void report_error(const char * name);

void input_handler(int fd, void * data) {
	static int i = 0;
	char buf[1024];
	
	int size = recv(fd,buf,1023,0);
	buf[size] = '\0';
	if(size == 0) {
		printf("close\n");
		Fl::remove_fd(fd);
		close(fd);
	} else {
		box->copy_label(buf);
	}
}
void new_connection(int fd, void * data) {
	struct sockaddr_in peer_name;
	socklen_t addrlen = sizeof(peer_name);
	int sock2 = accept(fd, (struct sockaddr*)&peer_name, &addrlen);
    if(sock2 == -1) {
    	report_error("accept");
    	exit(1);
    }
    Fl::add_fd(sock2, FL_READ, input_handler, NULL);
    
}

void init_listener() {
	int sockd = socket(AF_INET, SOCK_STREAM, 0);
	//avoid address in use error that occur if we quit with a client connected
	int t = 1;
	int status = setsockopt(sockd,SOL_SOCKET,SO_REUSEADDR,(const char *)&t,sizeof(int)); 
	if(status == -1) {
		report_error("setsockopt");
		exit(2);
	}
	struct sockaddr_in name;
	name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
	name.sin_port = htons(10000);
  
	if (sockd == -1) {
   		report_error("socket");
   		exit(3);
  	}
  	status = bind(sockd, (struct sockaddr*)&name, sizeof(name));
  	if(status == -1) {
  		report_error("bind");
  		exit(4);
  	}
  	status = listen(sockd, 5);
  	if(status == -1) {
  		report_error("listen");
		exit(5);
  	}
  	Fl::add_fd(sockd, FL_READ, new_connection, NULL);
}


int application_main(int argc, char ** argv) {
  Fl_Window *window = new Fl_Window(300,180);
  box = new Fl_Box(20,40,260,100,"Hello, World!");
  box->box(FL_UP_BOX);
  box->labelsize(36);
  window->end();
  window->show(argc, argv);
  init_listener();
  return Fl::run();
}

#ifdef _WIN32
int main(int argc, char ** argv) {
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2,2), &wsaData)) {
        // WSAStartup failed
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
#else
int main(int argc, char ** argv) {
	return application_main(argc, argv);
}
void report_error(const char * name) {
	perror(name); //a little simpler on anything but windows, don't you thin
}
#endif