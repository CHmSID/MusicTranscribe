#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include <QScrollBar>
#include <QWidget>

/*
 * An extension of QScrollBar with changes allowing to
 * increment and decrement the value of the scrollbar.
 */
class ScrollBar : public QScrollBar
{
	Q_OBJECT

public:
	ScrollBar(Qt::Orientation s, QWidget* parent = nullptr);

public slots:
	void addValue(int x);
	void subValue(int x);
};

#endif // SCROLLBAR_H
