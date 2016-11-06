#ifndef LIBJMPX_H
#define LIBJMPX_H

#include <QObject>
#include "JMPXInterface.h"
#include "JDSP.h"
#include "JSound.h"

#include "rds.h"

class JMPXEncoder : public JMPXInterface
{
     Q_OBJECT
public:
    JMPXEncoder(QObject *parent = 0);
    ~JMPXEncoder();

private:
    TJCSound *pJCSound;
    TSetGen ASetGen;
    std::auto_ptr< TDspGen > pTDspGen;
    std::auto_ptr< WaveTable > pWaveTable;

    bool stereo;
    bool RDS_enabled;

    TSigStats sigstats;

    PreEmphasis lpreemp;
    PreEmphasis rpreemp;

    Clipper clipper;

    double lbigval;
    double rbigval;
    double outbigval;
    int decl;

    RDS *rds;

    QString rt_default;
    QString rt_dynamic;

    double pilotlevel;
    double rdslevel;
    double monolevel;
    double level38k;
    bool compositeclipper;

public:

    void SetEnableStereo(bool enable);
    bool GetEnableStereo();

    void Active(bool Enabled);
    bool IsActive(){return pJCSound->IsActive();}



    QString GetSoundCardInName()
    {
        for(unsigned int device=0;device<pJCSound->Devices.NumberOfDevices;device++)
        {
            if(pJCSound->Devices.Device[device].dev==pJCSound->iParameters.deviceId)
            {
                return ((QString)pJCSound->Devices.Device[device].name);
            }
        }
        return ("None");
    }
    QString GetSoundCardOutName()
    {
        for(unsigned int device=0;device<pJCSound->Devices.NumberOfDevices;device++)
        {
            if(pJCSound->Devices.Device[device].dev==pJCSound->oParameters.deviceId)
            {
                return ((QString)pJCSound->Devices.Device[device].name);
            }
        }
        return ("None");
    }
    bool SetSoundCardInName(const QString &name)
    {
        int firstindevice=-1;
        for(unsigned int device=0;device<pJCSound->Devices.NumberOfDevices;device++)
        {
            if(pJCSound->Devices.Device[device].inchannelcount==0)continue;
            if(firstindevice<0)firstindevice=device;
            if(((QString)pJCSound->Devices.Device[device].name)==name)
            {
                SetSoundCardIn(device);
                pJCSound->wantedInDeviceName=name;
                return true;
            }
        }
        SetSoundCardIn(firstindevice);
        pJCSound->wantedInDeviceName=name;
        return false;
    }
    bool SetSoundCardOutName(const QString &name)
    {
        int firstindevice=-1;
        for(unsigned int device=0;device<pJCSound->Devices.NumberOfDevices;device++)
        {
            if(pJCSound->Devices.Device[device].outchannelcount==0)continue;
            if(firstindevice<0)firstindevice=device;
            if(((QString)pJCSound->Devices.Device[device].name)==name)
            {
                SetSoundCardOut(device);
                pJCSound->wantedOutDeviceName=name;
                return true;
            }
        }
        SetSoundCardOut(firstindevice);
        pJCSound->wantedOutDeviceName=name;
        return false;
    }
    void SetSoundCardDefault(){SetSoundCardIn(-1);SetSoundCardOut(-1);}
    void SetSoundCard(int device){SetSoundCardIn(device);SetSoundCardOut(device);}
    void SetSoundCardIn(int device)
    {
        if(device<0)pJCSound->iParameters.deviceId=pJCSound->AnRtAudio.getDefaultInputDevice();
         else pJCSound->iParameters.deviceId=device;
        pJCSound->wantedInDeviceName.clear();
    }
    void SetSoundCardOut(int device)
    {
        if(device<0)pJCSound->oParameters.deviceId=pJCSound->AnRtAudio.getDefaultOutputDevice();
         else pJCSound->oParameters.deviceId=device;
        pJCSound->wantedOutDeviceName.clear();
    }
    int GetSoundCardIn()
    {
        return pJCSound->iParameters.deviceId;
    }
    int GetSoundCardOut()
    {
        return pJCSound->oParameters.deviceId;
    }

    void SetSampleRate(int sampleRate){pJCSound->sampleRate=sampleRate;}
    void SetBufferFrames(int bufferFrames){pJCSound->bufferFrames=bufferFrames;}

    void SetPreEmphasis(TimeConstant timeconst);
    TimeConstant GetPreEmphasis();

    bool GotError(){return pJCSound->GotError;}
    const char* GetLastRTAudioError(){pJCSound->GotError=false;return pJCSound->LastErrorMessage.data();}

    TSigStats* GetSignalStats();

    SDevices* GetDevices(void)
    {
        pJCSound->PopulateDevices();
        return &pJCSound->Devices;
    }

