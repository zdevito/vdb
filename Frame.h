/*
 *  Frame.h
 *  vdb
 *
 *  Created by Zachary DeVito on 2/7/12.
 *  Copyright 2012 Stanford. All rights reserved.
 *
 */
#ifndef _FRAME_H
#define _FRAME_H

	typedef struct Frame Frame;
	Frame * Frame_init();
	void Frame_draw(Frame * f);
	void Frame_setColor3(Frame * f, float * r);
	void Frame_addTriangle(Frame * f, float * data); // (x,y,z) x 3
	void Frame_addLine(Frame * f, float * data); // (x,y,z) x 2
	void Frame_addPoint(Frame * f, float * data); //(x,y,z) x 1
	void Frame_free(Frame * f);

#endif