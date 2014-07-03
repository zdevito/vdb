
#include <vdb.h>

void rot(int n, int *x, int *y, int rx, int ry);
//convert d to (x,y)
void d2xy(int n, int d, int *x, int *y) {
    int rx, ry, s, t=d;
    *x = *y = 0;
    for (s=1; s<n; s*=2) {
        rx = 1 & (t/2);
        ry = 1 & (t ^ rx);
        rot(s, x, y, rx, ry);
        *x += s * rx;
        *y += s * ry;
        t /= 4;
    }
}
 
//rotate/flip a quadrant appropriately
void rot(int n, int *x, int *y, int rx, int ry) {
    int t;
    if (ry == 0) {
        if (rx == 1) {
            *x = n-1 - *x;
            *y = n-1 - *y;
        }
        t  = *x;
        *x = *y;
        *y = t;
    }
}


int main() {
	int N = 512;
	for(int i = 0; i < N * N - 1; i++) {
		int x0,y0,x1,y1;
		d2xy(N,i,&x0,&y0);
		d2xy(N,i+1,&x1,&y1);
		vdb_line((float)x0,y0,0,x1,y1,0);
	}
}
