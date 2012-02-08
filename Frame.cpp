/*
 *  Frame.cpp
 *  vdb
 *
 *  Created by Zachary DeVito on 2/7/12.
 *  Copyright 2012 Stanford. All rights reserved.
 *
 */

extern "C" {
#include "Frame.h"
}
#include <vector.h>
#include <OpenGL/OpenGL.h>
#include <assert.h>
#include <float.h>
#include <math.h>

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
}
struct Point {
	float x,y,z;
	float r,g,b;
};

#define OFFSET(typ,field) (&((typ*)NULL)->field)

struct DisplayBuffer {
	std::vector<unsigned short> idx;
	std::vector<Point> pts;
	bool dirty;
	GLuint vbo;
	GLuint ibo;
	void init() {
		dirty = true;
		glGenBuffers(1,&vbo);
		glGenBuffers(1,&ibo);
	}
	void draw(GLenum mode) {
		if(dirty) {
			glDeleteBuffers(1,&vbo);
			glDeleteBuffers(1,&ibo);
			glGenBuffers(1,&vbo);
			glGenBuffers(1,&ibo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
			
			glBufferData(GL_ARRAY_BUFFER, sizeof(Point) * pts.size(), &pts[0], GL_STATIC_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * idx.size(), &idx[0], GL_STATIC_DRAW);
			dirty = false;
		} else {
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizeof(Point), OFFSET(Point,x));
		glColorPointer(3, GL_FLOAT, sizeof(Point), OFFSET(Point,r));
		glDrawElements(mode, idx.size(), GL_UNSIGNED_SHORT, NULL);
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
struct Frame {
	DisplayBuffer tris;
	DisplayBuffer lines;
	DisplayBuffer points;
	float r,g,b; //current color
	BBox bounds;
	float last_diag;
	bool normals_dirty;
	std::vector<Normal> normals;
};
static void Frame_initPoint(Frame * f, float * data, Point * p) {
	p->x = data[0];
	p->y = data[1];
	p->z = data[2];
	p->r = f->r;
	p->g = f->g;
	p->b = f->b;
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
extern "C" {
	Frame * Frame_init() {
		Frame * f = new Frame;
		f->tris.init();
		f->lines.init();
		f->points.init();
		f->r = f->g = f->b = .5f;
		f->normals_dirty = false;
		BBox_empty(&f->bounds);
		f->last_diag = 1.f;
		return f;
	}
	void Frame_draw(Frame * f, BBox * draw_box) {
		
		float diag = BBox_diagonal_length(draw_box);
		if(f->last_diag != diag || f->normals_dirty) {
			
			for(int i = 0; i < f->normals.size(); i++) {
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
		
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glPointSize(5.f);
		f->tris.draw(GL_TRIANGLES);
		f->lines.draw(GL_LINES);
		f->points.draw(GL_POINTS);
		glPopMatrix();
	}
	void Frame_setColor3(Frame * f, float * r) {
		f->r = r[0];
		f->g = r[1];
		f->b = r[2];
	}
	void Frame_addTriangle(Frame * f, float * data){  // (x,y,z) x 3
		Frame_addPoints(f,&f->tris,3,data);
	}
	void Frame_addLine(Frame * f, float * data) { // (x,y,z) x 2
		Frame_addPoints(f,&f->lines,2,data);
	}
	void Frame_addPoint(Frame * f, float * data) { //(x,y,z) x 1
		Frame_addPoints(f,&f->points,1,data);
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
}