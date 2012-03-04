#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>

#include "GLWindow.h"

static void FixViewport(int W,int H) {
	glLoadIdentity();
    glViewport(0,0,W,H);
    glOrtho(-W,W,-H,H,-1,1);
}


GLWindow::GLWindow(int X,int Y,int W,int H) : Fl_Gl_Window(X,Y,W,H,NULL) {
    end();
}

void GLWindow::draw() {
	printf("draw\n");
	if (!valid()) { valid(1); FixViewport(w(), h()); }      // first time? init
	// Clear screen to bg color
	//glClearColor(bg, bg, bg, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_LINE_STRIP); glVertex2f(w(), h()); glVertex2f(-w(),-h()); glEnd();
	glBegin(GL_LINE_STRIP); glVertex2f(w(),-h()); glVertex2f(-w(), h()); glEnd();
}
void GLWindow::resize(int X,int Y,int W,int H) {
    Fl_Gl_Window::resize(X,Y,W,H);
    FixViewport(W,H);
    redraw();
}

void GLWindow::clear() {
    printf("CLEAR\n");
}


int GLWindow::handle(int event) {
	switch(event) {
		case FL_PUSH:
			switch(Fl::event_button()) {
				case FL_LEFT_MOUSE:
					printf("Click.\n");
					break;
				case FL_RIGHT_MOUSE:
					printf("Right Click\n");
					break;
			}
			break;
		case FL_DRAG:
			printf("DRAG\n");
			break;
		case FL_RELEASE:
			printf("RELEASE\n");
			break;
		case FL_MOUSEWHEEL:
			printf("WHEEL %d %d\n",Fl::event_dx(),Fl::event_dy());
			break;
		default:
			return Fl_Gl_Window::handle(event);
			break;
	}
	return 1;
}