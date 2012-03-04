#ifndef _GL_WINDOW_H
#define _GL_WINDOW_H

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>

struct GLWindow : public Fl_Gl_Window {
    double point_size;                       
    double filter_value;                     
    void draw();
    void resize(int X,int Y,int W,int H);
    
    void clear();
    int handle(int event);
    GLWindow(int X,int Y,int W,int H);
};

#endif