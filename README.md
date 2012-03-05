vdb: the printf of visual debugging
===================================

Debugging visual programs like graphics, physical simulation, or
plotting is difficult. Errors in math functions typically do not change
program control flow, so the errors manifest long after they were
introduced. Printing out coordinates is typically not very helpful.
Typically, the best approach is to try to visualize intermediate values,
but setting up an OpenGL viewer and figuring out how to link it into
your large application is often too much effort.

vdb allows you to debug visual programs using a simple interface to draw
points, lines, and triangles:


```
int vdb_point(float x, float y, float z);
int vdb_line(float x0, float y0, float z0, 
             float x1, float y1, float z1);
int vdb_normal(float x, float y, float z, 
               float dx, float dy, float dz);
int vdb_triangle(float x0, float y0, float z0,
                 float x1, float y1, float z1,
                 float x2, float y2, float z2);
```

vdb includes an interactive viewer that runs in a separate process from
your code; so you can use vdb by simply including its stand-alone header
file.  There is no need to link any external libraries into the
application you want to debug.

Using vdb 
---------

1. Download the viewer application and header file, or compile it from
source:

	* [vdb-osx.tar.gz](https://github.com/downloads/zdevito/vdb/vdb-osx.tar.gz)
	* [vdb-win.zip](https://github.com/downloads/zdevito/vdb/vdb-win.zip)
	* [vdb-source.zip](https://github.com/zdevito/vdb/zipball/master)

2. Run the viewer:
	
	```
	$ ./vdb &
	```

3. Insert vdb calls into you program. Here is an example that samples a
sphere at regular intervals:

	```
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
	```

4. Compile and run your code as you normally would, just make sure your
build system can find the vdb.h header file. Don't worry about adjusting
your objects to appear on the screen -- vdb will center the camera on
the objects that you draw.

Other calls
-----------
* `vdb_frame()` start a new frame by clearing the scene. This is useful if you are debugging an animation.
* `vdb_sample(float prob)` calls between this call and the next call to vdb_sample will only be drawn with probability `prob` (either all the calls are performed or none of them). You can use this to easily subsample a large set of data.
* `vdb_begin()/vdb_end()` calls between begin and end statements will appear atomically on the screen. This is completely optional but can prevent the flickering that might occur if the screen refreshed while you are drawing an object.

Remote debugging
----------------
vdb can debug programs on remote computers as well. vdb listens for
drawing commands on port 10000. For safety, it only accepts connections
for the local machine. You can, however, use port forwarding to debug
code remotely. For instance if you are running the viewer on your local
machine, you can debug a program on a server foo:

```
$ ./vdb
$ ssh -R 10000:127.0.0.1:10000 foo
$ ./my_remote_program
```

Compiling vdb
-------------

The included Makefile can be used to compile vdb on linux and OSX:

```
$ make
```

It will download a version of the fltk library used for the UI.



