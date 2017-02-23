#include "mainwindow.h"
#include "mainwidget.h"

#include <QLabel>
#include <QSettings>
#include <QPoint>

MainWindow::MainWindow(QApplication *parent)
    :parent(parent)
{
	setWindowTitle("4th Year Project");
	mainWidget = new MainWidget(parent, this);
	setCentralWidget(mainWidget);

	createMenuActions();
	createMenus();

	setMenuBar(menuBar);

	// Set the window position from the previously saved state
	QSettings settings(QSettings::IniFormat,
	                   QSettings::UserScope, "FYP", "FYP");
	settings.beginGroup("MainWindow");
	QPoint pos = settings.value("Position").toPoint();
	QSize size = settings.value("Size").toSize();

	if(size.width() == -1 || size.height() == -1)
	   size = QSize(1280, 720);

	move(pos);
	resize(size);

	statusBar = new QStatusBar();
	statusText = new QLabel("");
	setStyleSheet("QStatusBar::item { border: 0px solid black};");
	statusBar->addPermanentWidget(statusText, 0);
	setStatusBar(statusBar);
}

MainWindow::~MainWindow()
{

	delete statusText;
	delete statusBar;
}

QLabel* MainWindow::getStatusText() const
{
	return statusText;
}

/*
 * This event registers the X button being pressed
 * or a key combination of Alt-F4 being used.
 *
 * It also handles saving of position and size of the window
 * before it's closed, so when the user opens the application
 * it'll remember it's last position.
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
	QSettings settings(QSettings::IniFormat,
	                   QSettings::UserScope, "FYP", "FYP");
	settings.beginGroup("MainWindow");
	settings.setValue("Position", pos());
	settings.setValue("Size", size());
	settings.endGroup();

	event->accept();
}

/*
 * This function creates various actions for use in the menu bar.
 * Those have to be created first before the menus are created.
 */
void MainWindow::createMenuActions()
{
	exitAction = new QAction("Exit", this);
	connect(exitAction, SIGNAL(triggered()),
	        this, SLOT(close()));

	aboutQtAction = new QAction("Qt", this);
	connect(aboutQtAction, SIGNAL(triggered()),
	        parent, SLOT(aboutQt()));

	importMusicAction = new QAction("Music", this);
	connect(importMusicAction, SIGNAL(triggered()),
	        mainWidget, SLOT(loadMusic()));
}

/*
 * This function creates the menus such as File, Edit or Help
 */
void MainWindow::createMenus()
{
	menuBar = new QMenuBar(this);

	fileMenu = new QMenu("File");

	importMenu = new QMenu("Import");
	importMenu->addAction(importMusicAction);
	fileMenu->addMenu(importMenu);

	fileMenu->addAction(exitAction);
	menuBar->addMenu(fileMenu);

	helpMenu = new QMenu("Help");

	aboutMenu = new QMenu("About");
	aboutMenu->addAction(aboutQtAction);
	helpMenu->addMenu(aboutMenu);
	menuBar->addMenu(helpMenu);
}
