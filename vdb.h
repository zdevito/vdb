#ifndef __VDB_H
#define __VDB_H

#define VDB_CALL static inline

VDB_CALL int vdb_point(float x, float y, float z);
VDB_CALL int vdb_line(float x0, float y0, float z0, 
                      float x1, float y1, float z1);
VDB_CALL int vdb_normal(float x, float y, float z, 
                        float dx, float dy, float dz);
VDB_CALL int vdb_triangle(float x0, float y0, float z0, 
                          float x1, float y1, float z1,
                          float x2, float y2, float z2);

VDB_CALL int vdb_color(float r, float g, float b);

//By default, vdb will refresh the contents of the view at the end of every API call,
//you can surround multiple calls to vdb functions with vdb_begin/vdb_end
//to prevent the viewer from refreshing the contents until all the draw calls have been made

VDB_CALL int vdb_begin();
VDB_CALL int vdb_end();

VDB_CALL int vdb_flush();

//create a new blank frame. Currently just clears the screen, but eventually the viewer may keep
//around the contents of previous frames for inspection.
VDB_CALL int vdb_frame();


//versions that take direct pointers to floating point data
//this works well if you have a Point or Line struct
VDB_CALL int vdb_point_v(void * p);
VDB_CALL int vdb_line_v(void * p);
VDB_CALL int vdb_normal_v(void * p);
VDB_CALL int vdb_triangle_v(void * p);                 
VDB_CALL int vdb_color_v(void * c);



VDB_CALL int vdb_sample(float p);
VDB_CALL int vdb_label(const char * lbl); 
VDB_CALL int vdb_label(int i); 

//for simplicity all the implementation to interface with vdb is in this header, just include it in your project

#include <stdarg.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
VDB_CALL int vdb_close(int i) { return close(i); }
#else
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")
typedef int socklen_t;
VDB_CALL int vdb_close(int i) { return closesocket(i); }
#endif

static void vdb_report_error();

#include <stdio.h>
#include <stdlib.h>

#include <map>
#include <string>

#define VDB_BUFFER_SIZE (64*1024)
//maximum characters in a vdb command
#define VDB_REDZONE_SIZE 512
static struct {
	int is_initialized;
	int init_error; 
	int in_group;
	int sample_enabled;
	int fd;
	size_t n_bytes;
	char buffer[VDB_BUFFER_SIZE];
	//TODO: remove c++ to make this header c compatible
	std::map< std::string, int> string_table;
} __vdb;


VDB_CALL int vdb_sample(float p) {
	if(p >= 1.f)
		return __vdb.sample_enabled = 1;
	else {
#ifndef __APPLE__
		double r = rand() / (double) RAND_MAX;
#else
		double r = arc4random() / (double) 0xFFFFFFFF;
#endif
		return __vdb.sample_enabled = (r < p);
	}
}

static void vdb_os_init();

VDB_CALL void vdb_exit() {
	if(__vdb.init_error == 0) {
		vdb_close(__vdb.fd);
		__vdb.init_error = 1;
	}
}
VDB_CALL int vdb_init() {
	if(!__vdb.is_initialized) {
		__vdb.is_initialized = 1;
		__vdb.sample_enabled = 1;
		vdb_os_init();
		__vdb.fd = socket(AF_INET, SOCK_STREAM, 0);
		if(__vdb.fd == -1) {
			vdb_report_error();
			__vdb.init_error = 1;
		} else {
			struct sockaddr_in serv_name;
			serv_name.sin_family = AF_INET;
			serv_name.sin_addr.s_addr = htonl(0x7F000001L);
			serv_name.sin_port = htons(10000);
			if(-1 == connect(__vdb.fd, (struct sockaddr*) &serv_name, sizeof(serv_name))) {
				vdb_report_error();
				__vdb.init_error = 1;
			}
			atexit(vdb_exit);
		}
	}
	if(__vdb.is_initialized == 2) { 
		//this never runs, but it tricks compilers into including these functions as debug symbols 
		//even though they may never be used
		//useful if you want to call them from the debugger directly
		vdb_point_v(NULL);
		vdb_line_v(NULL);
		vdb_normal_v(NULL);
		vdb_triangle_v(NULL);
		vdb_point(0,0,0);
		vdb_line(0,0,0,0,0,0);
		vdb_normal(0,0,0,0,0,0);
		vdb_triangle(0,0,0,0,0,0,0,0,0);
		vdb_color_v(NULL);
		vdb_color(0,0,0);
		vdb_label("");
		vdb_label(0);
	}
	return __vdb.init_error;
}

#define VDB_INIT do { if(vdb_init()) return 1; } while(0)

