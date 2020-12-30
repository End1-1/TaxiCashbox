#include "lineedit.h"

LineEdit::LineEdit(QWidget *parent) :
    QLineEdit(parent)
{

}

void LineEdit::focusInEvent(QFocusEvent *e)
{
    Q_UNUSED(e);
    emit focusIn();
}

void LineEdit::focusOutEvent(QFocusEvent *e)
{
    Q_UNUSED(e);
    emit focusOut();
}
