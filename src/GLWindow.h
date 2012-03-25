#ifndef _GL_WINDOW_H
#define _GL_WINDOW_H

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>
#include "Frame.h"
#include "StringTable.h"
#include "LabelTable.h"

struct recVec {
   GLdouble x,y,z;
};

struct recCamera {
	recVec viewPos; // View position
	recVec viewDir; // View direction vector
	recVec viewUp; // View up direction
	recVec rotPoint; // Point to rotate about
	GLdouble aperture; // pContextInfo->camera aperture
	GLint viewWidth, viewHeight; // current window/screen height and width
};

enum {
	LABEL_LINE,
	LABEL_GROUP,
	LABEL_SIZE
};

struct ClientState {
	ClientState() {
		for(int i = 0; i < N_COLOR_GROUPS; i++) {
			colors[i].r = colors[i].g = colors[i].b = 0.5f;
		}
	}
	std::map<int,int> client_key_to_string;
	Color colors[N_COLOR_GROUPS];
};

struct GLWindow : public Fl_Gl_Window {
    double point_size;                       
    double filter_value;                     
    void draw();
    void resize(int X,int Y,int W,int H);
    
    void interactive_clear();
    void clear(bool reset_bb);
    int handle(int event);
    void updateProjection();
    void updateModelView();
    void check_resize(int width, int height);
	void mouseDolly(int x, int y);
	void mousePan(int x, int y);
	void resetCamera();
	void mouseDown(int x, int y);
	void rightMouseDown(int x, int y);
	void otherMouseDown(int x, int y);
	void mouseUp(int x, int y);
	void mouseDragged(int x, int y);
	void scrollWheel(int x, int y, int delta_x, int delta_y);
	void prepareOpenGL(int width, int height);
	bool command(int client_id, const char * line);
	void set_color_by(int idx);
    GLWindow(int X,int Y,int W,int H);
    
    recCamera camera;
    GLfloat worldRotation [4];
	GLfloat objectRotation [4];
	GLfloat shapeSize;
	int scroll_delta[2];
	BBox current_bounds;
	bool refresh_posted;
	bool clear_posted;
	Frame * frame;
	StringTable string_table;
	int color_by;
	std::map< int, ClientState> client_state;
	LabelTable label_table[LABEL_SIZE];
};

#endif