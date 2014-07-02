#ifndef _FRAME_H
#define _FRAME_H

#include <stddef.h>
typedef struct BBox {
	union {
		float data[6];
		struct {
			float x0,y0,z0,x1,y1,z1;
		};
	};
} BBox;

#define N_COLOR_GROUPS 2
struct Color {
	union {
		float c[3];
		struct { float r; float g; float b; };
	};
};


void BBox_empty(BBox * b);
void BBox_all(BBox * b);
void BBox_union(BBox * b, float * point, BBox * r);
void BBox_intersect(BBox * a, BBox * b, BBox * r);
float BBox_diagonal_length(BBox * b);
bool BBox_center(BBox * b, float * r);
typedef struct Frame Frame;
Frame * Frame_init();
void Frame_draw(Frame * f, BBox * b, float point_size, int grp);
void Frame_clear(Frame * f, bool reset_bounds);
void Frame_setColor(Frame * f, Color * colors);
void Frame_addTriangle(Frame * f, float * data); // (x,y,z) x 3
void Frame_addLine(Frame * f, float * data); // (x,y,z) x 2
void Frame_addPoint(Frame * f, float * data); //(x,y,z) x 1
void Frame_addNormal(Frame * f, float * data);

void Frame_free(Frame * f);
void Frame_getBBox(Frame * f, BBox * b);
size_t Frame_nObjects(Frame * f);
void Frame_setVisibleRange(Frame * f, size_t l, size_t h);

#endif