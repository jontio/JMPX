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
    void updatesongtitle();
    QDBusInterface *ptr_propiface;
    QDBusServiceWatcher *ptr_watcher;
    QTimer *ptr_timer;
    QString lastmsg;
private slots:
    void mpris2PropertiesChanged(QString interface_name, QVariantMap changed_properties, QStringList invalidated_properties);
    void serviceUnregistered(QString serviceName);
    void checkdbus();
};


#endif // NOWPLAYING_H
