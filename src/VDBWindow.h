#ifndef _VDBWINDOW_H
#define _VDBWINDOW_H

#include "GLWindow.h"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Browser.H>

struct VDBWindow : public Fl_Window {
    GLWindow *gl;          
    Fl_Slider * point_size;       
    Fl_Slider * filter_value;
    Fl_Button * clear_button;
    Fl_Choice * color_by;
    void slider_changed();
    VDBWindow();
};
#endif