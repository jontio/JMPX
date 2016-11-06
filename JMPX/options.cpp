#include "options.h"
#include "ui_options.h"
#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>

Options::Options(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Options)
{
    ui->setupUi(this);

    update_rt_music_title=true;
    quit_on_error=false;

    QStringList list;
    list<<"None";
    for(double d=87.6;d<107.95;d+=0.1)list<<(((QString)"%1Mhz").arg(d,-1,'f',1,'0'));
    ui->comboBox_af_1->addItems(list);
    ui->comboBox_af_2->addItems(list);
}

Options::~Options()
{
    delete ui;
}

void Options::savesettings(JMPXInterface *pJMPX,FileLoader *fileloader)
{
    QSettings settings("JontiSoft", "JMPX");

//first tab

    //soundcard
    settings.setValue("soundcard_in",pJMPX->GetSoundCardInName());
    settings.setValue("soundcard_out",pJMPX->GetSoundCardOutName());

    //preemp
    settings.setValue("PreEmphasis",(int)pJMPX->GetPreEmphasis());

    //stereo
    settings.setValue("EnableStereo",pJMPX->GetEnableStereo());

    //set rds
    settings.setValue("EnableRDS",pJMPX->GetEnableRDS());

    //set CompositeClipper
    settings.setValue("EnableCompositeClipper",pJMPX->GetEnableCompositeClipper());

    //set mono level
    settings.setValue("MonoLevel",pJMPX->GetMonoLevel());

    //set 38k level
    settings.setValue("38kLevel",pJMPX->Get38kLevel());

    //set pilot level
    settings.setValue("PilotLevel",pJMPX->GetPilotLevel());

    //set rds level
    settings.setValue("RDSLevel",pJMPX->GetRDSLevel());

//second tab

    //rbds
    settings.setValue("RDS_RBDS",pJMPX->RDS_Get_RBDS());

    //ps
    settings.setValue("RDS_PS",pJMPX->RDS_GetPS());

    //pi
    settings.setValue("RDS_PI",pJMPX->RDS_GetPI());

    //rt default
    settings.setValue("RDS_RT_default",pJMPX->RDS_GetDefaultRT());

    //pty
    settings.setValue("RDS_PTY",pJMPX->RDS_GetPTY());

    //tp
    settings.setValue("RDS_TP",pJMPX->RDS_Get_TP());

    //ta
    settings.setValue("RDS_TA",pJMPX->RDS_Get_TA());

    //ms
    settings.setValue("RDS_MS",pJMPX->RDS_Get_MS());

    //rt enable
    settings.setValue("RDS_RT_Enable",pJMPX->RDS_Get_RT_Enable());

    //ct
    settings.setValue("RDS_CT",pJMPX->RDS_Get_CT());

    //di stereo
    settings.setValue("RDS_DI_Stereo",pJMPX->RDS_Get_DI_Stereo());

    //di compressed
    settings.setValue("RDS_DI_Compressed",pJMPX->RDS_Get_DI_Compressed());

    //di ai head
    settings.setValue("RDS_DI_Artificial_Head",pJMPX->RDS_Get_DI_Artificial_Head());

    //di dynamic pty
    settings.setValue("RDS_DI_Dynamic_PTY",pJMPX->RDS_Get_DI_Dynamic_PTY());

    //clocktimeoffset
    settings.setValue("RDS_clocktimeoffset",pJMPX->RDS_Get_clocktimeoffset());

    //af 1
    settings.setValue("RDS_altfreq1",pJMPX->RDS_Get_altfreq1());

    //af 2
    settings.setValue("RDS_altfreq2",pJMPX->RDS_Get_altfreq2());

    //update_rt_music_title
    settings.setValue("update_rt_music_title",update_rt_music_title);

//third tab

    //5a enable
    settings.setValue("RDS_5A_Enable",pJMPX->RDS_Get_5A_Enable());

    //5a filename ashextext
    settings.setValue("5A_fileloader_ashextext",fileloader->get_ashextext());

    //5a filename
    settings.setValue("5A_fileloader_filename",fileloader->get_filename());

    //group percentages
    settings.setValue("RDS_grp0Awantedbandwidthusage",pJMPX->RDS_Get_grp0Awantedbandwidthusage());
    settings.setValue("RDS_grp2Awantedbandwidthusage",pJMPX->RDS_Get_grp2Awantedbandwidthusage());
    settings.setValue("RDS_grp5Awantedbandwidthusage",pJMPX->RDS_Get_grp5Awantedbandwidthusage());

}

