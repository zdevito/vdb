#include <FL/Fl.H>
#include "VDBWindow.h"

int main(int argc, char ** argv) {
  VDBWindow * win = new VDBWindow();
  win->show();
  return Fl::run();
}

#ifdef _WIN32
int CALLBACK WinMain(
  __in  HINSTANCE hInstance,
  __in  HINSTANCE hPrevInstance,
  __in  LPSTR lpCmdLine,
  __in  int nCmdShow
) {
	return main(0,NULL);
}
#endif