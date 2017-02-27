#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <AL/alc.h>

#include <QApplication>
#include <QWidget>

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

	// Audio
	ALCcontext* alContext = nullptr;
	ALCdevice* alDevice = nullptr;

	void initAudio();
	void destroyAudio();

public slots:

private slots:
	void loadMusic();
};

#endif // MAINWIDGET_H
