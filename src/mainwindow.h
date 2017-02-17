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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
	MainWindow(QApplication* app);
    ~MainWindow();

	QLabel* getStatusText() const;

private:

	void createMenuActions();
	void createMenus();

	QApplication* parent =  nullptr;
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

protected:
	void closeEvent(QCloseEvent *event);

};

#endif // MAINWINDOW_H
