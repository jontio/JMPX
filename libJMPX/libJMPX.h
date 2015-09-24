#ifndef LIBJMPX_H
#define LIBJMPX_H

#include <QObject>
#include "JMPXInterface.h"
#include "JDSP.h"
#include "JSound.h"

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

    TSigStats sigstats;

    PreEmphasis lpreemp;
    PreEmphasis rpreemp;

    Clipper clipper;

    double lbigval;
    double rbigval;
    double outbigval;
    int decl;

public:

    void EnableStereo(bool enable);

    void Active(bool Enabled);
    bool IsActive(){return pJCSound->IsActive();}

    void SetSoundCardDefault(){SetSoundCardIn(-1);SetSoundCardOut(-1);}
    void SetSoundCard(int device){SetSoundCardIn(device);SetSoundCardOut(device);}
    void SetSoundCardIn(int device)
    {
        if(device<0)pJCSound->iParameters.deviceId=pJCSound->AnRtAudio.getDefaultInputDevice();
         else pJCSound->iParameters.deviceId=device;
    }
    void SetSoundCardOut(int device)
    {
        if(device<0)pJCSound->oParameters.deviceId=pJCSound->AnRtAudio.getDefaultOutputDevice();
         else pJCSound->oParameters.deviceId=device;
    }
    void SetSampleRate(int sampleRate){pJCSound->sampleRate=sampleRate;}
    void SetBufferFrames(int bufferFrames){pJCSound->bufferFrames=bufferFrames;}

    void SetPreEmphasis(TimeConstant timeconst);

    bool GotError(){return pJCSound->GotError;}
    const char* GetLastRTAudioError(){pJCSound->GotError=false;return pJCSound->LastErrorMessage.data();}

    TSigStats* GetSignalStats();

    SDevices* GetDevices(void)
    {
        return &pJCSound->Devices;
    }
private slots:
    void Update(double *DataIn,double *DataOut, int Size);
private:
    //FIRFilter ltfir,rtfir;//old method (slow fir)
    InterleavedStereo16500Hz192000bpsFilter stereofir;//new method (fast fir)
};

#endif	// LIBJMPX_H

