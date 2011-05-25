#include "qtport.h"

QtPort::QtPort(QObject *parent) :
    QObject(parent)
{
}
void QtPort::emitEvent()
{
    emit trigger();
}



