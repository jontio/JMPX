#include "libJMPX.h"
#include <QDebug>
#include <QtEndian>

#include <random>

#include <QTimer>

JMPXEncoder::JMPXEncoder(QObject *parent):
    JMPXInterface(parent),
	pTDspGen( new TDspGen(&ASetGen)),
    pWaveTable( new WaveTable(pTDspGen.get(),19000)),
    pFMModulator( new FMModulator(pTDspGen.get()))
{
	stereo=true;
    RDS_enabled=true;
    SCA_enabled=true;
    dsca_enable_send_rds=true;
    decl=0;
    scabigval=0.0;
    lbigval=0.0;
    rbigval=0.0;
    outbigval=0.0;
    sigstats.scavol=0.0;
    sigstats.lvol=0.0;
    sigstats.rvol=0.0;
    sigstats.outvol=0.0;
    sigstats.opusbufferuseagepercent=0.0;

    pOQPSKModulator = new OQPSKModulator(pTDspGen.get(),this);

    pJCSound = new TJCSound(this);
    pJCSound->iParameters.nChannels=2;
    pJCSound->oParameters.nChannels=2;
    pJCSound->sampleRate=192000;
    pJCSound->bufferFrames=8096;
    pJCSound->options.streamName="JMPX";
    rds = new RDS(this);

    //SCA setup
    pJCSound_SCA = new TJCSound(this);
    pJCSound_SCA->iParameters.nChannels=2;//stero input
    pJCSound_SCA->oParameters.nChannels=0;//no output channels
    pJCSound_SCA->options.streamName="JMPX_SCA";
    pJCSound_SCA->sampleRate=48000;//48khz
    SCA_MaxDeviation=3000;//3khz
    SCA_Level=0.1;//10%
    SCA_CarrierFrequency=67500;//67.5khz
    SCA_MaxInputFrequency=5000;//5khz
    //

    compositeclipper=true;
    monolevel=0.9;
    level38k=1.0;
    pilotlevel=0.07;
    rdslevel=0.06;

    scaPeak.setSettings(40,0.01,0.8,0.0001,100,false);
    lPeak.setSettings(40,0.01,0.8,0.0001,100,false);
    rPeak.setSettings(40,0.01,0.8,0.0001,100,false);
    outPeak.setSettings(160,0.01/4.0,0.95,0.0001/4.0,100,false);
    opusBufferUsagePeak.setSettings(160,0.001,0.99,0.0001/10.0,40000,true);

    rdsbpf = new JFastFIRFilter;
    rdsbpf->setKernel(JFilterDesign::BandPassHanning(57000-2400,57000+2400,pJCSound->sampleRate,512-1));

    //oqpsk modulator for opus
    connect(pOQPSKModulator,SIGNAL(CallForMoreData(int)),this,SLOT(onCallForMoreData(int)));

    //data formatter for modulator
    oqpskdataformatter.setMode(DataFormatter::mode1);

    //opus setup
    int err;
    encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_VOIP,&err);//APPLICATION, &err);
    if (err<0)qDebug()<<"failed to create opus encoder: "<<opus_strerror(err);
    int application;
    opus_encoder_ctl(encoder, OPUS_GET_APPLICATION(&application));
    //qDebug()<<"start application= "<<application;
    err = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(BITRATE));
    if (err<0)qDebug()<<"failed to set bitrate: "<<opus_strerror(err);
    SCA_opus=true;

    //periodic rds info sending
    QTimer *timer=new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(DSCAsendRds()));
    timer->start(4000);

}

JMPXEncoder::~JMPXEncoder()
{
    pJCSound->Active(false);
    pJCSound_SCA->Active(false);
    delete rdsbpf;
    opus_encoder_destroy(encoder);
}

void JMPXEncoder::onCallForMoreData(int maxbitswanted)
{
    Q_UNUSED(maxbitswanted);
    callback_mutex.lock();
    pOQPSKModulator->LoadBits(oqpskdataformatter.getFrame());
    callback_mutex.unlock();
}

void JMPXEncoder::DSCAsendRds()
{
    if(!dsca_enable_send_rds)return;
    if(!pOQPSKModulator->isSpooling())return;

    QByteArray ba;
    ba.push_back(rds->get_ps().size());
    ba+=rds->get_ps().toLatin1();
    ba.push_back(rds->get_rt().size());
    ba+=rds->get_rt().toLatin1();

    oqpskdataformatter.pushPacket(DataFormatter::PacketType_RDS,ba);
}

