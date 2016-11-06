#ifndef JMPXINTERFACE_H
#define	JMPXINTERFACE_H

#include <QtCore/qglobal.h>
#include <QObject>
#include "Definitions.h"

#if defined(JMPX_LIBRARY)
#  define JMPXSHARED_EXPORT Q_DECL_EXPORT
#else
#  define JMPXSHARED_EXPORT Q_DECL_IMPORT
#endif

class JMPXInterface : public QObject
{
public:
    virtual void Active(bool Enabled)=0;

    virtual bool IsActive()=0;

    virtual void SetSoundCard(int device)=0;
    virtual void SetSoundCardIn(int device)=0;
    virtual void SetSoundCardOut(int device)=0;
    virtual bool SetSoundCardInName(const QString &name)=0;
    virtual bool SetSoundCardOutName(const QString &name)=0;
    virtual void SetSoundCardDefault()=0;
    virtual void SetSampleRate(int sampleRate)=0;
    virtual void SetBufferFrames(int bufferFrames)=0;
    virtual int GetSoundCardIn()=0;
    virtual int GetSoundCardOut()=0;
    virtual QString GetSoundCardInName()=0;
    virtual QString GetSoundCardOutName()=0;

    virtual SDevices* GetDevices(void)=0;

    JMPXInterface(QObject *parent = 0):QObject(parent){}
    virtual ~JMPXInterface(){}

    virtual void SetEnableStereo(bool enable)=0;
    virtual bool GetEnableStereo()=0;

    virtual void SetPreEmphasis(TimeConstant timeconst)=0;
    virtual TimeConstant GetPreEmphasis()=0;

    virtual TSigStats* GetSignalStats()=0;

    virtual bool GotError()=0;
    virtual const char* GetLastRTAudioError()=0;

    virtual void SetEnableCompositeClipper(bool enable)=0;
    virtual bool GetEnableCompositeClipper()=0;
    virtual void SetMonoLevel(double value)=0;
    virtual double GetMonoLevel()=0;
    virtual void Set38kLevel(double value)=0;
    virtual double Get38kLevel()=0;
    virtual void SetPilotLevel(double value)=0;
    virtual double GetPilotLevel()=0;
    virtual void SetRDSLevel(double value)=0;
    virtual double GetRDSLevel()=0;

    //--RDS interface

    virtual void SetEnableRDS(bool enable)=0;
    virtual bool GetEnableRDS()=0;

    virtual void RDS_SetPI(int pi)=0;
    virtual int RDS_GetPI()=0;

    virtual void RDS_SetPS(const QString &ps)=0;
    virtual QString RDS_GetPS()=0;

    virtual void RDS_SetDefaultRT(const QString &rt)=0;
    virtual QString RDS_GetDefaultRT()=0;
    virtual void RDS_SetRT(const QString &rt)=0;
    virtual QString RDS_GetRT()=0;

    virtual void RDS_SetPTY(int pty)=0;
    virtual int RDS_GetPTY()=0;

    virtual void RDS_Set_DI_Stereo(bool enable)=0;
    virtual bool RDS_Get_DI_Stereo()=0;
    virtual void RDS_Set_DI_Compressed(bool enable)=0;
    virtual bool RDS_Get_DI_Compressed()=0;
    virtual void RDS_Set_DI_Artificial_Head(bool enable)=0;
    virtual bool RDS_Get_DI_Artificial_Head()=0;
    virtual void RDS_Set_DI_Dynamic_PTY(bool enable)=0;
    virtual bool RDS_Get_DI_Dynamic_PTY()=0;

    virtual void RDS_Set_TP(bool enable)=0;
    virtual bool RDS_Get_TP()=0;
    virtual void RDS_Set_CT(bool enable)=0;
    virtual bool RDS_Get_CT()=0;
    virtual void RDS_Set_RBDS(bool enable)=0;
    virtual bool RDS_Get_RBDS()=0;
    virtual void RDS_Set_MS(bool enable)=0;
    virtual bool RDS_Get_MS()=0;
    virtual void RDS_Set_TA(bool enable)=0;
    virtual bool RDS_Get_TA()=0;

    virtual void RDS_Set_RT_Enable(bool enable)=0;
    virtual bool RDS_Get_RT_Enable()=0;

    virtual void RDS_Set_5A_Enable(bool enable)=0;
    virtual bool RDS_Get_5A_Enable()=0;

    virtual void RDS_Set_5A_data(const QByteArray &data)=0;

    virtual void RDS_Set_clocktimeoffset(int offset_in_groups)=0;
    virtual int RDS_Get_clocktimeoffset()=0;

    virtual double RDS_Get_altfreq1()=0;
    virtual void RDS_Set_altfreq1(double freq)=0;
    virtual double RDS_Get_altfreq2()=0;
    virtual void RDS_Set_altfreq2(double freq)=0;

    virtual double RDS_Get_grp0Awantedbandwidthusage()=0;
    virtual double RDS_Get_grp2Awantedbandwidthusage()=0;
    virtual double RDS_Get_grp5Awantedbandwidthusage()=0;
    virtual void RDS_Set_grouppercentages(double grp0Awantedbandwidthusage,double grp2Awantedbandwidthusage,double grp5Awantedbandwidthusage)=0;



    //--RDS interface end

signals:


};

extern "C" JMPXSHARED_EXPORT JMPXInterface* createObject(QObject *parent);
typedef JMPXInterface*(*createJMPXfunction)(QObject *parent);

#endif	// JMPXINTERFACE_H


