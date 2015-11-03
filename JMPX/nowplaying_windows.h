#ifndef NOWPLAYING_H
#define NOWPLAYING_H

//this needs winamp compatible API media players. I've tested mediamonkey

#include <QObject>
#include <QTimer>
#include <QByteArray>
#include <windows.h>

class NowPlaying : public QObject
{
    Q_OBJECT
public:
    explicit NowPlaying(QObject *parent = 0);
    QString rt_title;
signals:
    void songtitlechanged(const QString &title);
public slots:
private slots:
    void songtitlecheck();
private:
    QByteArray rt_title_ba;
};

#endif // NOWPLAYING_H
