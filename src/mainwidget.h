#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <AL/alc.h>

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <string>

using std::string;

class MainWindow;
class Audio;

class KeyboardWidget;
class SpectrumWidget;
class WaveformWidget;
class ScrollBar;

class MainWidget : public QWidget
{
	Q_OBJECT
public:
	MainWidget(QApplication* parentApp, MainWindow* parentWin);
	~MainWidget();

signals:

private:
	QApplication* parentApp = nullptr;
	MainWindow* parentWin = nullptr;

	QTimer* logicTimer = nullptr;

	// Audio
	Audio*      audio = nullptr;
	ALCcontext* alContext = nullptr;
	ALCdevice*  alDevice = nullptr;

	// GUI
	QPushButton* playButton = nullptr;
	QPushButton* stopButton = nullptr;
    QPushButton* followButton = nullptr;
    QSlider* speedSlider = nullptr;
    QSlider* toneSlider = nullptr;

    QLabel** speedLabels;
    QLabel** toneLabels;

	QVBoxLayout mainLayout;
	QHBoxLayout controlsLayout;
	QVBoxLayout keyboardLayout;

	bool followMusicMarker = true;

	// Widgets
	KeyboardWidget* keyboardWidget = nullptr;
	SpectrumWidget* spectrumWidget = nullptr;
	WaveformWidget* waveformWidget = nullptr;
	ScrollBar*      waveformScrollbar = nullptr;

	// Audio
	void initAudio();
	void checkForErrors(string prefix);
	void destroyAudio();

	// GUI
	void createLayout();
	void createButtons();
	void createWidgets();

public slots:

private slots:
	void loadMusic();

	void logic();
	void playMusic();
	void stopMusic();

	void turnOffMusicFollowing();

protected slots:
	void setFollowingMusicMarker(bool set);

protected:
	void resizeEvent(QResizeEvent *);
};

#endif // MAINWIDGET_H
