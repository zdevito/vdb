#include "vdb.h"
#include <stdlib.h>
#include <math.h>


double drand() {
#ifdef __APPLE__
	return arc4random() / (double) (0xFFFFFFFF);
#else
	return rand() / (double) RAND_MAX;
#endif
}


int main() {
	for(int i = 0; i < 10000; i++) {
		double x = 2*drand()-1;
		double y = 2*drand()-1;
		double z = 2*drand()-1;
		
		if(x * x + y * y + z * z < 1) {
			vdb_point(x,y,z);
		}
	}
	return 0;
}