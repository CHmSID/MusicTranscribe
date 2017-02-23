#include "mainwidget.h"

#include <QApplication>
#include "mainwindow.h"

MainWidget::MainWidget(QApplication* parentApp, MainWindow* parentWin)
{
	this->parentApp = parentApp;
	this->parentWin = parentWin;
}

MainWidget::~MainWidget()
{
	if(audio != nullptr)
		delete audio;

	destroyAudio();
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

	alcMakeContextCurrent(alContext);
	alcProcessContext(alContext);
}

void MainWidget::destroyAudio()
{
	alcDestroyContext(alContext);
	alcCloseDevice(alDevice);
}
