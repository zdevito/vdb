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

VDB_CALL int vdb_batched(int enabled);
VDB_CALL int vdb_flush();

//create a new blank frame. Currently just clears the screen, but eventually the viewer may keep
//around the contents of previous frames for inspection.
VDB_CALL int vdb_frame();


//utility functions to draw more than 1 object with a single call.
VDB_CALL int vdb_point(int N, void * p, int stride);
VDB_CALL int vdb_point(int N, float * p);
VDB_CALL int vdb_point(float * p);

VDB_CALL int vdb_line(int N, void * p, int stride);
VDB_CALL int vdb_line(int N, float * p);
VDB_CALL int vdb_line(float * p);

VDB_CALL int vdb_normal(int N, void * p, int stride);
VDB_CALL int vdb_normal(int N, float * p);
VDB_CALL int vdb_normal(float * p);

VDB_CALL int vdb_triangle(int N, void * p, int stride);
VDB_CALL int vdb_triangle(int N, float * p);
VDB_CALL int vdb_triangle(float * p);
                 
VDB_CALL int vdb_color(float * c);
VDB_CALL int vdb_sample(float p);
 

//for simplicity all the implementation to interface with vdb is in this header, just include it in your project

#include <arpa/inet.h>
#include <stdarg.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
int vdb_close(int i) { return close(i); }
#else
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")
typedef int socklen_t;
int vdb_close(int i) { return closesocket(i); }
#endif

#include <stdio.h>
#include <stdlib.h>

#define VDB_BUFFER_SIZE (64*1024)
//maximum characters in a vdb command
#define VDB_REDZONE_SIZE 128
static struct {
	int is_initialized;
	int init_error; 
	int in_group;
	int sample_enabled;
	int batched; //if true, we only issue sends when the buffer is full, even if refreshes are posted
	int fd;
	size_t n_bytes;
	char buffer[VDB_BUFFER_SIZE];
} __vdb;


VDB_CALL int vdb_sample(float p) {
	if(p >= 1.f)
		return __vdb.sample_enabled = 1;
	else {
		double r = arc4random() / (double) 0xFFFFFFFF;
		return __vdb.sample_enabled = (r < p);
	}
}

VDB_CALL int vdb_init() {
	if(!__vdb.is_initialized) {
		__vdb.is_initialized = 1;
		__vdb.sample_enabled = 1;
		
		__vdb.fd = socket(AF_INET, SOCK_STREAM, 0);
		if(__vdb.fd == -1) {
			perror("vdb");
			__vdb.init_error = 1;
		} else {
			struct sockaddr_in serv_name;
			serv_name.sin_family = AF_INET;
			inet_aton("127.0.0.1",&serv_name.sin_addr);
			serv_name.sin_port = htons(10000);
			if(-1 == connect(__vdb.fd, (struct sockaddr*) &serv_name, sizeof(serv_name))) {
				perror("vdb");
				__vdb.init_error = 1;
			}
		}
	}
	return __vdb.init_error;
}

#define VDB_INIT do { if(vdb_init()) return 1; } while(0)

int vdb_flush() {
	VDB_INIT;
	int s = send(__vdb.fd,__vdb.buffer,__vdb.n_bytes,0);
	if(s != __vdb.n_bytes) {
		perror("vdb");
		__vdb.init_error = 1;
	}
	__vdb.n_bytes = 0;
	return 0;
}
void vdb_raw_print(const char * fmt, ...) {
	va_list argp;
	va_start(argp,fmt);
	__vdb.n_bytes += vsnprintf(__vdb.buffer + __vdb.n_bytes, VDB_BUFFER_SIZE - __vdb.n_bytes,fmt,argp);
	va_end(argp);
	if(__vdb.buffer[__vdb.n_bytes-1] == '\n' &&
	   VDB_BUFFER_SIZE - __vdb.n_bytes < VDB_REDZONE_SIZE) {
		if(__vdb.batched == 2) {
			__vdb.batched = 1;
			//we suppressed refresh meshes until the buffer was full, now we need to insert one
			//flush will be called on the recursion
			vdb_raw_print("r\n");
		} else {
			vdb_flush();
		}
	}
}

VDB_CALL int vdb_refresh() {
	VDB_INIT;
	if(!__vdb.batched) {
		vdb_raw_print("r\n");
		vdb_flush();
	} else {
		__vdb.batched = 2;
	}
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

VDB_CALL int vdb_batched(int enabled) {
	__vdb.batched = enabled;
}


VDB_CALL int vdb_begin() {
	__vdb.in_group++;
}
VDB_CALL int vdb_end() {
	if(__vdb.in_group > 0)
		__vdb.in_group--;
	if(__vdb.in_group == 0)
		vdb_refresh();
}

VDB_CALL int vdb_point(int N, void * p, int stride) {
	return vdb_print('p',N,stride,3,p);
}	

VDB_CALL int vdb_point(int N, float * p) { 
	return vdb_point(N,p,3*sizeof(float));
}
VDB_CALL int vdb_point(float * p) {
	return vdb_point(1,p);
}
VDB_CALL int vdb_point(float x, float y, float z) {
	float p[] = {x,y,z};
	return vdb_point(p);
}

VDB_CALL int vdb_line(int N, void * p, int stride) {
	return vdb_print('l',N,stride,6,p);
}
VDB_CALL int vdb_line(int N, float * p) {
	return vdb_line(N,p,sizeof(float) * 6);
}
VDB_CALL int vdb_line(float * p) {
	return vdb_line(1,p);
}
VDB_CALL int vdb_line(float x0, float y0, float z0, float x1, float y1, float z1) {
	float p[] = {x0,y0,z0,x1,y1,z1};
	return vdb_line(p);
}


VDB_CALL int vdb_normal(int N, void * p, int stride) {
	return vdb_print('n',N,stride,6,p);
}
VDB_CALL int vdb_normal(int N, float * p) {
	return vdb_normal(N,p,sizeof(float) * 6);
}
VDB_CALL int vdb_normal(float * p) {
	return vdb_normal(1,p);
}
VDB_CALL int vdb_normal(float x0, float y0, float z0, float x1, float y1, float z1) {
	float p[] = {x0,y0,z0,x1,y1,z1};
	return vdb_normal(p);
}

VDB_CALL int vdb_triangle(int N, void * p, int stride) {
	return vdb_print('t',N,stride,9,p);
}
VDB_CALL int vdb_triangle(int N, float * p) {
	return vdb_triangle(N,p,sizeof(float) * 9);
}
int vdb_triangle(float * p) {
	return vdb_triangle(1,p);
}
VDB_CALL int vdb_triangle(float x0, float y0, float z0, float x1, float y1, float z1,float x2, float y2, float z2) {
	float p[] = {x0,y0,z0,x1,y1,z1,x2,y2,z2};
	return vdb_triangle(p);
}
VDB_CALL int vdb_frame() {
	return vdb_print('f',1,0,0,NULL);
}
VDB_CALL int vdb_color(float * c) {
	return vdb_print('c',1,0,3,c);
}
VDB_CALL int vdb_color(float r, float g, float b) {
	float c[] = {r,g,b};
	return vdb_color(c);
}

#undef VDB_INIT
#undef VDB_CALL
#undef VDB_BUFFER_SIZE
#undef VDB_REDZONE_SIZE
#endif