#ifndef NOWPLAYING_H
#define NOWPLAYING_H

//this can talk to mpris2 compatible media players. I've tested audacious and glome player
//it uses the first mpris2 player it finds

#include <QObject>
#include <QtDBus/QtDBus>
#include <QDBusServiceWatcher>

class NowPlaying : public QObject
{
    Q_OBJECT
public:
    explicit NowPlaying(QObject *parent = 0);
    QString rt_title;
signals:
    void songtitlechanged(const QString &title);
public slots:
private:
    bool mpris2connect();
    void mpris2disconnect(); 
    QDBusInterface *ptr_propiface;
    QDBusServiceWatcher *ptr_watcher;
    QTimer *ptr_timer;
    QString lastmsg;
    QTimer *watcher_timer_hack;//with vlc when listening to radio "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties" ,"PropertiesChanged" never emits when vlc:nowplaying changes only xesam:title. Why not???. this is a heck to make sure we get these changes
private slots:
    void mpris2PropertiesChanged(QString interface_name, QVariantMap changed_properties, QStringList invalidated_properties);
    void serviceUnregistered(QString serviceName);
    void checkdbus();
    void updatesongtitle();
};


#endif // NOWPLAYING_H