void Options::loadsettings(JMPXInterface *pJMPX,FileLoader *fileloader)
{
    QSettings settings("JontiSoft", "JMPX");

    //soundcard
    pJMPX->SetSoundCardInName(settings.value("soundcard_in","Default").toString());
    pJMPX->SetSoundCardOutName(settings.value("soundcard_out","Default").toString());

    //preemp
    pJMPX->SetPreEmphasis((TimeConstant)settings.value("PreEmphasis",(int)WORLD).toInt());

    //stereo
    pJMPX->SetEnableStereo(settings.value("EnableStereo",true).toBool());

    //set rds
    pJMPX->SetEnableRDS(settings.value("EnableRDS",true).toBool());

    //set CompositeClipper
    pJMPX->SetEnableCompositeClipper(settings.value("EnableCompositeClipper",false).toBool());

    //set mono level
    pJMPX->SetMonoLevel(settings.value("MonoLevel",0.9).toDouble());

    //set 38k level
    pJMPX->Set38kLevel(settings.value("38kLevel",1.0).toDouble());

    //set pilot level
    pJMPX->SetPilotLevel(settings.value("PilotLevel",0.07).toDouble());

    //set rds level
    pJMPX->SetRDSLevel(settings.value("RDSLevel",0.06).toDouble());

//second tab

    //rbds
    pJMPX->RDS_Set_RBDS(settings.value("RDS_RBDS",false).toBool());

    //ps
    pJMPX->RDS_SetPS(settings.value("RDS_PS","JMPX RDS").toString());

    //pi
    pJMPX->RDS_SetPI(settings.value("RDS_PI",0x9000).toInt());

    //rt default
    pJMPX->RDS_SetDefaultRT(settings.value("RDS_RT_default","RT test text").toString());

    //pty
    pJMPX->RDS_SetPTY(settings.value("RDS_PTY",0).toInt());

    //tp
    pJMPX->RDS_Set_TP(settings.value("RDS_TP",false).toBool());

    //ta
    pJMPX->RDS_Set_TA(settings.value("RDS_TA",false).toBool());

    //ms
    pJMPX->RDS_Set_MS(settings.value("RDS_MS",true).toBool());

    //rt enable
    pJMPX->RDS_Set_RT_Enable(settings.value("RDS_RT_Enable",true).toBool());

    //ct
    pJMPX->RDS_Set_CT(settings.value("RDS_CT",true).toBool());

    //di stereo
    pJMPX->RDS_Set_DI_Stereo(settings.value("RDS_DI_Stereo",true).toBool());

    //di compressed
    pJMPX->RDS_Set_DI_Compressed(settings.value("RDS_DI_Compressed",false).toBool());

    //di ai head
    pJMPX->RDS_Set_DI_Artificial_Head(settings.value("RDS_DI_Artificial_Head",false).toBool());

    //di dynamic pty
    pJMPX->RDS_Set_DI_Dynamic_PTY(settings.value("RDS_DI_Dynamic_PTY",false).toBool());

    //clocktimeoffset
    pJMPX->RDS_Set_clocktimeoffset(settings.value("RDS_clocktimeoffset",0).toInt());

    //af 1
    pJMPX->RDS_Set_altfreq1(settings.value("RDS_altfreq1",205).toDouble());

    //af 2
    pJMPX->RDS_Set_altfreq2(settings.value("RDS_altfreq2",205).toDouble());

    //update_rt_music_title
    update_rt_music_title=(settings.value("update_rt_music_title",true).toBool());

//third tab

    //5a enable
    pJMPX->RDS_Set_5A_Enable(settings.value("RDS_5A_Enable",false).toBool());

    //5a filename and ashextext
    fileloader->set_filename(settings.value("5A_fileloader_filename","").toString(),settings.value("5A_fileloader_ashextext",true).toBool());

    //group percentages
    pJMPX->RDS_Set_grouppercentages(settings.value("RDS_grp0Awantedbandwidthusage",0.8).toDouble(),settings.value("RDS_grp2Awantedbandwidthusage",0.2).toDouble(),settings.value("RDS_grp5Awantedbandwidthusage",0.0).toDouble());


}

