#include "controlwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	ControlWidget w;
	w.show();

	return a.exec();
}
