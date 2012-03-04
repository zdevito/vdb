#include <FL/Fl.H>
#include <assert.h>
#include <stdio.h>
#include <vector.h>

#include "SocketManager.h"

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

static void report_error(const char * name);

static SocketManagerCallback_t callback_fn;
static void * callback_data;
static bool callbacks_paused;


#define BUFFER_SIZE (64 * 1024)
struct Buffer {
	Buffer(int fd_) {
		start = end = 0;
		fd = fd_;
	}
	int read() {
		if(start != 0) {
			size_t size = end - start;
			memmove(data,&data[start],size);
			start = 0;
			end = size;
		}
		int r = recv(fd,&data[end],BUFFER_SIZE - end,0);
		if(r > 0) {
			end += r;
		}
		return r;
	}
	bool getNextLine(const char * * l) {
		if(start < end) {
			size_t new_start = start;
			while(new_start < end && data[new_start] != '\n')
				new_start++;
			if(new_start < end) {
				data[new_start] = '\0'; 
				*l = &data[start];
				start = new_start + 1;
				return true;
			} else return false; //we have a partial line, need to wait for more data
		} else return false;
	}
	int fd;
	size_t start,end;
	char data[BUFFER_SIZE];
};

static std::vector<Buffer*> buffers;

static void input_handler(int fd, void * data) {	
	if(callbacks_paused)
		return;
		
	Buffer * buf = (Buffer *) data;
	
	assert(buf->fd == fd);
	
	int status = buf->read(); //read up to BUFFER_SIZE data and handle it, this will be called again if there is data left in the FD
	if(status == 0) {
		Fl::remove_fd(fd);
		close(fd);
		for(size_t i = 0; i < buffers.size(); i++) {
			if(buffers[i] == buf) {
				buffers.erase(buffers.begin() + i);
				break;
			}
		}
	} else if(status < 0) {
		report_error("read");
		exit(1);
	} else {
		const char * line;
		while(buf->getNextLine(&line)) {
			if(!callback_fn(line,callback_data)) {
				//a display clear has be request with a redraw pending,
				//we can not handle any more requests until it is fulfilled
				//at which time SocketManager_reenableCallbacks() will be called
				callbacks_paused = true; 
				return;
			}
		}
	}
}

void SocketManager_reenableCallbacks() {
	assert(callbacks_paused);
	callbacks_paused = false;
	//we may have lines left in the buffers that need to be flushed 
	for(size_t i = 0; i < buffers.size(); i++) {
		const char * line;
		while(buffers[i]->getNextLine(&line)) {
			if(!callback_fn(line,callback_data)) {
				callbacks_paused = true;
				return;
			}
		}
	}
}

static void new_connection(int fd, void * data) {
	struct sockaddr_in peer_name;
	socklen_t addrlen = sizeof(peer_name);
	int sock2 = accept(fd, (struct sockaddr*)&peer_name, &addrlen);
    if(sock2 == -1) {
    	report_error("accept");
    	exit(1);
    }
    Buffer * buf = new Buffer(sock2);
    buffers.push_back(buf);
    Fl::add_fd(sock2, FL_READ, input_handler, buf);
}


static void init_listener() {
	int sockd = socket(AF_INET, SOCK_STREAM, 0);
	//avoid address in use error that occur if we quit with a client connected
	int t = 1;
	int status = setsockopt(sockd,SOL_SOCKET,SO_REUSEADDR,(const char *)&t,sizeof(int)); 
	if(status == -1) {
		report_error("setsockopt");
		exit(1);
	}
	struct sockaddr_in name;
	name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
	name.sin_port = htons(10000);
  
	if (sockd == -1) {
   		report_error("socket");
   		exit(1);
  	}
  	status = bind(sockd, (struct sockaddr*)&name, sizeof(name));
  	if(status == -1) {
  		report_error("bind");
  		exit(1);
  	}
  	status = listen(sockd, 5);
  	if(status == -1) {
  		report_error("listen");
		exit(1);
  	}
  	Fl::add_fd(sockd, FL_READ, new_connection, NULL);
}

static void SocketManager_os_init(); //specific for each OS

void SocketManager_init(SocketManagerCallback_t cb, void *d) {
	callback_fn = cb;
	callback_data = d;
	SocketManager_os_init();
	init_listener();
}

#ifndef _WIN32

static void report_error(const char * name) {
	perror(name); //a little simpler on anything but windows, don't you think
}
static void SocketManager_os_init() {}


#else
static void SocketManager_os_init() {
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2,2), &wsaData)) {
    	exit(1);
    }
}
static void report_error(const char * name) {
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