void Options::populatesettings(JMPXInterface *pJMPX, FileLoader *fileloader)
{

//first tab

    //get soundcards
    SDevices* pdev=pJMPX->GetDevices();
    ui->comboBox_soundcard_in->clear();
    ui->comboBox_soundcard_out->clear();
    for(unsigned int i=0;i<pdev->NumberOfDevices;i++)
    {
        if(pdev->Device[i].inchannelcount>0)ui->comboBox_soundcard_in->addItem(pdev->Device[i].name);
        if(pdev->Device[i].outchannelcount>0)ui->comboBox_soundcard_out->addItem(pdev->Device[i].name);
    }
    if(!pdev->NumberOfDevices)ui->comboBox_soundcard_in->addItem("None");
    if(!pdev->NumberOfDevices)ui->comboBox_soundcard_out->addItem("None");

    //thanks doqtor for this solution to text overflow of qcomboboxes
    //determinge the maximum width required to display all names in full
    {
    int max_width = 0;
    QFontMetrics fm(ui->comboBox_soundcard_in->font());
    for(int x = 0; x < ui->comboBox_soundcard_in->count(); ++x)
    {
        int width = fm.width(ui->comboBox_soundcard_in->itemText(x));
        if(width > max_width)
            max_width = width;
    }
    if(ui->comboBox_soundcard_in->view()->minimumWidth() < max_width)
    {
        // add scrollbar width and margin
        max_width += ui->comboBox_soundcard_in->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
        max_width += ui->comboBox_soundcard_in->view()->autoScrollMargin();
        // set the minimum width of the combobox drop down list
        ui->comboBox_soundcard_in->view()->setMinimumWidth(max_width);
    }
    }
    {
    int max_width = 0;
    QFontMetrics fm(ui->comboBox_soundcard_out->font());
    for(int x = 0; x < ui->comboBox_soundcard_out->count(); ++x)
    {
        int width = fm.width(ui->comboBox_soundcard_out->itemText(x));
        if(width > max_width)
            max_width = width;
    }
    if(ui->comboBox_soundcard_out->view()->minimumWidth() < max_width)
    {
        // add scrollbar width and margin
        max_width += ui->comboBox_soundcard_out->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
        max_width += ui->comboBox_soundcard_out->view()->autoScrollMargin();
        // set the minimum width of the combobox drop down list
        ui->comboBox_soundcard_out->view()->setMinimumWidth(max_width);
    }
    }

    //soundcard   
    ui->comboBox_soundcard_in->setCurrentIndex(0);
    {int index=ui->comboBox_soundcard_in->findText(pJMPX->GetSoundCardInName());ui->comboBox_soundcard_in->setCurrentIndex(index);}
    ui->comboBox_soundcard_out->setCurrentIndex(0);
    {int index=ui->comboBox_soundcard_out->findText(pJMPX->GetSoundCardOutName());ui->comboBox_soundcard_out->setCurrentIndex(index);}

    //preemp
    switch(pJMPX->GetPreEmphasis())
    {
    case WORLD:
        ui->preemp_radio50->setChecked(true);
        break;
    case USA:
        ui->preemp_radio75->setChecked(true);
        break;
    default:
        ui->preemp_radionone->setChecked(true);
    }

    //set stereo
    ui->checkBox_stereo->setChecked(pJMPX->GetEnableStereo());

    //set rds
    ui->checkBox_rds->setChecked(pJMPX->GetEnableRDS());

    //set CompositeClipper
    ui->checkBox_compositeclipper->setChecked(pJMPX->GetEnableCompositeClipper());

    //set monolevel
    ui->horizontalSlider_monolevel->setValue(pJMPX->GetMonoLevel()*100.0);

    //set 38level
    ui->horizontalSlider_38klevel->setValue(pJMPX->Get38kLevel()*100.0);

    //set pilotlevel
    ui->horizontalSlider_pilotlevel->setValue(qRound(pJMPX->GetPilotLevel()*1000.0));

    //set rdslevel
    ui->horizontalSlider_rdslevel->setValue(qRound(pJMPX->GetRDSLevel()*1000.0));

//second tab

    //rbds
    ui->checkBox_rbds->setChecked(pJMPX->RDS_Get_RBDS());

    //ps
    ui->lineEdit_ps->setText(pJMPX->RDS_GetPS());

    //pi
    ui->lineEdit_pi->setText(QString::number(pJMPX->RDS_GetPI(),16));

    //rt default
    ui->lineEdit_rt_default->setText(pJMPX->RDS_GetDefaultRT());

    //pty
    on_checkBox_rbds_clicked(ui->checkBox_rbds->isChecked());
    ui->comboBox_pty->setCurrentIndex(pJMPX->RDS_GetPTY());

    //tp
    ui->checkBox_tp->setChecked(pJMPX->RDS_Get_TP());

    //ta
    ui->checkBox_ta->setChecked(pJMPX->RDS_Get_TA());

    //ms
    ui->checkBox_ms->setChecked(pJMPX->RDS_Get_MS());

    //rt enable
    ui->checkBox_rt_enable->setChecked(pJMPX->RDS_Get_RT_Enable());

    //ct
    ui->checkBox_ct->setChecked(pJMPX->RDS_Get_CT());

    //di stereo
    ui->checkBox_di_stereo->setChecked(pJMPX->RDS_Get_DI_Stereo());

    //di compressed
    ui->checkBox_di_compressed->setChecked(pJMPX->RDS_Get_DI_Compressed());

    //di ai head
    ui->checkBox_di_artificial_head->setChecked(pJMPX->RDS_Get_DI_Artificial_Head());

    //di dynamic pty
    ui->checkBox_di_dynamic_pty->setChecked(pJMPX->RDS_Get_DI_Dynamic_PTY());

    //clocktimeoffset
    ui->spinBox_clocktimeoffset->setValue(pJMPX->RDS_Get_clocktimeoffset());

    //af 1
    if(pJMPX->RDS_Get_altfreq1()>108.0)ui->comboBox_af_1->setCurrentIndex(0);
     else {int index=ui->comboBox_af_1->findText(((QString)"%1Mhz").arg(pJMPX->RDS_Get_altfreq1(),-1,'f',1,'0'));if(index<0)index=0;ui->comboBox_af_1->setCurrentIndex(index);} // not all Qt version have ui->comboBox_af_1->setCurrentText( (((QString)"%1Mhz").arg(pJMPX->RDS_Get_altfreq1(),-1,'f',1,'0')) );

    //af 2
    if(pJMPX->RDS_Get_altfreq2()>108.0)ui->comboBox_af_2->setCurrentIndex(0);
     else {int index=ui->comboBox_af_2->findText(((QString)"%1Mhz").arg(pJMPX->RDS_Get_altfreq2(),-1,'f',1,'0'));if(index<0)index=0;ui->comboBox_af_2->setCurrentIndex(index);} // not all Qt version have ui->comboBox_af_2->setCurrentText( (((QString)"%1Mhz").arg(pJMPX->RDS_Get_altfreq2(),-1,'f',1,'0')) );

    //update_rt_music_title
    ui->checkBox_update_rt_music_title->setChecked(update_rt_music_title);

//third tab

    //5a enable
    ui->groupBox_5a->setChecked(pJMPX->RDS_Get_5A_Enable());

    //5a filename ashextext
    ui->checkBox_5a_file_is_hextext->setChecked(fileloader->get_ashextext());

    //5a filename
    ui->lineEdit_5a_filename->setText(fileloader->get_filename());

    //group percentages
    ui->spinBox_0A_percent->setValue(pJMPX->RDS_Get_grp0Awantedbandwidthusage()*100+0.5);
    ui->spinBox_2A_percent->setValue(pJMPX->RDS_Get_grp2Awantedbandwidthusage()*100+0.5);
    ui->spinBox_5A_percent->setValue(pJMPX->RDS_Get_grp5Awantedbandwidthusage()*100+0.5);

}

