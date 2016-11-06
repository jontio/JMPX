#ifndef OPTIONS_H
#define OPTIONS_H

#include <QDialog>

#include "../libJMPX/JMPXInterface.h"
#include "fileloader.h"

namespace Ui {
class Options;
}

class Options : public QDialog
{
    Q_OBJECT

public:
    explicit Options(QWidget *parent = 0);
    ~Options();
    void populatesettings(JMPXInterface *pJMPX,FileLoader *fileloader);
    void pushsetting(JMPXInterface *pJMPX,FileLoader *fileloader);
    void savesettings(JMPXInterface *pJMPX,FileLoader *fileloader);
    void loadsettings(JMPXInterface *pJMPX,FileLoader *fileloader);
    bool update_rt_music_title;
    bool quit_on_error;
private slots:
    void on_checkBox_rbds_clicked(bool checked);
    void on_horizontalSlider_monolevel_valueChanged(int value);
    void on_horizontalSlider_38klevel_valueChanged(int value);
    void on_horizontalSlider_pilotlevel_valueChanged(int value);
    void on_horizontalSlider_rdslevel_valueChanged(int value);
    void on_toolButton_5a_filename_clicked();

    void on_checkBox_rt_enable_clicked();

    void on_groupBox_5a_clicked();

    void validatepercentagespinboxes();

    void on_spinBox_2A_percent_valueChanged(int arg1);

    void on_spinBox_5A_percent_valueChanged(int arg1);

private:
    Ui::Options *ui;
};

#endif // OPTIONS_H