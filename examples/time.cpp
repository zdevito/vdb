#include "vdb.h"
#include <stdlib.h>
#include <sys/time.h>


double gettime() {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec + tv.tv_usec / (double) 1000000.0;
}

int main() {
	double begin = gettime();
	vdb_line(0,0,0,15,0,0);
	vdb_line(0,0,0,0,15,0);
	for(int i = 0; i < 1000000; i++) {
		vdb_point(150*i/1000000.0,gettime() - begin,0);
	}
	return 0;
}