void JMPXEncoder::Active(bool Enabled)
{
        if(Enabled==pJCSound->IsActive())return;
        if(Enabled)
        {//here we dont need lock i think as other threads are stoped
            disconnect(pJCSound,SIGNAL(SoundEvent(double*,double*,int)),this,SLOT(Update(double*,double*,int)));

            ASetGen.SampleRate=pJCSound->sampleRate;
            pTDspGen->ResetSettings();

            pWaveTable->RefreshSettings(19000.0);

            pOQPSKModulator->RefreshSettings();//incase samplerate has changed

            pFMModulator->RefreshSettings();//incase samplerate has changed

            rds->reset();

            rt_dynamic.clear();
            rds->set_rt(rt_default);

            //4.8khz rds bpf
            rdsbpf->setKernel(JFilterDesign::BandPassHanning(57000-2400,57000+2400,pJCSound->sampleRate,512-1));

            //enable sca connections
            oqpskdataformatter.clearBuffer();
            disconnect(pJCSound_SCA,SIGNAL(SoundEvent(double*,double*,int)),this,SLOT(Update_SCA(double*,double*,int)));
            disconnect(pJCSound_SCA,SIGNAL(SoundEvent(qint16*,qint16*,int)),this,SLOT(Update_opusSCA(qint16*,qint16*,int)));
            if(SCA_opus){pJCSound_SCA->audioformat=RTAUDIO_SINT16;connect(pJCSound_SCA,SIGNAL(SoundEvent(qint16*,qint16*,int)),this,SLOT(Update_opusSCA(qint16*,qint16*,int)),Qt::DirectConnection);}//DirectConnection!!!
             else {pJCSound_SCA->audioformat=RTAUDIO_FLOAT64;connect(pJCSound_SCA,SIGNAL(SoundEvent(double*,double*,int)),this,SLOT(Update_SCA(double*,double*,int)),Qt::DirectConnection);}//DirectConnection!!!

            connect(pJCSound,SIGNAL(SoundEvent(double*,double*,int)),this,SLOT(Update(double*,double*,int)),Qt::DirectConnection);//DirectConnection!!!

            //calculate upsample rate and buffer frames so callbacks run at about the same time
            SCA_ratechange=0;
            SCA_rate=((double)pJCSound->sampleRate)/((double)pJCSound_SCA->sampleRate);
            pJCSound_SCA->bufferFrames=1.0*((double)pJCSound->bufferFrames)/SCA_rate;// 1.0 --> same 0.5 --> twice as often, etc

            //setup shared cycle buffer for enough space for 4 calls from the fast callback
            //SCA_buffer is a mono signal buffer
            SCA_buffer.fill(0,4*pJCSound->bufferFrames);
            SCA_buf_ptr_head=0;
            SCA_buf_ptr_tail=SCA_buffer.size()/2;
            SCA_buffer_use_percentage=0.5;
            //

        }
        pJCSound->Active(Enabled);
        pJCSound_SCA->Active(SCA_enabled&&Enabled);
        scaPeak.zero();
        lPeak.zero();
        rPeak.zero();
        outPeak.zero();
        opusBufferUsagePeak.zero();

        if(SCA_opus&&pJCSound_SCA->IsActive())pOQPSKModulator->StartSpooling();
         else pOQPSKModulator->StopSpooling();
}

void JMPXEncoder::SetEnableStereo(bool enable)
{
	stereo=enable;
}

bool JMPXEncoder::GetEnableStereo()
{
    return stereo;
}

void JMPXEncoder::SetEnableRDS(bool enable)
{
    RDS_enabled=enable;
}

bool JMPXEncoder::GetEnableRDS()
{
    return RDS_enabled;
}

TSigStats* JMPXEncoder::GetSignalStats()
{
	return &sigstats;
}

TimeConstant JMPXEncoder::GetPreEmphasis()
{
    return (TimeConstant)lpreemp.GetTc();
}

void JMPXEncoder::SetPreEmphasis(TimeConstant timeconst)
{
    lpreemp.SetTc(timeconst);
    rpreemp.SetTc(timeconst);
    pFMModulator->SetTc(timeconst);
}

