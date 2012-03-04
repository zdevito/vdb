#include "../vdb.h"
#include <stdlib.h>
#include <math.h>

double drand() {
	return 2.0 * arc4random() / (double) (0xFFFFFFFF) - 1;
}

int main() {
	for(int i = 0; i < 10000; i++) {
		double x = drand();
		double y = drand();
		double z = drand();
		
		if(x * x + y * y + z * z < 1) {
			vdb_point(x,y,z);
		}
	}
	return 0;
}