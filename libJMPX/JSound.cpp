//---------------------------------------------------------------------------

#include "JSound.h"
#include <QDebug>

 TJCSound::TJCSound(QObject *parent) : QObject(parent)
{

        GotError=false;

        oParameters.deviceId = AnRtAudio.getDefaultOutputDevice();
        oParameters.nChannels = 2;
        oParameters.firstChannel = 0;
        
        iParameters.deviceId = AnRtAudio.getDefaultInputDevice();
        iParameters.nChannels = 2;
        iParameters.firstChannel = 0;

        sampleRate = 192000;
        bufferFrames = 8096; // 256 sample frames

        PopulateDevices();

}

void TJCSound::PopulateDevices()
{
	try
	{
		Device.resize(AnRtAudio.getDeviceCount());
		RtAudioDevices.resize(Device.size());
		Devices.NumberOfDevices=0;
		for(unsigned int dev=0;dev<Device.size();dev++)
		{
			RtAudioDevices[dev]=AnRtAudio.getDeviceInfo(dev);
			if(RtAudioDevices[dev].probed)
			{

                qDebug()<<dev;
                qDebug()<<RtAudioDevices[dev].name.c_str();
                qDebug()<<"inputChannels="<<RtAudioDevices[dev].inputChannels;
                qDebug()<<"outputChannels="<<RtAudioDevices[dev].outputChannels;


                Device[Devices.NumberOfDevices].dev=dev;
				Device[Devices.NumberOfDevices].name=RtAudioDevices[dev].name.c_str();
				Device[Devices.NumberOfDevices].SampleRates=&RtAudioDevices[dev].sampleRates[0];
				Device[Devices.NumberOfDevices].NumberOfSampleRates=RtAudioDevices[dev].sampleRates.size();
				Devices.NumberOfDevices++;
			}
		}
		Devices.Device=&Device[0];
	}
    catch(RtAudioError& e)
	{
		LastErrorMessage=e.getMessage();
		GotError=true;
	}
	if(GotError)throw(LastErrorMessage.c_str());
}

void TJCSound::Active(bool State)
{
	try
	{
		if(AnRtAudio.isStreamOpen()==State)return;
        if(State)
        {
        	if(bufferFrames%2)bufferFrames++; //make sure bufferFrames is even
            AnRtAudio.openStream( &oParameters,  &iParameters, RTAUDIO_FLOAT64,sampleRate, &bufferFrames, Dispatcher, (void *)this, &options );
            AnRtAudio.startStream();
        }
         else
         {
        	 try
        	 {
        		 AnRtAudio.stopStream();
        	 }
             catch(RtAudioError& e)
	         {
        		 LastErrorMessage=e.getMessage();
	        	 GotError=true;
	         }
             if ( AnRtAudio.isStreamOpen() ) AnRtAudio.closeStream();
         }
    }
    catch(RtAudioError& e)
    {
    	LastErrorMessage=e.getMessage();
    	GotError=true;
    	try{AnRtAudio.stopStream();}
    	catch(...){}
    	if ( AnRtAudio.isStreamOpen() ) AnRtAudio.closeStream();
    }
}

bool TJCSound::IsActive()
{
	return AnRtAudio.isStreamOpen();
}


 TJCSound::~TJCSound()
{
        try{AnRtAudio.stopStream();}
        catch(...){}
        if ( AnRtAudio.isStreamOpen() ) AnRtAudio.closeStream();
}

//---------------------------------------------------------------------------