//WARING called from another thread for speed reasons.
//There are some thread safty concerns here but nothing catastrophic should happen
void JMPXEncoder::Update(double *DataIn,double *DataOut, int nFrames)
{

    callback_mutex.lock();

    //calculate SCA buffer usage
    SCA_buffer_use_percentage=((double)(SCA_buf_ptr_head-SCA_buf_ptr_tail))/((double)SCA_buffer.size());
    if(SCA_buffer_use_percentage<0)SCA_buffer_use_percentage=1.0+SCA_buffer_use_percentage;
    double rval;
	double lval;
    double sca_val=0;
    double rival;
    double lival;
    double oval;

    //rds generation
    rds->FIRFilterImpulses(nFrames);//  number of frames

    //low pass filter. about 16.5Khz is standard
    //fast fir
    stereofir.Update(DataIn,nFrames);

	if(!stereo)
	{
        for(int i=0;i<nFrames*2;i+=2)
        {

            pWaveTable->WTnextFrame();

            lval=DataIn[i];
            rval=DataIn[i+1];

            lval=lpreemp.Update(lval*0.5);
            rval=rpreemp.Update(rval*0.5);

            lival=lval*2.0;
            rival=rval*2.0;

            lval=clipper.Update(lval);
            rval=clipper.Update(rval);

            DataOut[i]=monolevel*0.608*(lval+rval);//sum at 0Hz

            if(RDS_enabled)DataOut[i]+=rdslevel*rdsbpf->Update_Single(pWaveTable->WTSin3Value()*rds->outputsignal[i/2]);//RDS at 57kHz

            //SCA start
            if(SCA_enabled)
            {
                if(!SCA_opus)
                {
                    //SCA input
                    //upsample from 48000 to 192000
                    sca_val=0;
                    SCA_ratechange+=1.0;
                    if(SCA_ratechange>=SCA_rate)
                    {
                        SCA_ratechange-=SCA_rate;
                        SCA_ratechange+=0.1*(SCA_buffer_use_percentage-0.5);//adjust phase of sampleing so as to keep asynchronous sound cards synchronised

                        //check for under run
                        if(SCA_buf_ptr_head==SCA_buf_ptr_tail)
                        {
                            qDebug()<<"SCA input UnderRun";
                            //this should only happen when the buffer is spooling up
                            //set tail to be as far away as posible from the head
                            SCA_buffer.fill(0);
                            SCA_buf_ptr_tail=SCA_buf_ptr_head+SCA_buffer.size()/2;
                            SCA_buf_ptr_tail%=SCA_buffer.size();
                        }

                        //take sample (left and right have been summed in slow callback)
                        sca_val=SCA_buffer[SCA_buf_ptr_tail];
                        SCA_buf_ptr_tail++;SCA_buf_ptr_tail%=SCA_buffer.size();
                    }

                    //Modulate SCA signal
                    DataOut[i]+=SCA_Level*pFMModulator->update(sca_val);
                } else DataOut[i]+=SCA_Level*pOQPSKModulator->update();
            }
            //SCA end

            // add white noise hack
            if(noiselevel>0)
            {
                const double mean = 0.0;
                const double stddev = noiselevel;
                std::normal_distribution<double> dist(mean, stddev);
                DataOut[i]+=dist(generator);
            }

            oval=DataOut[i];
            if(compositeclipper)DataOut[i]=clipper.Update(DataOut[i]);

            if(fabs(sca_val)>scabigval)scabigval=fabs(sca_val);
            if(fabs(DataIn[i])>lbigval)lbigval=fabs(lival);
            if(fabs(DataIn[i+1])>rbigval)rbigval=fabs(rival);
            if(fabs(DataOut[i])>outbigval)outbigval=fabs(oval);
			decl++;
            if(!(decl%=1250))//about 154 times a second
			{
                if(!SCA_opus)opusBufferUsagePeak.zero();
                sigstats.outvol=outPeak.update(outbigval);
                sigstats.scavol=scaPeak.update(scabigval);
                sigstats.lvol=lPeak.update(lbigval);
                sigstats.rvol=rPeak.update(rbigval);
                sigstats.opusbufferuseagepercent=opusBufferUsagePeak.update(oqpskdataformatter.getBufferUsagePercentage());
				decl=0;
                scabigval=0.0;
                lbigval=0.0;
                rbigval=0.0;
                outbigval=0.0;
			}

            DataOut[i+1]=DataOut[i];

        }
        callback_mutex.unlock();
		return;
	}

    for(int i=0;i<nFrames*2;i+=2)
	{
        pWaveTable->WTnextFrame();

        lval=DataIn[i];
        rval=DataIn[i+1];

        lval=lpreemp.Update(lval*0.5);
        rval=rpreemp.Update(rval*0.5);

        lival=lval*2.0;
        rival=rval*2.0;

        lval=clipper.Update(lval);
        rval=clipper.Update(rval);

        DataOut[i]=monolevel*0.608*(lval+rval);//sum at 0Hz
        DataOut[i]+=level38k*0.608*(pWaveTable->WTSin2Value())*(lval-rval);//diff at 38kHz
        DataOut[i]+=pilotlevel*pWaveTable->WTSinValue();//19kHz pilot
        if(RDS_enabled)DataOut[i]+=rdslevel*rdsbpf->Update_Single(pWaveTable->WTSin3Value()*rds->outputsignal[i/2]);//RDS at 57kHz

        //SCA start
        if(SCA_enabled)
        {
            if(!SCA_opus)
            {
                //SCA input
                //upsample from 48000 to 192000
                sca_val=0;
                SCA_ratechange+=1.0;
                if(SCA_ratechange>=SCA_rate)
                {
                    SCA_ratechange-=SCA_rate;
                    SCA_ratechange+=0.1*(SCA_buffer_use_percentage-0.5);//adjust phase of sampleing so as to keep asynchronous sound cards synchronised

                    //check for under run
                    if(SCA_buf_ptr_head==SCA_buf_ptr_tail)
                    {
                        qDebug()<<"SCA input UnderRun";
                        //this should only happen when the buffer is spooling up
                        //set tail to be as far away as posible from the head
                        SCA_buffer.fill(0);
                        SCA_buf_ptr_tail=SCA_buf_ptr_head+SCA_buffer.size()/2;
                        SCA_buf_ptr_tail%=SCA_buffer.size();
                    }

                    //take sample (left and right have been summed in slow callback)
                    sca_val=SCA_buffer[SCA_buf_ptr_tail];
                    SCA_buf_ptr_tail++;SCA_buf_ptr_tail%=SCA_buffer.size();
                }

                //Modulate SCA signal
                DataOut[i]+=SCA_Level*pFMModulator->update(sca_val);
            } else DataOut[i]+=SCA_Level*pOQPSKModulator->update();
        }
        //SCA end

        // add white noise hack
        if(noiselevel>0)
        {
            const double mean = 0.0;
            const double stddev = noiselevel;
            std::normal_distribution<double> dist(mean, stddev);
            DataOut[i]+=dist(generator);
        }

        double oval=DataOut[i];
        if(compositeclipper)DataOut[i]=clipper.Update(DataOut[i]);

        if(fabs(sca_val)>scabigval)scabigval=fabs(sca_val);
        if(fabs(DataIn[i])>lbigval)lbigval=fabs(lival);
        if(fabs(DataIn[i+1])>rbigval)rbigval=fabs(rival);
        if(fabs(DataOut[i])>outbigval)outbigval=fabs(oval);

		decl++;
        if(!(decl%=1250))//about 154 times a second
		{
            if(!SCA_opus)opusBufferUsagePeak.zero();
            sigstats.outvol=outPeak.update(outbigval);
            sigstats.scavol=scaPeak.update(scabigval);
            sigstats.lvol=lPeak.update(lbigval);
            sigstats.rvol=rPeak.update(rbigval);
            sigstats.opusbufferuseagepercent=opusBufferUsagePeak.update(oqpskdataformatter.getBufferUsagePercentage());
			decl=0;
            scabigval=0.0;
            lbigval=0.0;
            rbigval=0.0;
            outbigval=0.0;
        }

        DataOut[i+1]=DataOut[i];
    }
    callback_mutex.unlock();


}

