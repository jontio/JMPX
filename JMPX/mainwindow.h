#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QTimer>
#include <QLibrary>
#include <QPointer>

#include "../libJMPX/JMPXInterface.h"

#include "options.h"

#ifdef Q_OS_WIN
#include "nowplaying_windows.h"
#endif
#ifdef Q_OS_LINUX
#include "nowplaying_linux.h"
#endif
#ifdef Q_OS_MAC
#include "nowplaying_mac.h"
#endif


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected:
    void changeEvent(QEvent *e);
private:
    Ui::MainWindow *ui;
    QPointer<JMPXInterface> pJMPX;
    QTimer *ptimer;
    Options *options;
    void updatelowrateinfo();
    NowPlaying  *nowplaying;
private slots:
    void volbarssetfixedequalwidth();
    void updatedisplay();
    void on_action_Options_triggered();
    void on_checkBox_modulate_stateChanged(int state);
    void on_action_About_triggered();
    void on_actionAbout_Qt_triggered();
    void songtitlecheck(const QString &title);
};

#endif // MAINWINDOW_H
