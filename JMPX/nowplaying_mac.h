#ifndef NOWPLAYING_H
#define NOWPLAYING_H

//This is currently a dummy class as I don't know how to impliment a now playing class for Mac OS

#include <QObject>

class NowPlaying : public QObject
{
    Q_OBJECT
public:
    explicit NowPlaying(QObject *parent = 0);
    QString rt_title;
signals:
    void songtitlechanged(const QString &title);
public slots:
};

#endif // NOWPLAYING_H
