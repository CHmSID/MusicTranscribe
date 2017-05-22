#include "mainwidget.h"

#include <QApplication>
#include <QFileDialog>
#include <QDialog>
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

	createWidgets();
    createButtons();
	createLayout();

	initAudio();

	// When the user uses scrollbar, move the waveform up/down
	connect(waveformScrollbar, SIGNAL(valueChanged(int)),
	        waveformWidget, SLOT(setPosition(int)));

	// When the waveform moves (due to following a marker), update the scrollbar
	connect(waveformWidget, SIGNAL(positionChanged(int)),
	        waveformScrollbar, SLOT(subValue(int)));

    // When the user presses on the scrollbar, turn off following the marker
	connect(waveformScrollbar, SIGNAL(sliderPressed()),
	        this, SLOT(turnOffMusicFollowing()));
}

MainWidget::~MainWidget()
{
	delete logicTimer;

	if(audio != nullptr)
		delete audio;

	destroyAudio();

	delete playButton;
	delete stopButton;
    delete speedSlider;
    delete toneSlider;

	delete keyboardWidget;
	delete spectrumWidget;
	delete waveformWidget;
	delete waveformScrollbar;
}

void MainWidget::resizeEvent(QResizeEvent *)
{
	waveformScrollbar->setMaximum(waveformWidget->getSize() * 100);
	waveformScrollbar->setMinimum(0);
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

    // Loading dialog for long songs
    QProgressDialog* dialog = new QProgressDialog("Loading", "cancel", 0, 100, this);
    dialog->show();
    dialog->setMinimumDuration(3000);
    dialog->setAutoReset(false);
    dialog->setModal(true);
    dialog->setAutoClose(false);

    audio = new Audio(fileName.toStdString().c_str(), dialog);

	if(audio == nullptr){

		qDebug() << "Could not load audio";
	}
	else
	{
		// Set the window title to the music file name
		int i = fileName.lastIndexOf("/", -1) + 1;
		parentWin->setWindowTitle(fileName.right(fileName.length() - i));

		/*
		 * The following code resets the waveform and
		 * regenerates the visual data.
		 * The properties of the scrollbar are also changed
		 * according to the length of the music
		 */
		waveformWidget->clearChunks();
		waveformWidget->clearMarkers();
        waveformWidget->generateData(audio);
		waveformScrollbar->setMaximum(waveformWidget->getSize() * 100);
		waveformScrollbar->setMinimum(0);
		waveformScrollbar->setValue(0);

        spectrumWidget->connectToAudio(audio);
		spectrumWidget->calculateSpectrum(2048);
	}

    // Connect the slider to our audio
    connect(speedSlider, SIGNAL(valueChanged(int)),
            audio, SLOT(updateStretchFactor(int)));

    connect(toneSlider, SIGNAL(valueChanged(int)),
            audio, SLOT(updateToneTransposition(int)));

    delete dialog;
}

void MainWidget::logic()
{
	if(audio != nullptr)
	{
		if(audio->isPlaying())
		{
			audio->update();

            // If we're playing in real time, use small window size,
            // otherwise be more accurate
            int binSize = 2048;
            if(audio->getStretchFactor() > 1)
                binSize = 8192;

            spectrumWidget->calculateSpectrum(binSize);
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
    playButton->setToolTip("Play/Pause");

	stopButton = new QPushButton(QIcon(":/data/icons/stop.png"), "", this);
	connect(stopButton, SIGNAL(released()),
	        this, SLOT(stopMusic()));
	stopButton->setFixedSize(QSize(32, 32));
    stopButton->setToolTip("Stop");

    followButton = new QPushButton(QIcon(":/data/icons/follow.png"), "", this);
    connect(followButton, SIGNAL(toggled(bool)),
            waveformWidget, SLOT(toggleFollowMarker(bool)));
    connect(waveformWidget, SIGNAL(areaScrolled(bool)),
            followButton, SLOT(setChecked(bool)));
    followButton->setFixedSize(QSize(32, 32));
    followButton->setCheckable(true);
    followButton->setChecked(true);
    followButton->setToolTip("Follow the marker");

    speedSlider = new QSlider(Qt::Orientation::Horizontal);
    speedSlider->setMinimum(1);
    speedSlider->setMaximum(4);
    speedSlider->setTickPosition(QSlider::TicksAbove);
    speedSlider->setSingleStep(1);
    speedSlider->setTickInterval(1);
    speedSlider->setMinimumSize(100, 50);
    speedSlider->setToolTip("Stretch factor");

    speedLabels = new QLabel*[4];
    speedLabels[0] = new QLabel("1", speedSlider);
    speedLabels[0]->move(5, 10);
    speedLabels[1] = new QLabel("2", speedSlider);
    speedLabels[1]->move(33, 10);
    speedLabels[2] = new QLabel("3", speedSlider);
    speedLabels[2]->move(60, 10);
    speedLabels[3] = new QLabel("4", speedSlider);
    speedLabels[3]->move(85, 10);

    toneSlider = new QSlider(Qt::Orientation::Horizontal);
    toneSlider->setMinimum(-12);
    toneSlider->setMaximum(12);
    toneSlider->setValue(0);
    toneSlider->setTickPosition(QSlider::TicksAbove);
    toneSlider->setSingleStep(1);
    toneSlider->setTickInterval(2);
    toneSlider->setMinimumSize(300, 50);
    toneSlider->setToolTip("Transpose");

    toneLabels = new QLabel*[3];
    toneLabels[0] = new QLabel("-12", toneSlider);
    toneLabels[0]->move(0, 10);
    toneLabels[0] = new QLabel("0", toneSlider);
    toneLabels[0]->move(146, 10);
    toneLabels[0] = new QLabel("12", toneSlider);
    toneLabels[0]->move(282, 10);
}

void MainWidget::createLayout()
{
	mainLayout.setSpacing(1);
	controlsLayout.setSpacing(1);
	keyboardLayout.setSpacing(1);

	controlsLayout.addWidget(playButton);
	controlsLayout.addWidget(stopButton);
    controlsLayout.addWidget(followButton);
    controlsLayout.addWidget(speedSlider);
    controlsLayout.addWidget(toneSlider);
	controlsLayout.addStretch(32);

	keyboardLayout.addWidget(keyboardWidget);

	mainLayout.addLayout(&controlsLayout);
	mainLayout.addWidget(waveformScrollbar);
	mainLayout.addWidget(waveformWidget);
	mainLayout.addWidget(spectrumWidget);
	mainLayout.addLayout(&keyboardLayout);

	setLayout(&mainLayout);
}

void MainWidget::createWidgets()
{
	keyboardWidget = new KeyboardWidget(parentApp, this);
	keyboardWidget->setMinimumSize(100, 100);
	keyboardWidget->setMaximumHeight(100);

	spectrumWidget = new SpectrumWidget(parentApp, keyboardWidget);
	spectrumWidget->setMinimumSize(100, 100);

	waveformScrollbar = new ScrollBar(Qt::Orientation::Horizontal,
	                                  waveformWidget);

	waveformWidget = new WaveformWidget(parentApp, this, waveformScrollbar);
	waveformWidget->setMinimumSize(100, 100);
	waveformWidget->setMaximumHeight(100);
}

void MainWidget::turnOffMusicFollowing()
{
	waveformWidget->toggleFollowMarker(false);
}

void MainWidget::setFollowingMusicMarker(bool set)
{
	followMusicMarker = set;
}

// Creates OpenAL context
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
