#include "vdb.h"
#include <stdlib.h>
#include <math.h>

double drand() {
	return arc4random() / (double) (0xFFFFFFFF);
}

int main() {
	vdb_color(1,0,1);
	for(int i = 0; i < 100000; i++) {
		double s0 = drand();
		double s1 = drand();
		
		double r = s0;
		double t = 2 * M_PI * s1;
		
		vdb_point(r * sin(t), r * cos(t), 0);
		r = s0 * s0;
		vdb_point(r * sin(t), r * cos(t), 1);
		r = sqrt(s0);
		vdb_point(r * sin(t), r * cos(t), 2);
	}
	return 0;
}