    void SetEnableCompositeClipper(bool enable){compositeclipper=enable;}
    bool GetEnableCompositeClipper(){return compositeclipper;}
    void SetMonoLevel(double value){monolevel=value;}
    double GetMonoLevel(){return monolevel;}
    void Set38kLevel(double value){level38k=value;}
    double Get38kLevel(){return level38k;}
    void SetPilotLevel(double value){pilotlevel=value;}
    double GetPilotLevel(){return pilotlevel;}
    void SetRDSLevel(double value){rdslevel=value;}
    double GetRDSLevel(){return rdslevel;}

    //RDS interface to implimentation start

    void SetEnableRDS(bool enable);
    bool GetEnableRDS();

    void RDS_SetPI(int pi){rds->set_pi((quint16)pi);}
    int RDS_GetPI(){return rds->get_pi();}

    void RDS_SetPS(const QString &ps){rds->set_ps(ps);}
    QString RDS_GetPS(){return rds->get_ps();}

    void RDS_SetDefaultRT(const QString &rt)
    {
        rt_default=rt;
        if(rt_dynamic.isEmpty())rds->set_rt(rt_default);
    }
    QString RDS_GetDefaultRT(){return rt_default;}
    void RDS_SetRT(const QString &rt)
    {
        rt_dynamic=rt;
        if(rt_dynamic.isEmpty())rds->set_rt(rt_default);
         else rds->set_rt(rt_dynamic);
    }
    QString RDS_GetRT(){return rds->get_rt();}

    void RDS_SetPTY(int pty){rds->set_pty((RDS::pty_type)pty);}
    int RDS_GetPTY(){return (int)rds->get_pty();}

    void RDS_Set_DI_Stereo(bool enable){rds->set_stereo(enable);}
    bool RDS_Get_DI_Stereo(){return rds->get_stereo();}
    void RDS_Set_DI_Compressed(bool enable){rds->set_compressed(enable);}
    bool RDS_Get_DI_Compressed(){return rds->get_compressed();}
    void RDS_Set_DI_Artificial_Head(bool enable){rds->set_artificial_head(enable);}
    bool RDS_Get_DI_Artificial_Head(){return rds->get_artificial_head();}
    void RDS_Set_DI_Dynamic_PTY(bool enable){rds->set_dynamic_pty(enable);}
    bool RDS_Get_DI_Dynamic_PTY(){return rds->get_dynamic_pty();}

    void RDS_Set_TP(bool enable){rds->set_tp(enable);}
    bool RDS_Get_TP(){return rds->get_tp();}
    void RDS_Set_CT(bool enable){rds->ct_enabled=enable;}
    bool RDS_Get_CT(){return rds->ct_enabled;}    
    void RDS_Set_MS(bool enable){rds->set_ms(enable);}
    bool RDS_Get_MS(){return rds->get_ms();}
    void RDS_Set_TA(bool enable){rds->set_ta(enable);}
    bool RDS_Get_TA(){return rds->get_ta();}

    void RDS_Set_RBDS(bool enable){rds->rbds=enable;}
    bool RDS_Get_RBDS(){return rds->rbds;}

    void RDS_Set_RT_Enable(bool enable){rds->set_rt_enable(enable);}
    bool RDS_Get_RT_Enable(){return rds->get_rt_enable();}

    void RDS_Set_5A_Enable(bool enable){rds->set_5a_enable(enable);}
    bool RDS_Get_5A_Enable(){return rds->get_5a_enable();}

    void RDS_Set_5A_data(const QByteArray &data){rds->set_5a_data(data);}

    void RDS_Set_clocktimeoffset(int offset_in_groups){rds->clocktimeoffset=offset_in_groups;}
    int RDS_Get_clocktimeoffset(){return rds->clocktimeoffset;}

    double RDS_Get_altfreq1(){return rds->get_altfreq1();}
    void RDS_Set_altfreq1(double freq){rds->set_altfreq1(freq);}

    double RDS_Get_altfreq2(){return rds->get_altfreq2();}
    void RDS_Set_altfreq2(double freq){rds->set_altfreq2(freq);}

    double RDS_Get_grp0Awantedbandwidthusage(){return rds->get_grp0Awantedbandwidthusage();}
    double RDS_Get_grp2Awantedbandwidthusage(){return rds->get_grp2Awantedbandwidthusage();}
    double RDS_Get_grp5Awantedbandwidthusage(){return rds->get_grp5Awantedbandwidthusage();}
    void RDS_Set_grouppercentages(double grp0Awantedbandwidthusage,double grp2Awantedbandwidthusage,double grp5Awantedbandwidthusage){rds->set_grouppercentages(grp0Awantedbandwidthusage,grp2Awantedbandwidthusage,grp5Awantedbandwidthusage);}

    //RDS interface to implimentation stop

private slots:
    void Update(double *DataIn,double *DataOut, int Size);
private:
    InterleavedStereo16500Hz192000bpsFilter stereofir;//fast fir LPF
private slots:

};

#endif	// LIBJMPX_H

