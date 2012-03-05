#include "vdb.h"
int main() {
	for(int x = 0; x < 10; x++) {
		for(int y = 0; y < 10; y++) {
			for(int z = 0; z < 10; z++) {
				if( (x - 5) * (x - 5) + 
				    (y - 5) * (y - 5) + 
				    (z - 5) * (z - 5) < 5 * 5) {
				    	vdb_point(x,y,z);
				}
			}
		}
	}
	return 0;
}