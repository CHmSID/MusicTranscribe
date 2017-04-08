#include "mainwindow.h"
#include <QApplication>

#include <QDebug>
#include <QLabel>

int main(int argc, char *argv[])
{
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QApplication app(argc, argv);

    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(fmt);

	MainWindow window(&app);
	window.show();

	return app.exec();
}
