#include "VDBWindow.h"

static Fl_Color bgcolor() {
    return fl_rgb_color(237,237,237);
}
    
static void slider_changed_wrapper(Fl_Widget *, void * win) {
    ((VDBWindow*) win)->slider_changed();
}
static void clear_wrapper(Fl_Widget *, void * win) {
    ((GLWindow*) win)->clear();
}
    
static void makePretty(Fl_Widget * w) {
    w->color(bgcolor());
    w->labelsize(12);
}

static void setupSlider(Fl_Slider * s, double low, double high, double value, void * data) {
	s->align(FL_ALIGN_TOP);
	s->type(FL_HOR_SLIDER);
	s->bounds(low, high);
	s->value(value);
	s->callback(slider_changed_wrapper, (void*)data);
	makePretty(s);
}

void VDBWindow::slider_changed() {
	gl->point_size = point_size->value();
	gl->filter_value = filter_value->value();
	gl->redraw();
}

VDBWindow::VDBWindow() : Fl_Window(640,480,"vdb") {
	gl = new GLWindow(0, 0, w(), h()-50);
	
	Fl_Group * group = new Fl_Group(0,h()-50,w(),50);
	
	point_size = new Fl_Slider(10, h()-30, 160, 20, "Point Size");
	setupSlider(point_size,0,1,1,this);
	filter_value = new Fl_Slider(170, h()-30, 160 , 20, "Filter");
	setupSlider(filter_value,0,1,0,this);
	slider_changed();
	clear_button = new Fl_Button(340, h() - 40, 40, 30, "Clear");
	clear_button->callback(clear_wrapper,this);
	makePretty(clear_button);
	
	makePretty(this);
	
	group->end();
	group->resizable(NULL);
	
	this->resizable(gl);
	this->size_range(480,400);
	
	this->end();
};