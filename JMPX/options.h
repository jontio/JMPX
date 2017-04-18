#ifndef OPTIONS_H
#define OPTIONS_H

#include <QDialog>

#include "../libJMPX/JMPXInterface.h"
#include "fileloader.h"

//#define DEV_TAB

#ifndef SYSTEM_BITRATE
#define SYSTEM_BITRATE 192000
#endif

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
signals:
    void Show_SCA_Volume_Meter_signal(bool show);
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

    void on_horizontalSlider_scalevel_valueChanged(int value);

    void on_comboBox_dsca_mode_currentIndexChanged(int index);

    void on_horizontalSlider_noise_valueChanged(int value);

private:
    Ui::Options *ui;
    void updateDSCADescription(int mode);
};

#endif // OPTIONS_H
