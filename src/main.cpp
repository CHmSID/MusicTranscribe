#include "mainwindow.h"
#include <QApplication>

#include <QDebug>
#include <QLabel>

int main(int argc, char *argv[])
{
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QApplication app(argc, argv);

    // Set the application to use Core OpenGL 3.3
    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(fmt);

    // Start the application
	MainWindow window(&app);
	window.show();

	return app.exec();
}
