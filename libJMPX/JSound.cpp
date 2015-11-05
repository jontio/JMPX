//---------------------------------------------------------------------------

#include "JSound.h"
#include <QDebug>

 TJCSound::TJCSound(QObject *parent) : QObject(parent)
{

        GotError=false;

        options.streamName="JMPX";

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

                /*qDebug()<<dev;
                qDebug()<<RtAudioDevices[dev].name.c_str();
                qDebug()<<"inputChannels="<<RtAudioDevices[dev].inputChannels;
                qDebug()<<"outputChannels="<<RtAudioDevices[dev].outputChannels;*/

                Device[Devices.NumberOfDevices].dev=dev;
				Device[Devices.NumberOfDevices].name=RtAudioDevices[dev].name.c_str();
				Device[Devices.NumberOfDevices].SampleRates=&RtAudioDevices[dev].sampleRates[0];
				Device[Devices.NumberOfDevices].NumberOfSampleRates=RtAudioDevices[dev].sampleRates.size();
                Device[Devices.NumberOfDevices].inchannelcount=RtAudioDevices[dev].inputChannels;
                Device[Devices.NumberOfDevices].outchannelcount=RtAudioDevices[dev].outputChannels;

                //if Jack dont include ourselves
                if(AnRtAudio.getCurrentApi()==RtAudio::UNIX_JACK)
                {
                    if(RtAudioDevices[dev].name==options.streamName)
                    {
                        Device[Devices.NumberOfDevices].inchannelcount=0;
                        Device[Devices.NumberOfDevices].outchannelcount=0;
                    }
                }

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

bool TJCSound::SetSoundCardInByName()
{
    if(wantedInDeviceName.isEmpty())return false;
    int firstindevice=-1;
    for(unsigned int device=0;device<Devices.NumberOfDevices;device++)
    {
        if(Devices.Device[device].inchannelcount==0)continue;
        if(firstindevice<0)firstindevice=device;
        if(((QString)Devices.Device[device].name)==wantedInDeviceName)
        {
            iParameters.deviceId=device;
            return true;
        }
    }
    if(firstindevice<0)iParameters.deviceId=AnRtAudio.getDefaultInputDevice();
     else iParameters.deviceId=firstindevice;
    return false;
}

bool TJCSound::SetSoundCardOutByName()
{
    if(wantedOutDeviceName.isEmpty())return false;
    int firstindevice=-1;
    for(unsigned int device=0;device<Devices.NumberOfDevices;device++)
    {
        if(Devices.Device[device].outchannelcount==0)continue;
        if(firstindevice<0)firstindevice=device;
        if(((QString)Devices.Device[device].name)==wantedOutDeviceName)
        {
            oParameters.deviceId=device;
            return true;
        }
    }
    if(firstindevice<0)oParameters.deviceId=AnRtAudio.getDefaultOutputDevice();
     else oParameters.deviceId=firstindevice;
    return false;
}

void TJCSound::Active(bool State)
{
	try
	{
		if(AnRtAudio.isStreamOpen()==State)return;
        if(State)
        {

            //names can change id number
            PopulateDevices();
            SetSoundCardInByName();
            SetSoundCardOutByName();

        	if(bufferFrames%2)bufferFrames++; //make sure bufferFrames is even

            //10 tries as on JACK sometimes stream wont open first time. Don't know why. This should solve the problem.
            int i=0;
            tryagain:
            AnRtAudio.openStream( &oParameters,  &iParameters, RTAUDIO_FLOAT64,sampleRate, &bufferFrames, Dispatcher, (void *)this, &options );
            i++;
            try{AnRtAudio.startStream();}
            catch(RtAudioError& e)
            {
                if(i>=10)
                {
                    LastErrorMessage=e.getMessage();
                    GotError=true;
                }
                if ( AnRtAudio.isStreamOpen() ) AnRtAudio.closeStream();
                if(i<10)goto tryagain;
                return;
            }



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