VDB_CALL int vdb_flush() {
	VDB_INIT;
	unsigned int s = send(__vdb.fd,__vdb.buffer,__vdb.n_bytes,0);
	if(s != __vdb.n_bytes) {
		vdb_report_error();
		__vdb.init_error = 1;
	}
	__vdb.n_bytes = 0;
	return 0;
}
VDB_CALL void vdb_raw_print(const char * fmt, ...) {
	va_list argp;
	va_start(argp,fmt);
	__vdb.n_bytes += vsnprintf(__vdb.buffer + __vdb.n_bytes, VDB_BUFFER_SIZE - __vdb.n_bytes,fmt,argp);
	va_end(argp);
	if(__vdb.buffer[__vdb.n_bytes-1] == '\n' &&
	   VDB_BUFFER_SIZE - __vdb.n_bytes < VDB_REDZONE_SIZE) {
		vdb_flush();
	}
}

VDB_CALL int vdb_refresh() {
	VDB_INIT;
	vdb_raw_print("r\n");
	vdb_flush();
	return 0;
}

VDB_CALL int vdb_print(char cmd, int N, int stride, int nelems, void * p) {
	VDB_INIT;
	if(!__vdb.sample_enabled)
		return 0;
	unsigned char * b = (unsigned char *) p;
	for(int i = 0; i < N; i++) {
		float * f = (float*)b;
		vdb_raw_print("%c ",cmd);
		for(int j = 0; j < nelems; j++) {
			vdb_raw_print("%f ",f[j]);
		}
		vdb_raw_print("\n");
		b += stride;
	}
	if(__vdb.in_group == 0)
		vdb_refresh(); //statements not between calls to vdb_begin() and vdb_end() are flushed immediately
	return 0;	
}


VDB_CALL int vdb_begin() {
	__vdb.in_group++;
	return 0;
}
VDB_CALL int vdb_end() {
	if(__vdb.in_group > 0)
		__vdb.in_group--;
	if(__vdb.in_group == 0)
		vdb_refresh();
	return 0;
}

VDB_CALL int vdb_point_fn(int N, void * p, int stride) {
	return vdb_print('p',N,stride,3,p);
}	

VDB_CALL int vdb_point_fn(int N, void * p) { 
	return vdb_point_fn(N,p,3*sizeof(float));
}
VDB_CALL int vdb_point_v(void * p) {
	return vdb_point_fn(1,p);
}
VDB_CALL int vdb_point(float x, float y, float z) {
	float p[] = {x,y,z};
	return vdb_point_v(p);
}

VDB_CALL int vdb_line_fn(int N, void * p, int stride) {
	return vdb_print('l',N,stride,6,p);
}
VDB_CALL int vdb_line_fn(int N, void * p) {
	return vdb_line_fn(N,p,sizeof(float) * 6);
}
VDB_CALL int vdb_line_v(void * p) {
	return vdb_line_fn(1,p);
}
VDB_CALL int vdb_line(float x0, float y0, float z0, float x1, float y1, float z1) {
	float p[] = {x0,y0,z0,x1,y1,z1};
	return vdb_line_v(p);
}


VDB_CALL int vdb_normal_fn(int N, void * p, int stride) {
	return vdb_print('n',N,stride,6,p);
}
VDB_CALL int vdb_normal_fn(int N, void * p) {
	return vdb_normal_fn(N,p,sizeof(float) * 6);
}
VDB_CALL int vdb_normal_v(void * p) {
	return vdb_normal_fn(1,p);
}
VDB_CALL int vdb_normal(float x0, float y0, float z0, float x1, float y1, float z1) {
	float p[] = {x0,y0,z0,x1,y1,z1};
	return vdb_normal_v(p);
}

VDB_CALL int vdb_triangle_fn(int N, void * p, int stride) {
	return vdb_print('t',N,stride,9,p);
}
VDB_CALL int vdb_triangle_fn(int N, void * p) {
	return vdb_triangle_fn(N,p,sizeof(float) * 9);
}
VDB_CALL int vdb_triangle_v(void * p) {
	return vdb_triangle_fn(1,p);
}
VDB_CALL int vdb_triangle(float x0, float y0, float z0, float x1, float y1, float z1,float x2, float y2, float z2) {
	float p[] = {x0,y0,z0,x1,y1,z1,x2,y2,z2};
	return vdb_triangle_v(p);
}
VDB_CALL int vdb_frame() {
	return vdb_print('f',1,0,0,NULL);
}
VDB_CALL int vdb_color_v(void * c) {
	return vdb_print('c',1,0,3,c);
}
VDB_CALL int vdb_color(float r, float g, float b) {
	float c[] = {r,g,b};
	return vdb_color_v(c);
}