void Options::pushsetting(JMPXInterface *pJMPX,FileLoader *fileloader)
{

//first tab

    if((pJMPX->GetSoundCardInName()!=ui->comboBox_soundcard_in->currentText())||(pJMPX->GetSoundCardOutName()!=ui->comboBox_soundcard_out->currentText()))
    {
        qDebug()<<"sound card changed";

        bool orgstate=pJMPX->IsActive();

        pJMPX->Active(false);

        pJMPX->SetSampleRate(192000);
        pJMPX->SetSoundCardInName(ui->comboBox_soundcard_in->currentText());
        pJMPX->SetSoundCardOutName(ui->comboBox_soundcard_out->currentText());
        pJMPX->SetBufferFrames(8096);//adjust this for latency or for lost frames

        pJMPX->Active(orgstate);
        if((orgstate)&&(pJMPX->GotError()))
        {
            QMessageBox msgBox;
            if(quit_on_error){qDebug()<<pJMPX->GetLastRTAudioError();exit(1);}
            msgBox.setText(pJMPX->GetLastRTAudioError());
            msgBox.exec();
            pJMPX->Active(false);
        }

    }

    //preemp
    if(ui->preemp_radionone->isChecked())pJMPX->SetPreEmphasis(NONE);
     else if(ui->preemp_radio50->isChecked())pJMPX->SetPreEmphasis(WORLD);
      else if(ui->preemp_radio75->isChecked())pJMPX->SetPreEmphasis(USA);

    //set stereo
    pJMPX->SetEnableStereo(ui->checkBox_stereo->isChecked());

    //set rds
    pJMPX->SetEnableRDS(ui->checkBox_rds->isChecked());

    //set CompositeClipper
    pJMPX->SetEnableCompositeClipper(ui->checkBox_compositeclipper->isChecked());

    //set monolevel
    pJMPX->SetMonoLevel(((double)ui->horizontalSlider_monolevel->value())/100.0);

    //set 38level
    pJMPX->Set38kLevel(((double)ui->horizontalSlider_38klevel->value())/100.0);

    //set pilotlevel
    pJMPX->SetPilotLevel(((double)ui->horizontalSlider_pilotlevel->value())/1000.0);

    //set rdslevel
    pJMPX->SetRDSLevel(((double)ui->horizontalSlider_rdslevel->value())/1000.0);

//second tab

    //rbds
    pJMPX->RDS_Set_RBDS(ui->checkBox_rbds->isChecked());

    //ps
    pJMPX->RDS_SetPS(ui->lineEdit_ps->text());

    //pi
    pJMPX->RDS_SetPI(ui->lineEdit_pi->text().toInt(0,16));

    //rt default
    pJMPX->RDS_SetDefaultRT(ui->lineEdit_rt_default->text());

    //pty
    pJMPX->RDS_SetPTY(ui->comboBox_pty->currentIndex());

    //tp
    pJMPX->RDS_Set_TP(ui->checkBox_tp->isChecked());

    //ta
    pJMPX->RDS_Set_TA(ui->checkBox_ta->isChecked());

    //ms
    pJMPX->RDS_Set_MS(ui->checkBox_ms->isChecked());

    //rt enable
    pJMPX->RDS_Set_RT_Enable(ui->checkBox_rt_enable->isChecked());

    //ct
    pJMPX->RDS_Set_CT(ui->checkBox_ct->isChecked());

    //di stereo
    pJMPX->RDS_Set_DI_Stereo(ui->checkBox_di_stereo->isChecked());

    //di compressed
    pJMPX->RDS_Set_DI_Compressed(ui->checkBox_di_compressed->isChecked());

    //di ai head
    pJMPX->RDS_Set_DI_Artificial_Head(ui->checkBox_di_artificial_head->isChecked());

    //di dynamic pty
    pJMPX->RDS_Set_DI_Dynamic_PTY(ui->checkBox_di_dynamic_pty->isChecked());

    //clocktimeoffset
    pJMPX->RDS_Set_clocktimeoffset(ui->spinBox_clocktimeoffset->value());

    //af 1
    double val=ui->comboBox_af_1->currentText().left(ui->comboBox_af_1->currentText().size()-3).toDouble();;
    if(val==0)pJMPX->RDS_Set_altfreq1(205);//set filler
     else pJMPX->RDS_Set_altfreq1(val);

    //af 2
    val=ui->comboBox_af_2->currentText().left(ui->comboBox_af_2->currentText().size()-3).toDouble();;
    if(val==0)pJMPX->RDS_Set_altfreq2(205);//set filler
     else pJMPX->RDS_Set_altfreq2(val);

    //update_rt_music_title
    update_rt_music_title=ui->checkBox_update_rt_music_title->isChecked();

//third tab

    //5a enable
    pJMPX->RDS_Set_5A_Enable(ui->groupBox_5a->isChecked());

    //5a filename and ashextext
    fileloader->set_filename(ui->lineEdit_5a_filename->text(),ui->checkBox_5a_file_is_hextext->isChecked());

    //group percentages
    pJMPX->RDS_Set_grouppercentages(ui->spinBox_0A_percent->value(),ui->spinBox_2A_percent->value(),ui->spinBox_5A_percent->value());

}

