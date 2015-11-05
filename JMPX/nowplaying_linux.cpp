#include "nowplaying_linux.h"
#include <QDebug>

NowPlaying::NowPlaying(QObject *parent) : QObject(parent)
{
    ptr_propiface=NULL;

    ptr_watcher = new QDBusServiceWatcher(this);
    ptr_watcher->setWatchMode(QDBusServiceWatcher::WatchForOwnerChange);
    ptr_watcher->setConnection(QDBusConnection::sessionBus());
    connect(ptr_watcher,SIGNAL(serviceUnregistered(QString)),this,SLOT(serviceUnregistered(QString)));

    ptr_timer=new QTimer(this);
    ptr_timer->setInterval(1000);
    connect(ptr_timer,SIGNAL(timeout()),this,SLOT(checkdbus()));

    rt_title.clear();

    ptr_timer->start();
    checkdbus();

}

bool NowPlaying::mpris2connect()
{
    mpris2disconnect();

    //get a media player service name
    QDBusReply<QStringList> reply = QDBusConnection::sessionBus().interface()->registeredServiceNames();
    if (!reply.isValid())
    {
        QString msg="No reply from dbus";
        if(msg!=lastmsg){qDebug()<<msg;lastmsg=msg;}
        return false;
    }
    QStringList list=reply.value().filter("org.mpris.MediaPlayer2");
    if (list.isEmpty())
    {
        QString msg="No media player found";
        if(msg!=lastmsg){qDebug()<<msg;lastmsg=msg;}
        return false;
    }
    QString service=list[0];

    ptr_propiface=new QDBusInterface(service, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", QDBusConnection::sessionBus(),this);//service path interface
    QDBusConnection::sessionBus().connect(service, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties" ,"PropertiesChanged", this, SLOT(mpris2PropertiesChanged(QString,QVariantMap,QStringList)));
    ptr_watcher->addWatchedService(service);

    ptr_timer->stop();

    QString msg="Connected to "+service;
    if(msg!=lastmsg){qDebug()<<msg;lastmsg=msg;}

    updatesongtitle();

    return true;
}

void NowPlaying::mpris2disconnect()
{
    if(!rt_title.isEmpty())
    {
        rt_title.clear();
        emit songtitlechanged(rt_title);
    }

    foreach(QString service,ptr_watcher->watchedServices())
    {
        QString msg="Disconnected from "+service;
        if(msg!=lastmsg){qDebug()<<msg;lastmsg=msg;}

        QDBusConnection::sessionBus().disconnect(service, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties" ,"PropertiesChanged", this, SLOT(mpris2PropertiesChanged(QString,QVariantMap,QStringList)));
        ptr_watcher->removeWatchedService(service);
    }
    if(ptr_propiface){ptr_propiface->deleteLater();ptr_propiface=NULL;}

}

void NowPlaying::mpris2PropertiesChanged(QString interface_name, QVariantMap changed_properties, QStringList invalidated_properties)
{
    Q_UNUSED(invalidated_properties);

    //qDebug()<<changed_properties;

    if(changed_properties.value("Metadata").isNull()&&changed_properties.value("PlaybackStatus").isNull())return;
    if(interface_name!="org.mpris.MediaPlayer2.Player")return;

    updatesongtitle();
}

void NowPlaying::updatesongtitle()
{
    if(!ptr_propiface)return;
    QDBusReply<QVariant> propreply = ptr_propiface->call("Get", "org.mpris.MediaPlayer2.Player", "PlaybackStatus");
    if (!propreply.isValid()){qDebug("Call failed: %s\n", qPrintable(propreply.error().message()));return;}
    bool playing=false;
    if(propreply.value().toString()=="Playing")playing=true;
    propreply = ptr_propiface->call("Get", "org.mpris.MediaPlayer2.Player", "Metadata");
    if (!propreply.isValid()){qDebug("Call failed: %s\n", qPrintable(propreply.error().message()));return;}
    QVariantMap vm=qdbus_cast<QVariantMap>(propreply.value());
    QString artist=vm.value("xesam:artist").toString();
    QString title=vm.value("xesam:title").toString();
    QString vlcnowplaying=vm.value("vlc:nowplaying").toString();
    QString str;
    if((!artist.isNull())&&(!title.isNull()))str=artist+" - "+title;
    else if(!title.isNull())str=title;
    if((!vlcnowplaying.isNull())&&(!vlcnowplaying.isEmpty()))str=vlcnowplaying;
    if(!playing)str.clear();

    if(rt_title!=str)
    {
        rt_title=str;
        emit songtitlechanged(rt_title);
    }
    else rt_title=str;
}

void NowPlaying::serviceUnregistered(QString serviceName)
{
    Q_UNUSED(serviceName);
    mpris2disconnect();
    ptr_timer->start();
}

void NowPlaying::checkdbus()
{
    mpris2connect();
}


