#include<vdb.h>
#include<math.h>

int main() {
    float t = 0.f;
    while(1) {
        float x,y;
        vdb_begin();
        vdb_frame();
        for(x = 0; x < 10.0; x += .1) {
            for(y = 0; y < 10.0; y += .1) {
                vdb_label_i( ((int)x)*10 + (int) y);
                vdb_point(x,y,sin(x+t)+cos(y+t));
            }
        }
        vdb_end();
        t += .1;
    }
	return 0;
}
