/*
 *  Frame.cpp
 *  vdb
 *
 *  Created by Zachary DeVito on 2/7/12.
 *  Copyright 2012 Stanford. All rights reserved.
 *
 */
#define NOMINMAX
#include "Frame.h"
#include <vector>
#include <FL/gl.h>
#include <FL/glu.h>
#include <assert.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <limits.h>

#ifdef _WIN32
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#include "wglext.h"
#include "glext.h"
#endif

#ifndef __APPLE__

#ifdef _WIN32
#define glGetProcAddress(x) wglGetProcAddress(x)
#endif

#ifdef __linux__
#include <GL/glx.h>
#include <GL/glext.h>
#define glGetProcAddress(x) glXGetProcAddressARB((const GLubyte*) (x))
#endif

#define OPENGL_PROCS(_) \
	_(glDeleteBuffers,GLDELETEBUFFERS) \
	_(glGenBuffers,GLGENBUFFERS) \
	_(glBindBuffer,GLBINDBUFFER) \
	_(glBufferData,GLBUFFERDATA) \

#define DECLARE_FNS(nm,typ) \
	static PFN##typ##PROC nm;

OPENGL_PROCS(DECLARE_FNS)

static void Frame_init_gl() {
#define DEFINE_FNS(nm,typ) \
	nm = (PFN##typ##PROC) glGetProcAddress(#nm); \
	assert(nm != NULL && #nm);
	OPENGL_PROCS(DEFINE_FNS)
}
#else
static void Frame_init_gl() {}
#endif
void BBox_empty(BBox * b) {
	for(int i = 0; i < 3; i++)
		b->data[i] = FLT_MAX;
	for(int i = 0; i < 3; i++)
		b->data[i+3] = FLT_MIN;
}
void BBox_all(BBox * b) {
	for(int i = 0; i < 3; i++)
		b->data[i] = FLT_MIN;
	for(int i = 0; i < 3; i++)
		b->data[i+3] = FLT_MAX;
}
void BBox_add(BBox * b, float * point, BBox * r) {
	for(int i = 0; i < 3; i++)
		r->data[i] = std::min(b->data[i],point[i]);
	for(int i = 0; i < 3; i++)
		r->data[i+3] = std::max(b->data[i+3],point[i]);
}
void BBox_union(BBox * a, BBox * b, BBox * r) {
	for(int i = 0; i < 3; i++)
		r->data[i] = std::min(a->data[i],b->data[i]);
	for(int i = 0; i < 3; i++)
		r->data[i+3] = std::max(a->data[i+3],b->data[i+3]);
}
void BBox_intersect(BBox * a, BBox * b, BBox * r) {
	for(int i = 0; i < 3; i++)
		r->data[i] = std::max(a->data[i],b->data[i]);
	for(int i = 0; i < 3; i++)
		r->data[i+3] = std::min(a->data[i+3],b->data[i+3]);
}
bool BBox_equal(BBox * a, BBox * b) {
	for(int i = 0; i < 6; i++)
		if(b->data[i] != b->data[i])
			return false;
	return true;
}
float BBox_diagonal_length(BBox * b) {
	float dx = std::max(0.f,b->x1 - b->x0);
	float dy = std::max(0.f,b->y1 - b->y0);
	float dz = std::max(0.f,b->z1 - b->z0);
	return sqrtf(dx * dx + dy * dy + dz * dz);
}
bool BBox_center(BBox * b, float * r) {
	r[0] = (b->x0 + b->x1) / 2.f;
	r[1] = (b->y0 + b->y1) / 2.f;
	r[2] = (b->z0 + b->z1) / 2.f;
	return false;
}

struct Point {
	float x,y,z;
	Color colors[N_COLOR_GROUPS];
};

#define OFFSET(typ,field) (&((typ*)NULL)->field)

struct DisplayBuffer {
	std::vector<unsigned int> idx;
	std::vector<Point> pts;
	size_t n_elements_in_draw_list;
	bool dirty;
	bool has_buffers;
	GLuint vbo;
	GLuint ibo;
	void init() {
		dirty = true;
		has_buffers = false;
		n_elements_in_draw_list = 0;
	}
	
	void refresh() {
		if(dirty) {
			if(has_buffers) {
				glDeleteBuffers(1,&vbo);
				glDeleteBuffers(1,&ibo);
			}
			has_buffers = true;
			glGenBuffers(1,&vbo);
			glGenBuffers(1,&ibo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
			
			if(pts.size() > 0) {
				glBufferData(GL_ARRAY_BUFFER, sizeof(Point) * pts.size(), &pts[0], GL_STATIC_DRAW);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * idx.size(), &idx[0], GL_STATIC_DRAW);
			}
			n_elements_in_draw_list = idx.size();
			dirty = false;
		}
	}
	void draw(GLenum mode, int color_group, size_t begin, size_t end_) {
		size_t end = std::min(end_,n_elements_in_draw_list);
		if(begin < end) {
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);
			glVertexPointer(3, GL_FLOAT, sizeof(Point), begin * sizeof(Point) + OFFSET(Point,x));
			glColorPointer(3, GL_FLOAT, sizeof(Point),  begin * sizeof(Point) + OFFSET(Point,colors[color_group]));
			glDrawElements(mode, end - begin, GL_UNSIGNED_INT, NULL);
		}
	}
	void clear() {
		dirty = true;
		pts.clear();
		idx.clear();
	}
	void addIndex() {
		dirty = true;
		idx.push_back(idx.size());
	}
	void addIndex(unsigned short i) {
		dirty = true;
		idx.push_back(i);
	}
	void addPoint(Point * p) {
		dirty = true;
		pts.push_back(*p);
	}
	void destroy() {
		glDeleteBuffers(1,&vbo);
		glDeleteBuffers(1,&ibo);
	}
};
struct Normal {
	size_t line_idx; //index into lines list where the first point describing this normal lives
	size_t point_idx; //index into poitns list where point for normal is
};

struct Sizes {
	size_t n_tri;
	size_t n_lines;
	size_t n_points;
};

struct Frame {
	DisplayBuffer tris;
	DisplayBuffer lines;
	DisplayBuffer points;
	Color * colors;
	BBox bounds;
	float last_diag;
	bool normals_dirty;
	std::vector<Normal> normals;
	std::vector<Sizes> history;
	size_t low,high;
	float line_size,point_size;
};
static void Frame_initPoint(Frame * f, float * data, Point * p) {
	p->x = data[0];
	p->y = data[1];
	p->z = data[2];
	assert(f->colors);
	memcpy(&p->colors[0],f->colors, sizeof(Color) * N_COLOR_GROUPS);
}
static void Frame_addPoints(Frame * f, DisplayBuffer * buffer, int n, float * ps) {
	for(int i = 0; i < n; i++) {
		Point p;
		Frame_initPoint(f, ps + i * 3, &p);
		buffer->addPoint(&p);
		buffer->addIndex();
		BBox_add(&f->bounds,ps + i * 3,&f->bounds);
	}
}
static void Frame_mark(Frame * f) {
	Sizes s = { f->tris.idx.size(), f->lines.idx.size(), f->points.idx.size() };
	f->history.push_back(s);
}
Frame * Frame_init() {
	Frame * f = new Frame;
	f->point_size = 5.0;
	f->line_size = 1.0;
	
	f->tris.init();
	f->lines.init();
	f->points.init();
	f->colors = NULL;
	BBox_empty(&f->bounds);
	f->last_diag = 1.f;
	Frame_clear(f,false);
	return f;
}
void Frame_refresh(Frame * f, BBox * draw_box) {
	float diag = BBox_diagonal_length(draw_box);
	if(f->last_diag != diag || f->normals_dirty) {
		
		for(size_t i = 0; i < f->normals.size(); i++) {
			Normal & n = f->normals[i];
			Point * p = &f->lines.pts[n.line_idx];
			float dx = p[1].x - p[0].x;
			float dy = p[1].y - p[0].y;
			float dz = p[1].z - p[0].z;
			float l = sqrtf(dx * dx + dy * dy + dz * dz);
			
			float s = 1.f/diag * .1 / l;
			p[1].x = p[0].x + s * dx;
			p[1].y = p[0].y + s * dy;
			p[1].z = p[0].z + s * dz;
			memcpy(&f->points.pts[n.point_idx].x, &p[1].x,sizeof(float) * 3);
			f->lines.dirty = true;
			f->points.dirty = true;
		}
		f->last_diag = diag;
	}
	f->tris.refresh();
	f->lines.refresh();
	f->points.refresh();
}
void Frame_clear(Frame * f,bool reset_bounds) {
	f->normals_dirty = false;
	f->normals.clear();
	f->tris.clear();
	f->lines.clear();
	f->points.clear();
	f->history.clear();
	f->low = 0;
	f->high = INT_MAX;
	Frame_mark(f);
	if(reset_bounds)
		BBox_empty(&f->bounds);
}
void Frame_draw(Frame * f, float point_size, int color_group) {
	Frame_init_gl();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	
	Sizes & l = f->history[std::min(f->low,f->history.size() - 1)];
	Sizes & h = f->history[std::min(f->high,f->history.size() - 1)];
	
	glPointSize(point_size);
	f->tris.draw(GL_TRIANGLES,color_group,l.n_tri,h.n_tri);
	f->lines.draw(GL_LINES,color_group,l.n_lines,h.n_lines);
	f->points.draw(GL_POINTS,color_group,l.n_points,h.n_points);
	
	glPopMatrix();
}
void Frame_setColor(Frame * f, Color * colors) {
	f->colors = colors;
}
size_t Frame_nObjects(Frame * f) {
	return f->history.size();
}
void Frame_setVisibleRange(Frame * f, size_t l, size_t h) {
	f->low = l;
	f->high = h;
}
void Frame_addTriangle(Frame * f, float * data){  // (x,y,z) x 3
	Frame_addPoints(f,&f->tris,3,data);
	Frame_mark(f);
}
void Frame_addLine(Frame * f, float * data) { // (x,y,z) x 2
	Frame_addPoints(f,&f->lines,2,data);
	Frame_mark(f);
}
void Frame_addPoint(Frame * f, float * data) { //(x,y,z) x 1
	Frame_addPoints(f,&f->points,1,data);
	Frame_mark(f);
}
void Frame_addNormal(Frame * f, float * data) { //ray of (x,y,z) (dx,dy,dz)
	f->normals_dirty = true;
	float p[6] = {data[0],data[1],data[2], data[0] + data[3],data[1] + data[4],data[2] + data[5]};
	Normal n;
	n.line_idx = f->lines.pts.size();
	n.point_idx = f->points.pts.size();
	f->normals.push_back(n);
	Frame_addPoint(f, p + 3);
	Frame_addLine(f, p);
	Frame_mark(f);
}
void Frame_free(Frame * f) {
	f->tris.destroy();
	f->lines.destroy();
	f->points.destroy();
	delete f;
}
void Frame_getBBox(Frame * f, BBox * b) {
	memcpy(b,&f->bounds,sizeof(BBox));
}