//TODO: check and remove or quote newlines from lbl
//TODO: limit max size
VDB_CALL int vdb_intern(const char * lbl) {
	if(__vdb.string_table.count(lbl) == 0) {
		int key = __vdb.string_table.size();
		__vdb.string_table[lbl] = key;
		vdb_raw_print("s %d %s\n",key,lbl);
		return key;
	} else {
		return __vdb.string_table[lbl];
	}
}

VDB_CALL int vdb_group(int n, const char * lbl) {
	VDB_INIT;
	int key = vdb_intern(lbl);
	vdb_raw_print("g %d %d\n",n,key);
	return 0;
}

VDB_CALL int vdb_label(const char * lbl) {
	return vdb_group(0,lbl);
}
VDB_CALL int vdb_label(int i) {
	char buf[128];
	snprintf(buf,128,"%d",i);
	return vdb_label(buf);
}
VDB_CALL int vdb_callsite(const char * lbl) {
	return vdb_group(1,lbl);
}

#ifdef _WIN32
static void vdb_report_error() {
    int errCode = WSAGetLastError();
    LPSTR errString = NULL;
    int size = FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                 FORMAT_MESSAGE_FROM_SYSTEM,
                 0,
                 errCode,
                 0,
                 (LPSTR)&errString,
                 0,          
                 0 );             
     printf( "vdb: %s (%d)\n", errString, errCode);
     LocalFree( errString );
}
static void vdb_os_init() {
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2,2), &wsaData)) {
    	exit(1);
    }
}
#else
static void vdb_os_init() {}
static void vdb_report_error() { perror("vdb"); }
#endif



//these will be overshadowed by the macros that capture line numbers
//however, they are useful to have because they will be visible to gdb 
//so you can call them directly from the debugger
VDB_CALL int vdb_point_fn(void * p) {
	return vdb_point_v(p);
}
VDB_CALL int vdb_line_fn(void * p) {
	return vdb_line_v(p);
}
VDB_CALL int vdb_normal_fn(void * p) {
	return vdb_normal_v(p);
}
VDB_CALL int vdb_triangle_fn(void * p) {
	return vdb_triangle_v(p);
}

VDB_CALL int vdb_point_fn(float x, float y, float z) {
	return vdb_point(x,y,z);
}
VDB_CALL int vdb_line_fn(float x0, float y0, float z0, float x1, float y1, float z1) {
	return vdb_line(x0,y0,z0,x1,y1,z1);
}
VDB_CALL int vdb_normal_fn(float x0, float y0, float z0, float x1, float y1, float z1) {
	return vdb_normal(x0, y0, z0, x1, y1, z1);
}
VDB_CALL int vdb_triangle_fn(float x0, float y0, float z0, float x1, float y1, float z1,float x2, float y2, float z2) {
	return vdb_triangle(x0,y0,z0,x1,y1,z1,x2,y2,z2);
}

#define VDB_STRINGIFY2(x) #x
#define VDB_STRINGIFY(x) VDB_STRINGIFY2(x)
#define VDB_LINE vdb_callsite(__FILE__ ":" VDB_STRINGIFY(__LINE__))


#define vdb_point_v(p) do { \
	VDB_LINE; \
	vdb_point_fn(p); \
} while(0)

#define vdb_line_v(p) do { \
	VDB_LINE; \
	vdb_line_fn(p); \
} while(0)

#define vdb_normal_v(p) do { \
	VDB_LINE; \
	vdb_normal_fn(p); \
} while(0)

#define vdb_triangle_v(p) do { \
	VDB_LINE; \
	vdb_triangle_fn(p); \
} while(0)


#define vdb_point(x,y,z) do { \
	VDB_LINE; \
	vdb_point_fn(x,y,z); \
} while(0)
#define vdb_line(x0,y0,z0,x1,y1,z1) do { \
	VDB_LINE; \
	vdb_line_fn(x0,y0,z0,x1,y1,z1); \
} while(0)
#define vdb_normal(x0, y0, z0, x1, y1, z1) do { \
	VDB_LINE; \
	vdb_normal_fn(x0, y0, z0, x1, y1, z1); \
} while(0)
#define vdb_triangle(x0,y0,z0,x1,y1,z1,x2,y2,z2) do { \
	VDB_LINE; \
	vdb_triangle_fn(x0,y0,z0,x1,y1,z1,x2,y2,z2); \
} while(0)

#undef VDB_INIT
#undef VDB_CALL
#undef VDB_BUFFER_SIZE
#undef VDB_REDZONE_SIZE
#endif