void Options::on_checkBox_rbds_clicked(bool checked)
{
    int idx=ui->comboBox_pty->currentIndex();

    //pty
    ui->comboBox_pty->clear();
    QStringList list;
    if(!checked)//RDS
    {
        list<<"undefined"
           <<"News"
          <<"Current affairs"
         <<"Information"
        <<"Sport"
        <<"Education"
        <<"Drama"
        <<"Culture"
        <<"Science"
        <<"Varied"
        <<"Pop music"
        <<"Rock music"
        <<"Easy listening"
        <<"Light classical"
        <<"Serious classical"
        <<"Other music"
        <<"Weather"
        <<"Finance"
        <<"Childrens programmes"
        <<"Social affairs"
        <<"Religion"
        <<"Phone in"
        <<"Travel"
        <<"Leisure"
        <<"Jazz music"
        <<"Country music"
        <<"National music"
        <<"Oldies music"
        <<"Folk music"
        <<"Documentary"
        <<"Alarm test"
        <<"Alarm";
        ui->comboBox_pty->addItems(list);
    }
    else//RBDS
    {
        list<<"undefined"
           <<"News"
          <<"Information"
         <<"Sports"
        <<"Talk"
        <<"Rock"
        <<"Classic rock"
        <<"Adult hits"
        <<"Soft rock"
        <<"Top 40"
        <<"Country"
        <<"Oldies"
        <<"Soft"
        <<"Nostalgia"
        <<"Jazz"
        <<"Classical"
        <<"Rhythm and blues"
        <<"Soft rhythm and blues"
        <<"Language"
        <<"Religious music"
        <<"Religious talk"
        <<"Personality"
        <<"Public"
        <<"College"
        <<"Spanish Talk"
        <<"Spanish Music"
        <<"Hip Hop"
        <<"Unassigned"
        <<"Unassigned"
        <<"Weather"
        <<"Emergency test"
        <<"Emergency";
        ui->comboBox_pty->addItems(list);
    }

    ui->comboBox_pty->setCurrentIndex(idx);
}

