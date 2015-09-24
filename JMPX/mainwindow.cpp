#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QThread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setFixedSize(width(),height());
    ui->lvolumemeter->setDirection(Volumemeter::Vertical);
    ui->rvolumemeter->setDirection(Volumemeter::Vertical);
    ui->outvolumemeter->setDirection(Volumemeter::Vertical);

    QLibrary library("../build-libJMPX-Desktop_64bit_MinGW-Release/release/libJMPX");
    //QLibrary library("libJMPX");
    if (!library.load())
    {
        qDebug() << library.errorString();
        QMessageBox::critical(this,"Error","<p><b>Error loading JMPX library.</b></p><p>"+library.errorString()+"</p><p>Note: The JMPX library needs to be placed in the gui's program directory or in system path. On Windows this should be called libJMPX.dll on linux something like libJQAM.so. Please find it and copy it over for JMPX to work.</p>");
    }
    if (library.load())
    {
        qDebug() << "library loaded";
        createJMPXfunction createJMPX = (createJMPXfunction)library.resolve("createObject");
        pJMPX = createJMPX(this);
        if(pJMPX)
        {
            pJMPX->SetPreEmphasis(WORLD);
            SDevices* pdev=pJMPX->GetDevices();
            ui->comboBox->clear();
            ui->comboBox->addItem("Default");
            for(unsigned int i=0;i<pdev->NumberOfDevices;i++)
            {
                ui->comboBox->addItem(pdev->Device[i].name);
            }
            if(!pdev->NumberOfDevices)ui->comboBox->addItem("None");
        }
         else QMessageBox::critical(this,"Error","<p><b>Error loading JMPX library.</b></p><p>Failed to create device</p>");
    }
    ptimer= new QTimer(this);
    ptimer->setInterval(20);
    connect(ptimer,SIGNAL(timeout()),this,SLOT(updatedisplay()));


    //thanks doqtor for this solution to text overflow of qcomboboxes
    //determinge the maximum width required to display all names in full
    int max_width = 0;
    QFontMetrics fm(ui->comboBox->font());
    for(int x = 0; x < ui->comboBox->count(); ++x)
    {
        int width = fm.width(ui->comboBox->itemText(x));
        if(width > max_width)
            max_width = width;
    }
    if(ui->comboBox->view()->minimumWidth() < max_width)
    {
        // add scrollbar width and margin
        max_width += ui->comboBox->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
        max_width += ui->comboBox->view()->autoScrollMargin();
        // set the minimum width of the combobox drop down list
        ui->comboBox->view()->setMinimumWidth(max_width);
    }


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::on_checkBox_stateChanged(int state)
{
    if(!pJMPX)return;
    if((!pJMPX->IsActive())&&(state))
    {
        pJMPX->SetSampleRate(192000);
        pJMPX->SetSoundCard(ui->comboBox->currentIndex()-1);//-1 cos added a default item to the combobox
        pJMPX->SetBufferFrames(8096*3/2);//adjust this for latency or for lost frames
    }

    pJMPX->Active(state);
    if((state)&&(pJMPX->GotError()))
    {
        QMessageBox msgBox;
        msgBox.setText(pJMPX->GetLastRTAudioError());
        msgBox.exec();
        state=false;
    }
    pJMPX->EnableStereo(ui->checkBox_2->isChecked());

    if(ui->radionone->isChecked())pJMPX->SetPreEmphasis(NONE);
     else if(ui->radio50->isChecked())pJMPX->SetPreEmphasis(WORLD);
      else if(ui->radio75->isChecked())pJMPX->SetPreEmphasis(USA);

    if(state)ptimer->start();
     else ptimer->stop();
     ui->lvolumemeter->setVolume(0);
     ui->rvolumemeter->setVolume(0);
     ui->outvolumemeter->setVolume(0);
}

void MainWindow::on_checkBox_2_stateChanged(int state)
{
    if(!pJMPX)return;
    pJMPX->EnableStereo(state);
}

void MainWindow::updatedisplay()
{
    if(!pJMPX)return;
    TSigStats *psigstats=pJMPX->GetSignalStats();
    ui->lvolumemeter->setVolume(psigstats->lvol);
    ui->rvolumemeter->setVolume(psigstats->rvol);
    ui->outvolumemeter->setVolume(psigstats->outvol);
}

void MainWindow::on_radio50_clicked()
{
    if(!pJMPX)return;
    pJMPX->SetPreEmphasis(WORLD);
}

void MainWindow::on_radio75_clicked()
{
    if(!pJMPX)return;
    pJMPX->SetPreEmphasis(USA);
}

void MainWindow::on_radionone_clicked()
{
    if(!pJMPX)return;
    pJMPX->SetPreEmphasis(NONE);
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    if(!pJMPX)return;
    pJMPX->SetSoundCard(index-1);
}
