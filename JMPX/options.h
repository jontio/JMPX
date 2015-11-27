#ifndef OPTIONS_H
#define OPTIONS_H

#include <QDialog>

#include "../libJMPX/JMPXInterface.h"

namespace Ui {
class Options;
}

class Options : public QDialog
{
    Q_OBJECT

public:
    explicit Options(QWidget *parent = 0);
    ~Options();
    void populatesettings(JMPXInterface *pJMPX);
    void pushsetting(JMPXInterface *pJMPX);
    void savesettings(JMPXInterface *pJMPX);
    void loadsettings(JMPXInterface *pJMPX);
    bool update_rt_music_title;
    bool quit_on_error;
private slots:
    void on_checkBox_rbds_clicked(bool checked);
    void on_horizontalSlider_monolevel_valueChanged(int value);
    void on_horizontalSlider_38klevel_valueChanged(int value);
    void on_horizontalSlider_pilotlevel_valueChanged(int value);
    void on_horizontalSlider_rdslevel_valueChanged(int value);
private:
    Ui::Options *ui;
};

#endif // OPTIONS_H
