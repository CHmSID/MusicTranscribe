#include "scrollbar.h"

// See the header for explanation
ScrollBar::ScrollBar(Qt::Orientation s, QWidget* parent)
    : QScrollBar(s, parent)
{

}

void ScrollBar::addValue(int x)
{
	setValue(value() + x);
}

void ScrollBar::subValue(int x)
{
	setValue(value() - x);
}
