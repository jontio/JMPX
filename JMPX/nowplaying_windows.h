#ifndef NOWPLAYING_H
#define NOWPLAYING_H

//this needs winamp compatible API media players. I've tested mediamonkey

//added a hack for VLC

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
    QByteArray rt_title_ba;
    bool TitleSetByWindowEnum;
signals:
    void songtitlechanged(const QString &title);
public slots:
private slots:
    void songtitlecheck();
};

#endif // NOWPLAYING_H
