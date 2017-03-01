#include "fileloader.h"
#include <QDebug>
#include <QTimer>
#include <QStringList>

FileLoader::FileLoader(QObject *parent) : QObject(parent)
{
    filewatcher = new QFileSystemWatcher(this);
    connect(filewatcher, SIGNAL(fileChanged(const QString &)), this, SLOT(fileChanged(const QString &)));
    ashextext=false;
    loaderror=true;
}

void FileLoader::fileLoadErrorTimeout()
{
    if(!hasloaderror())return;
    qDebug()<<"fileLoadErrorTimeout";
    if(!reloadfile())QTimer::singleShot(5000, this, SLOT(fileLoadErrorTimeout()));
     else if((filewatcher->files().isEmpty())&&(!filename.isEmpty()))
     {
        qDebug()<<"fileCreated:"<<filename;
        filewatcher->addPath(filename);
     }
}

void FileLoader::set_filename(QString filename, bool ashextext)
{
    //if we are already watching the file then just leave the watcher alone
    if(filewatcher->files().contains(filename))
    {
        //if we failed to load the file then try again later
        if(!load_file(filename,ashextext))QTimer::singleShot(5000, this, SLOT(fileLoadErrorTimeout()));
        return;
    }
    //remove the all files from the watcher, set the filename, load the file, add the file to the watcher
    if(!filewatcher->files().isEmpty())filewatcher->removePaths(filewatcher->files());
    //if we failed to load the file then try again later
    if(!load_file(filename,ashextext))QTimer::singleShot(5000, this, SLOT(fileLoadErrorTimeout()));
    if(!filename.isEmpty())filewatcher->addPath(get_filename());
}

void FileLoader::fileChanged(const QString &path)
{
    if(!QFile(filename).exists())//if removed
    {
        qDebug()<<"fileRemoved:"<<path;
    } else qDebug()<<"fileChanged:"<<path;
    if(!reloadfile())QTimer::singleShot(5000, this, SLOT(fileLoadErrorTimeout()));
}

bool FileLoader::load_file(QString _filename,bool _ashextext,bool invalidate)
{
    loaderror=true;
    if(_filename.isEmpty())invalidate=true;//invalidate if empty
    if(invalidate)
    {
        filename.clear();
        ashextext=!ashextext;
    }
    if((filename==_filename)&&(ashextext==_ashextext))
    {
        loaderror=false;
        emit dataLoadSignal(ba);
        return true;
    }
    filename=_filename;
    ashextext=_ashextext;
    ba.clear();
    if(filename.isEmpty())//lets say nothing is ok
    {
        loaderror=false;
        emit dataLoadSignal(ba);
        return true;
    }
    QFile file(filename);
    if(ashextext)
    {
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug()<<"failed opening file for reading";
            emit dataLoadSignal(ba);
            return false;
        }
        ba=QByteArray::fromHex(file.readAll());
    }
     else
     {
        if(!file.open(QIODevice::ReadOnly))
        {
            qDebug()<<"failed opening file for reading";
            emit dataLoadSignal(ba);
            return false;
        }
        ba=file.readAll();
     }
    loaderror=false;
    emit dataLoadSignal(ba);
    return true;
}

