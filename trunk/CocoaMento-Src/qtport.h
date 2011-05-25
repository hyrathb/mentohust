#ifndef QTPORT_H
#define QTPORT_H

#include <QObject>

class QtPort : public QObject
{
    Q_OBJECT
public:
    explicit QtPort(QObject *parent = 0);
    void emitEvent();
signals:
    void trigger();

};

#endif // QTPORT_H
