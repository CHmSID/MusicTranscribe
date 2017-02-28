#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <AL/alc.h>

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QTimer>

#include <string>

using std::string;

class MainWindow;
class Audio;

class MainWidget : public QWidget
{
	Q_OBJECT
public:
	MainWidget(QApplication* parentApp, MainWindow* parentWin);
	~MainWidget();

signals:

private:
	QApplication* parentApp;
	MainWindow* parentWin;

	Audio* audio = nullptr;
	QTimer* logicTimer = nullptr;

	// Audio
	ALCcontext* alContext = nullptr;
	ALCdevice* alDevice = nullptr;

	// GUI
	QPushButton* playButton = nullptr;

	void initAudio();
	void checkForErrors(string prefix);
	void destroyAudio();

public slots:

private slots:
	void loadMusic();

	void logic();
	void playMusic();
	void stopMusic();
};

#endif // MAINWIDGET_H
