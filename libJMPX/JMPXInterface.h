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
    virtual void SetSoundCardDefault()=0;
    virtual void SetSampleRate(int sampleRate)=0;
    virtual void SetBufferFrames(int bufferFrames)=0;

    virtual SDevices* GetDevices(void)=0;

    JMPXInterface(QObject *parent = 0):QObject(parent){}
    virtual ~JMPXInterface(){}

    virtual void EnableStereo(bool enable)=0;

    virtual void SetPreEmphasis(TimeConstant timeconst)=0;

    virtual TSigStats* GetSignalStats()=0;

    virtual bool GotError()=0;
    virtual const char* GetLastRTAudioError()=0;

signals:


};

extern "C" JMPXSHARED_EXPORT JMPXInterface* createObject(QObject *parent);
typedef JMPXInterface*(*createJMPXfunction)(QObject *parent);

#endif	// JMPXINTERFACE_H


