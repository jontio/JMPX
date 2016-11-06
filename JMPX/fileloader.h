#ifndef FILELOADER_H
#define FILELOADER_H
#include <QObject>
#include <QFileSystemWatcher>
#include <QFile>

class FileLoader : public QObject
{
    Q_OBJECT
public:
    explicit FileLoader(QObject *parent = 0);

    //just set the file name and this class will take care of the rest
    void set_filename(QString filename, bool ashextext);

    //if you want to force a reload but i cant see why you would want to
    bool reloadfile(){return load_file(filename,ashextext,true);}

    //was there an error the last time the file was loaded?
    bool hasloaderror(){return loaderror;}

    //what is the file that is being monitored?
    QString get_filename(){return filename;}

    //is it in hex text?
    bool get_ashextext(){return ashextext;}

signals:

    //emits when there is new data or the file has been reloaded
    void dataLoadSignal(const QByteArray &data);

private:
    bool ashextext;//
    bool loaderror;//
    QString filename;//
    QByteArray ba;//
    QFileSystemWatcher *filewatcher;

    bool load_file(QString _filename,bool _ashextext){return load_file(_filename,_ashextext,false);}
    bool load_file(QString _filename,bool _ashextext,bool invalidate);

private slots:
    void fileChanged(const QString &path);
    void fileLoadErrorTimeout();
};

#endif // FILELOADER_H