//opusSCA input callback
//WARING called from another thread for speed reasons.
void JMPXEncoder::Update_opusSCA(qint16 *DataIn, qint16 *DataOut, int nFrames)
{
    Q_UNUSED(DataOut);

    callback_mutex.lock();

    double sca_val=0;

    static int in_ptr=0;
    for(int i=0;i<nFrames*CHANNELS;i++)
    {

        sca_val=((double)DataIn[i])/32767.0;
        if(fabs(sca_val)>scabigval)scabigval=fabs(sca_val);

        if(in_ptr%2==i%2)//just incase something goes wrong with L/R sync
        {
            i++;
            if(i>=nFrames*CHANNELS)break;
        }

        in[in_ptr]=DataIn[i];
        in_ptr++;in_ptr%=FRAME_SIZE*CHANNELS;
        if(!in_ptr)
        {

            nbBytes = opus_encode(encoder, in, FRAME_SIZE, cbits, MAX_PACKET_SIZE);
            if(!oqpskdataformatter.pushPacket(DataFormatter::PacketType_OPUS,cbits,nbBytes))
            {
                qDebug()<<"packet buffer overflow";
            }

        }


    }

    callback_mutex.unlock();

}

//SCA input callback
//WARING called from another thread for speed reasons.
void JMPXEncoder::Update_SCA(double *DataIn, double *DataOut, int nFrames)
{
    Q_UNUSED(DataOut);

    callback_mutex.lock();
    for(int i=0;i<nFrames*2;i+=2)
    {
        SCA_buffer[SCA_buf_ptr_head]=(DataIn[i]+DataIn[i+1])*SCA_rate;//sum left and right channels
        SCA_buf_ptr_head++;SCA_buf_ptr_head%=SCA_buffer.size();
        if(SCA_buf_ptr_head==SCA_buf_ptr_tail)
        {
            qDebug()<<"SCA input Overflow";
            //this should only happen when the buffer is spooling up
            //set tail to be as far away as posible from the head
            SCA_buffer.fill(0);
            SCA_buf_ptr_tail=SCA_buf_ptr_head+SCA_buffer.size()/2;
            SCA_buf_ptr_tail%=SCA_buffer.size();
        }
    }
    callback_mutex.unlock();
}

//class factory
JMPXInterface* createObject(QObject *parent)
{
    return new JMPXEncoder(parent);
}

