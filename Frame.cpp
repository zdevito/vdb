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

struct Frame {
	DisplayBuffer tris;
	DisplayBuffer lines;
	DisplayBuffer points;
	float r,g,b; //current color
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
	}
}
extern "C" {
	Frame * Frame_init() {
		Frame * f = new Frame;
		f->tris.init();
		f->lines.init();
		f->points.init();
		f->r = f->g = f->b = .5f;
		return f;
	}
	void Frame_draw(Frame * f) {
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glPointSize(5.f);
		glScaled(1, 1, 1);
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
	void Frame_free(Frame * f) {
		f->tris.destroy();
		f->lines.destroy();
		f->points.destroy();
		delete f;
	}
}