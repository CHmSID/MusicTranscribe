#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include <QScrollBar>
#include <QWidget>

class ScrollBar : public QScrollBar
{
	Q_OBJECT

public:
	ScrollBar(Qt::Orientation s, QWidget* parent = nullptr);

public slots:
	void addValue(int x);
};

#endif // SCROLLBAR_H