void Options::on_horizontalSlider_monolevel_valueChanged(int value)
{
    ui->label_monolevel->setText(((QString)"%1%").arg(value));
}

void Options::on_horizontalSlider_38klevel_valueChanged(int value)
{
    ui->label_38klevel->setText(((QString)"%1%").arg(value));
}

void Options::on_horizontalSlider_pilotlevel_valueChanged(int value)
{
    ui->label_pilotlevel->setText(((QString)"%1%").arg(((double)value)/10.0,0,'f',1,'0'));
}

void Options::on_horizontalSlider_rdslevel_valueChanged(int value)
{
    ui->label_rdslevel->setText(((QString)"%1%").arg(((double)value)/10.0,0,'f',1,'0'));
}

void Options::on_toolButton_5a_filename_clicked()
{

    QFileDialog dlg( NULL,tr("Select 5A file"),ui->lineEdit_5a_filename->text(), tr("Any Files (*.*)"));
    dlg.setAcceptMode( QFileDialog::AcceptOpen );
    if(dlg.exec() != QDialog::Accepted)return;
    QString filename = dlg.selectedFiles().at(0);//QFileDialog::getOpenFileName(this,tr("Select 5A file"),lastdirectorytheuserlookedat, tr("Any Files (*.*)"));
    ui->lineEdit_5a_filename->setText(filename);
}

