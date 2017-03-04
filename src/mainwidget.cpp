#include "mainwidget.h"

#include <QApplication>
#include <QFileDialog>
#include <QSize>

#include "mainwindow.h"
#include "audio.h"
#include "keyboardwidget.h"
#include "spectrumwidget.h"
#include "waveformwidget.h"
#include "scrollbar.h"

MainWidget::MainWidget(QApplication* parentApp, MainWindow* parentWin)
{
	this->parentApp = parentApp;
	this->parentWin = parentWin;

	logicTimer = new QTimer();
	connect(logicTimer, SIGNAL(timeout()),
	        this, SLOT(logic()));
	logicTimer->start(32);

	createButtons();
	createWidgets();
	createLayout();

	initAudio();
}

MainWidget::~MainWidget()
{
	delete logicTimer;

	if(audio != nullptr)
		delete audio;

	destroyAudio();

	delete playButton;
	delete stopButton;

	delete keyboardWidget;
	delete spectrumWidget;
	delete waveformWidget;
	delete waveformScrollbar;
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
			playButton->setIcon(QIcon(":/data/icons/pause.png"));
			audio->play();
		}
		else
		{
			playButton->setIcon(QIcon(":/data/icons/play-button.png"));
			audio->pause();
		}
	}
}

void MainWidget::stopMusic()
{
	if(audio != nullptr)
	{
		playButton->setIcon(QIcon(":/data/icons/play-button.png"));
		audio->reset();
	}
}

void MainWidget::createButtons()
{
	playButton = new QPushButton(QIcon(":/data/icons/play-button.png"), "", this);
	connect(playButton, SIGNAL(released()),
	        this, SLOT(playMusic()));
	playButton->setFixedSize(QSize(32, 32));

	stopButton = new QPushButton(QIcon(":/data/icons/stop.png"), "", this);
	connect(stopButton, SIGNAL(released()),
	        this, SLOT(stopMusic()));
	stopButton->setFixedSize(QSize(32, 32));
}

void MainWidget::createLayout()
{
	mainLayout.setSpacing(1);
	controlsLayout.setSpacing(1);

	controlsLayout.addWidget(playButton);
	controlsLayout.addWidget(stopButton);
	controlsLayout.addStretch(32);

	mainLayout.addLayout(&controlsLayout);
	mainLayout.addWidget(waveformScrollbar);
	mainLayout.addWidget(waveformWidget);
	mainLayout.addWidget(keyboardWidget);
	mainLayout.addWidget(spectrumWidget);

	setLayout(&mainLayout);
}

void MainWidget::createWidgets()
{
	keyboardWidget = new KeyboardWidget(parentApp, this);
	keyboardWidget->setMinimumSize(100, 100);
	keyboardWidget->setStyleSheet("background-color:red;");

	spectrumWidget = new SpectrumWidget(parentApp, this);
	spectrumWidget->setMinimumSize(100, 100);
	spectrumWidget->setStyleSheet("background-color:black;");

	waveformWidget = new WaveformWidget(parentApp, this);
	waveformWidget->setMinimumSize(100, 100);
	waveformWidget->setStyleSheet("background-color:blue;");

	waveformScrollbar = new ScrollBar(Qt::Orientation::Horizontal,
	                                  waveformWidget);
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
