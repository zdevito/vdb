/*
  Copyright (c) 2010-2011, Intel Corporation
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Intel Corporation nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.


   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
*/

#include "vdb.h"
static int mandel(double c_re, double c_im, int count) {
    double z_re = c_re, z_im = c_im;
    int i;
    for (i = 0; i < count; ++i) {
        if (z_re * z_re + z_im * z_im > 4.)
            break;

        double new_re = z_re*z_re - z_im*z_im;
        double new_im = 2.f * z_re * z_im;
        z_re = c_re + new_re;
        z_im = c_im + new_im;
    }

    return i;
}

double drand() {
#ifdef __APPLE__
	return arc4random() / (double) (0xFFFFFFFF);
#else
	return rand() / (double) RAND_MAX;
#endif
}


void mandelbrot_serial(double x0, double y0, double x1, double y1,
                       int width, int height, int maxIterations)
{
    double dx = (x1 - x0) / width;
    double dy = (y1 - y0) / height;


	while(1) {
		double x = x0 + drand() * width * dx;
		double y = y0 + drand() * height * dy;
		int r = mandel(x, y, maxIterations);
		double v = .5 * (1 - r / (double)maxIterations);
		vdb_color(.5+v,.5+v,.5+v);
		vdb_point(x,y,v / 10);
    }
}
static const int V_WIDTH = 1024 * 768;
int main() {
	int width = 1024;
	int height = V_WIDTH / width;
	
	mandelbrot_serial(-2,-1,1,1,width,height,100);
	return 0;
}
