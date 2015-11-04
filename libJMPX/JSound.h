//---------------------------------------------------------------------------

#ifndef JSOUND_H
#define JSOUND_H
//---------------------------------------------------------------------------

#include "Definitions.h"
#include "../rtaudio-4.1.1/RtAudio.h"
#include <QObject>
#include <QDebug>
#include <QThread>

using namespace std;

typedef vector<short> ShortBuffer;

enum EnumSoundDir {sdirIn,sdirOut,sdirInOut};


//---------------------------------------------------------------------------
class TJCSound : public QObject
{
         Q_OBJECT
public:
         TJCSound(QObject *parent = 0);
         ~TJCSound();

private:
        static int Dispatcher( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData )
        {
            Q_UNUSED(streamTime);
            Q_UNUSED(status);
            TJCSound *pInstance=(TJCSound*)userData;
            pInstance->SoundEvent((double*)inputBuffer,(double*)outputBuffer,2*nBufferFrames);
            return 0;
        }
        vector<SDeviceInfo> Device;
        vector<RtAudio::DeviceInfo> RtAudioDevices;

        bool SetSoundCardInByName();
        bool SetSoundCardOutByName();

public:
        void Active(bool State);
        bool IsActive();

        void PopulateDevices();
        SDevices Devices;

        RtAudio AnRtAudio;

    	RtAudio::StreamParameters oParameters,iParameters;
    	unsigned int sampleRate;
    	unsigned int bufferFrames;
        QString wantedInDeviceName;
        QString wantedOutDeviceName;

        RtAudio::StreamOptions options;

        string LastErrorMessage;
        bool GotError;

signals:
        void SoundEvent(double* inputBuffer,double* outputBuffer,int nBufferFrames);
protected:


public:

protected:







};
//---------------------------------------------------------------------------






#endif  //JSOUND_H
