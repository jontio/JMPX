//---------------------------------------------------------------------------

#ifndef JSOUND_H
#define JSOUND_H
//---------------------------------------------------------------------------

#include "Definitions.h"
#include "../rtaudio-4.1.2/RtAudio.h"
#include <QObject>
#include <QDebug>
#include <QThread>


//---------------------------------------------------------------------------
class TJCSound : public QObject
{
         Q_OBJECT
public:
         TJCSound(QObject *parent = 0);
         ~TJCSound();

private:
         static int Dispatcher_double( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData )
         {
             Q_UNUSED(streamTime);
             Q_UNUSED(status);
             TJCSound *pInstance=(TJCSound*)userData;
             if(nBufferFrames>0)pInstance->SoundEvent((double*)inputBuffer,(double*)outputBuffer,nBufferFrames);
             return 0;
         }
         static int Dispatcher_qint32( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData )
         {
             Q_UNUSED(streamTime);
             Q_UNUSED(status);
             TJCSound *pInstance=(TJCSound*)userData;
             if(nBufferFrames>0)pInstance->SoundEvent((qint32*)inputBuffer,(qint32*)outputBuffer,nBufferFrames);
             return 0;
         }
         static int Dispatcher_qint16( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData )
         {
             Q_UNUSED(streamTime);
             Q_UNUSED(status);
             TJCSound *pInstance=(TJCSound*)userData;
             if(nBufferFrames>0)pInstance->SoundEvent((qint16*)inputBuffer,(qint16*)outputBuffer,nBufferFrames);
             return 0;
         }
        std::vector<SDeviceInfo> Device;
        std::vector<RtAudio::DeviceInfo> RtAudioDevices;

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
        RtAudioFormat audioformat;

        std::string LastErrorMessage;
        bool GotError;

signals:
        void SoundEvent(double* inputBuffer,double* outputBuffer,int nBufferFrames);
        void SoundEvent(qint32* inputBuffer,qint32* outputBuffer,int nBufferFrames);
        void SoundEvent(qint16* inputBuffer,qint16* outputBuffer,int nBufferFrames);
protected:


public:

protected:







};
//---------------------------------------------------------------------------






#endif  //JSOUND_H
