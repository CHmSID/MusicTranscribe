#include "mainwindow.h"
#include <QApplication>

#include <QDebug>
#include <QLabel>

int main(int argc, char *argv[])
{
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QApplication app(argc, argv);

	MainWindow window(&app);
	window.show();

	return app.exec();
}
