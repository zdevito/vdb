/*
 *  vdb.h
 *  vdb
 *
 *  Created by Zach Devito on 2/8/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

//for simplicity all the implementation to interface with vdb is in this header, just include it in your project

#include <stdio.h>

static FILE * __vdb_file = NULL;
static unsigned int __vdb_in_group = 0;

int vdb_init() {
	if(!__vdb_file) {
		__vdb_file = fopen("/tmp/vdb","w");
		if(!__vdb_file) {
			perror("vdb");
			return 1;
		}
		return 0;
	}
	return 0;
}

#define VDB_INIT do { if(vdb_init()) return 1; } while(0)

int vdb_flush() {
	VDB_INIT;
	fprintf(__vdb_file,"r\n");
	fflush(__vdb_file);
	return 0;
}

int vdb_print(char cmd, int N, int stride, int nelems, void * p) {
	VDB_INIT;
	unsigned char * b = (unsigned char *) p;
	for(int i = 0; i < N; i++) {
		float * f = (float*)b;
		fprintf(__vdb_file,"%c ",cmd);
		for(int j = 0; j < nelems; j++) {
			fprintf(__vdb_file,"%f ",f[j]);
		}
		fprintf(__vdb_file,"\n");
		b += stride;
	}
	if(__vdb_in_group == 0)
		vdb_flush(); //statements not between calls to vdb_begin() and vdb_end() are flushed immediately
	return 0;	
}



int vdb_begin() {
	__vdb_in_group++;
}
int vdb_end() {
	if(__vdb_in_group > 0)
		__vdb_in_group--;
	if(__vdb_in_group == 0)
		vdb_flush();
}

int vdb_point(int N, void * p, int stride) {
	return vdb_print('p',N,stride,3,p);
}	

int vdb_point(int N, float * p) { 
	return vdb_point(N,p,3*sizeof(float));
}
int vdb_point(float * p) {
	return vdb_point(1,p);
}
int vdb_point(float x, float y, float z) {
	float p[] = {x,y,z};
	return vdb_point(p);
}



int vdb_line(int N, void * p, int stride) {
	return vdb_print('l',N,stride,6,p);
}
int vdb_line(int N, float * p) {
	return vdb_line(N,p,sizeof(float) * 6);
}
int vdb_line(float * p) {
	return vdb_line(1,p);
}
int vdb_line(float x0, float y0, float z0, float x1, float y1, float z1) {
	float p[] = {x0,y0,z0,x1,y1,z1};
	return vdb_line(p);
}


int vdb_normal(int N, void * p, int stride) {
	return vdb_print('n',N,stride,6,p);
}
int vdb_normal(int N, float * p) {
	return vdb_normal(N,p,sizeof(float) * 6);
}
int vdb_normal(float * p) {
	return vdb_normal(1,p);
}
int vdb_normal(float x0, float y0, float z0, float x1, float y1, float z1) {
	float p[] = {x0,y0,z0,x1,y1,z1};
	return vdb_normal(p);
}

int vdb_triangle(int N, void * p, int stride) {
	return vdb_print('t',N,stride,9,p);
}
int vdb_triangle(int N, float * p) {
	return vdb_triangle(N,p,sizeof(float) * 9);
}
int vdb_triangle(float * p) {
	return vdb_triangle(1,p);
}
int vdb_triangle(float x0, float y0, float z0, float x1, float y1, float z1,float x2, float y2, float z2) {
	float p[] = {x0,y0,z0,x1,y1,z1,x2,y2,z2};
	return vdb_triangle(p);
}
int vdb_frame() {
	return vdb_print('f',1,0,0,NULL);
}
int vdb_color(float * c) {
	return vdb_print('c',1,0,3,c);
}
int vdb_color(float r, float g, float b) {
	float c[] = {r,g,b};
	return vdb_color(c);
}

#undef VDB_INIT