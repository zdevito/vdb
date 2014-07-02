#include "VDBWindow.h"

static Fl_Color bgcolor() {
    return fl_rgb_color(237,237,237);
}
    
static void slider_changed_wrapper(Fl_Widget *, void * win) {
    ((VDBWindow*) win)->slider_changed();
}
static void clear_wrapper(Fl_Widget *, void * win) {
    ((GLWindow*) win)->interactive_clear();
}
static void color_by_wrapper(Fl_Widget *, void * win) {
	VDBWindow * w = (VDBWindow*)win;
	w->gl->set_color_by(w->color_by->value());
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

VDBWindow::VDBWindow() : Fl_Window(640+180,480,"vdb") {
	gl = new GLWindow(0, 0, w() - 180, h());
	
	Fl_Group * group = new Fl_Group(640,0,180,h());
	
	point_size = new Fl_Slider(640+10, 20, 160, 20, "Point Size");
	setupSlider(point_size,1,5,5,this);
	filter_value = new Fl_Slider(640+10, 60, 160 , 20, "Filter");
	setupSlider(filter_value,0,1,1,this);
	
	color_by = new Fl_Choice(640+30, 100, 110, 20, "Color By");
	color_by->align(FL_ALIGN_TOP);
	color_by->add("vdb_color",0,color_by_wrapper,this);
	color_by->add("vdb_label",0,color_by_wrapper,this);
	color_by->value(0);
	makePretty(color_by);
	
	clear_button = new Fl_Button(640+130, h() - 40, 40, 30, "Clear");
	clear_button->callback(clear_wrapper,gl);
	makePretty(clear_button);
	
	makePretty(this);
	for(int i = 0; i < N_CATEGORY_COLORS; i++) {
		Fl::set_color(i+8,category_colors[i][0],category_colors[i][1],category_colors[i][2]);
	}
	
	static int column_widths[] = { 30, 100, 0 };
	Fl_Browser * b = gl->legend = new Fl_Browser(640+10,130, 160, h() - 130 - 50);
	b->column_widths(column_widths);
	b->format_char('@');
	b->column_char('\t');
	b->hide();
	group->end();
	group->resizable(NULL);
	
	this->resizable(gl);
	this->size_range(500,480);
	
	this->end();
};