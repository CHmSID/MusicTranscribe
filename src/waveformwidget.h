#ifndef WAVEFORMWIDGET_H
#define WAVEFORMWIDGET_H

#include <QApplication>
#include <QWidget>

class WaveformWidget: public QWidget
{
public:
	WaveformWidget(QApplication* parentApp = 0, QWidget* parentWid = 0);
};

#endif // WAVEFORMWIDGET_H
