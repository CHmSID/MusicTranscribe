#include "mainwidget.h"

#include <QApplication>
#include <QFileDialog>
#include "mainwindow.h"
#include "audio.h"

MainWidget::MainWidget(QApplication* parentApp, MainWindow* parentWin)
{
	this->parentApp = parentApp;
	this->parentWin = parentWin;

	logicTimer = new QTimer();
	connect(logicTimer, SIGNAL(timeout()),
	        this, SLOT(logic()));
	logicTimer->start(32);

	playButton = new QPushButton("Play", this);
	connect(playButton, SIGNAL(released()),
	        this, SLOT(playMusic()));

	initAudio();
}

MainWidget::~MainWidget()
{
	delete logicTimer;

	if(audio != nullptr)
		delete audio;

	destroyAudio();

	delete playButton;
}

void MainWidget::loadMusic()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "VMT", "VMT");
	settings.beginGroup("LastDirs");
	QString lastDir = settings.value("ImportMusicDir").toString();

	QString fileName = QFileDialog::getOpenFileName(this, tr("Import Music"),
	    QDir(lastDir).absolutePath(), tr("MP3 (*.mp3);;OGG (*.ogg);;WAV (*.wav)"));

	if(fileName.isEmpty())
		return;

	QFileInfo newDir(fileName);

	settings.setValue("ImportMusicDir", newDir.absolutePath());
	settings.endGroup();

	if(audio != nullptr){

		delete audio;
		audio = nullptr;
	}

	audio = new Audio(fileName.toStdString().c_str());

	if(audio == nullptr){

		qDebug() << "Could not load audio";
	}
}

void MainWidget::logic()
{
	if(audio != nullptr)
	{
		if(audio->isPlaying())
		{
			audio->update();
		}
	}
}

void MainWidget::playMusic()
{
	if(audio != nullptr)
	{
		if(!audio->isPlaying())
		{
			audio->play();
		}
		else
		{
			audio->pause();
		}
	}
}

void MainWidget::stopMusic()
{
	if(audio != nullptr)
	{
		audio->reset();
	}
}

void MainWidget::initAudio()
{
	const ALCchar* defaultDevice = alcGetString(
	    nullptr, ALC_DEFAULT_DEVICE_SPECIFIER
	);

	alDevice = alcOpenDevice(defaultDevice);
	if(alDevice == nullptr)
		printf("Could not open AL Device\n");

	alContext = alcCreateContext(alDevice, nullptr);
	if(alContext == nullptr)
		printf("Could not create AL Context\n");

	if(!alcMakeContextCurrent(alContext))
		qDebug() << "Failed to create context!";

	alcProcessContext(alContext);

	int error = alGetError();
	while(error != AL_NO_ERROR)
	{
		qDebug() << "ERROR! in context creation";
	}
}

void MainWidget::destroyAudio()
{
	alcDestroyContext(alContext);
	alcCloseDevice(alDevice);
}
