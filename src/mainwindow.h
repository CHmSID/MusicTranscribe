#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/*
 * MainWindow is a sort of wrapper around the main part of the application.
 * It provides a menu and a status bar.
 */

#include <QMainWindow>
#include <QApplication>
#include <QStatusBar>
#include <QtWidgets>

class MainWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
	MainWindow(QApplication* app);
    ~MainWindow();

	QLabel* getStatusText() const;

private:
	QApplication* parent =  nullptr;
	MainWidget* mainWidget = nullptr;
	QStatusBar* statusBar = nullptr;
	QLabel* statusText =    nullptr;

	QMenuBar* menuBar =     nullptr;
	QMenu* fileMenu =       nullptr;
	QMenu* importMenu =     nullptr;
	QMenu* helpMenu =       nullptr;
	QMenu* aboutMenu =      nullptr;

	QAction* importMusicAction = nullptr;
	QAction* aboutQtAction =     nullptr;
	QAction* exitAction =        nullptr;

	void createMenuActions();
	void createMenus();

protected:
	void closeEvent(QCloseEvent *event);

};

#endif // MAINWINDOW_H
