#include "TSPGA.h"
#include "threads.h"


int main(int argc, char **argv) {
	Fl::lock();
	makeMainWindow(argv[0]);
	mainWindow->show(argc, argv);
	return Fl::run();
}
