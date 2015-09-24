#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QTimer>
#include <QLibrary>
#include <QPointer>

#include "../libJMPX/JMPXInterface.h"

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

private slots:
    void on_comboBox_currentIndexChanged(int index);
    void on_radionone_clicked();
    void on_radio75_clicked();
    void on_radio50_clicked();
    void on_checkBox_2_stateChanged(int );
    void on_checkBox_stateChanged(int );
    void updatedisplay();
};

#endif // MAINWINDOW_H