void Options::on_checkBox_rt_enable_clicked()
{
    on_groupBox_5a_clicked();
}

void Options::on_groupBox_5a_clicked()
{
    if(ui->checkBox_rt_enable->isChecked()>0)
    {
        if(ui->groupBox_5a->isChecked())
        {
            ui->spinBox_0A_percent->setValue(40);
            ui->spinBox_2A_percent->setValue(20);
            ui->spinBox_5A_percent->setValue(40);
        }
         else
         {
            ui->spinBox_0A_percent->setValue(80);
            ui->spinBox_2A_percent->setValue(20);
            ui->spinBox_5A_percent->setValue(00);
         }
    }
     else
     {
        if(ui->groupBox_5a->isChecked())
        {
            ui->spinBox_0A_percent->setValue(50);
            ui->spinBox_2A_percent->setValue(00);
            ui->spinBox_5A_percent->setValue(50);
        }
         else
         {
            ui->spinBox_0A_percent->setValue(100);
            ui->spinBox_2A_percent->setValue(00);
            ui->spinBox_5A_percent->setValue(00);
         }
     }
}

//not used but it could if you really want to
void Options::validatepercentagespinboxes()
{
    double grp0Awantedbandwidthusage=ui->spinBox_0A_percent->value();
    double grp2Awantedbandwidthusage=ui->spinBox_2A_percent->value();
    double grp5Awantedbandwidthusage=ui->spinBox_5A_percent->value();
    double scalling=1.0/(grp0Awantedbandwidthusage+grp2Awantedbandwidthusage+grp5Awantedbandwidthusage);
    grp0Awantedbandwidthusage*=scalling;
    grp2Awantedbandwidthusage*=scalling;
    grp5Awantedbandwidthusage*=scalling;

    ui->spinBox_0A_percent->setValue(grp0Awantedbandwidthusage*100+0.5);
    ui->spinBox_2A_percent->setValue(grp2Awantedbandwidthusage*100+0.5);
    ui->spinBox_5A_percent->setValue(grp5Awantedbandwidthusage*100+0.5);
}

void Options::on_spinBox_2A_percent_valueChanged(int arg1)
{
    if(arg1==0)
    {
        ui->checkBox_rt_enable->setChecked(false);
    } else ui->checkBox_rt_enable->setChecked(true);
}

void Options::on_spinBox_5A_percent_valueChanged(int arg1)
{
    if(arg1==0)
    {
        ui->groupBox_5a->setChecked(false);
    } else ui->groupBox_5a->setChecked(true);
}
