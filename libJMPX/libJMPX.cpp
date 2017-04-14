#include "libJMPX.h"
#include <QDebug>
#include <QtEndian>
#include <unistd.h>

#include <random>

#include <QTimer>

static __inline__ unsigned long long rdtsc(void)
{
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

JMPXEncoder::JMPXEncoder(QObject *parent):
    JMPXInterface(parent),
	pTDspGen( new TDspGen(&ASetGen)),
    pWaveTable( new WaveTable(pTDspGen.get(),19000)),
    pFMModulator( new FMModulator(pTDspGen.get()))
{
    _GotError=false;
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

    //reserve 2 threads for the SCA and the other one for the usual one
    tp=new QThreadPool(this);
    tp->setMaxThreadCount(2);

    pOQPSKModulator = new OQPSKModulator(pTDspGen.get(),this);

    pJCSound = new TJCSound(this);
    pJCSound->iParameters.nChannels=2;
    pJCSound->oParameters.nChannels=2;
    pJCSound->sampleRate=192000;//default setting
    pJCSound->bufferFrames=8096;
    pJCSound->options.streamName="JMPX";
    rds = new RDS(this);

    //SCA setup
    pJCSound_SCA = new TJCSound(this);
    pJCSound_SCA->iParameters.nChannels=2;//stero input
    pJCSound_SCA->oParameters.nChannels=0;//no output channels
    pJCSound_SCA->bufferFrames=8096;
    pJCSound_SCA->options.streamName="JMPX_SCA";
    pJCSound_SCA->sampleRate=48000;//default setting
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

    //stop SoundcardInOut threads and reset channel
    StopSoundcardInOut();

    //stop SCA threads and reset channel
    StopSCA_threads();

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
        _GotError=false;
        if(Enabled==pJCSound->IsActive())return;

        //disconnect signals/slots
        disconnect(pJCSound,SIGNAL(SoundEvent(double*,double*,int)),this,SLOT(SoundcardInOut_Callback(double*,double*,int)));
        disconnect(pJCSound_SCA,SIGNAL(SoundEvent(qint16*,qint16*,int)),this,SLOT(SCA_Callback(qint16*,qint16*,int)));

        //stop SoundcardInOut threads and reset channel
        StopSoundcardInOut();

        //stop SCA threads and reset channel
        StopSCA_threads();

        if(Enabled)
        {
            //here we dont need lock as other threads should be stoped

            //we should already have 2 threads set aside for us in the tp thread pool
#if QT_VERSION_MAJOR >=5 && QT_VERSION_MINOR>=4
            //start SoundcardInOut_dispatcher thread and wait till it has started
            do_SoundcardInOut_dispatcher_cancel=true;
            future_SoundcardInOut_dispatcher = QtConcurrent::run(tp,this,&JMPXEncoder::SoundcardInOut_dispatcher);
            while(do_SoundcardInOut_dispatcher_cancel)usleep(10000);

            //start SCA_dispatcher thread and wait till it has started
            do_SCA_dispatcher_cancel=true;
            future_SCA_dispatcher = QtConcurrent::run(tp,this,&JMPXEncoder::SCA_dispatcher);
            while(do_SCA_dispatcher_cancel)usleep(10000);
#else //this is because with old versions of Qt you cant change the threadpool. as an alternitive we could use a class derived from QRunnable with our thread pool
            //start SoundcardInOut_dispatcher thread and wait till it has started
            do_SoundcardInOut_dispatcher_cancel=true;
            future_SoundcardInOut_dispatcher = QtConcurrent::run(this,&JMPXEncoder::SoundcardInOut_dispatcher);
            usleep(300000);
            while(do_SoundcardInOut_dispatcher_cancel)
            {
                usleep(1000000);
                if(do_SoundcardInOut_dispatcher_cancel)QThreadPool::globalInstance()->setMaxThreadCount(QThreadPool::globalInstance()->maxThreadCount()+1);
            }

            //start SCA_dispatcher thread and wait till it has started
            do_SCA_dispatcher_cancel=true;
            future_SCA_dispatcher = QtConcurrent::run(this,&JMPXEncoder::SCA_dispatcher);
            usleep(300000);
            while(do_SCA_dispatcher_cancel)
            {
                usleep(1000000);
                if(do_SCA_dispatcher_cancel)QThreadPool::globalInstance()->setMaxThreadCount(QThreadPool::globalInstance()->maxThreadCount()+1);
            }
#endif

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

            //clear OQPSK buffer
            oqpskdataformatter.clearBuffer();

            //SCA uses 16 bit int
            pJCSound_SCA->audioformat=RTAUDIO_SINT16;
            connect(pJCSound_SCA,SIGNAL(SoundEvent(qint16*,qint16*,int)),this,SLOT(SCA_Callback(qint16*,qint16*,int)),Qt::DirectConnection);//DirectConnection!!!

            //Usual sound card uses double
            connect(pJCSound,SIGNAL(SoundEvent(double*,double*,int)),this,SLOT(SoundcardInOut_Callback(double*,double*,int)),Qt::DirectConnection);//DirectConnection!!!

            //stop SCA working untill we know how big this buffer should be
            SCA_buffer.clear();

            //this is what we would like but there is no garentee this is what we will get once the soundcard has started
            SCA_ratechange=0;
            SCA_rate=((double)pJCSound->sampleRate)/48000.0;//SCA rate must be 48k as thats the fastest that opus can handle  //
            pJCSound_SCA->bufferFrames=1.0*((double)pJCSound->bufferFrames)/((((double)pJCSound->sampleRate)/((double)pJCSound_SCA->sampleRate)));// 1.0 --> same 0.5 --> twice as often, etc

        }

        try
        {
            pJCSound->Active(Enabled);
            if(pJCSound->GotError)throw 1;
            pJCSound_SCA->Active(SCA_enabled&&Enabled);
            if(pJCSound_SCA->GotError)throw 2;
        }

        catch(int e)
        {
            _GotError=true;
            switch(e)
            {
            case 1:
                LastErrorMessage=pJCSound->LastErrorMessage;
                break;
            case 2:
                LastErrorMessage="SCA input : "+pJCSound_SCA->LastErrorMessage;
                break;
            default:
                LastErrorMessage="unknowen";
            }
            pJCSound->GotError=false;
            pJCSound->Active(false);
            pJCSound->GotError=false;
            pJCSound_SCA->GotError=false;
            pJCSound->Active(false);
            pJCSound_SCA->GotError=false;
        }

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

//stops thread 1 and 2
void JMPXEncoder::StopSoundcardInOut()
{
    //stop sound thread and dispatcher thread
    //reset the comm channel between them
    qDebug()<<"stopping SoundcardInOut callback";
    pJCSound->Active(false);
    qDebug()<<"SoundcardInOut callback stoped";
    if(!future_SoundcardInOut_dispatcher.isFinished())
    {
            qDebug()<<"stopping SoundcardInOut_dispatcher";
            buffers_mut.lock();
            do_SoundcardInOut_dispatcher_cancel=true;
            buffers_process.wakeAll();
            buffers_mut.unlock();
            future_SoundcardInOut_dispatcher.waitForFinished();
            do_SoundcardInOut_dispatcher_cancel=false;
            qDebug()<<"SoundcardInOut_dispatcher stoped";
    }
    buffers_head_ptr_in=0;
    buffers_tail_ptr_in=0;
    buffers_used_in=0;
    buffers_head_ptr_out=0;
    buffers_tail_ptr_out=0;
    buffers_used_out=0;
    spooling=false;
    for(int i=0;i<N_BUFFERS;i++)
    {
        buffers_in[i].assign(buffers_in[i].size(),0.0);
        buffers_out[i].assign(buffers_in[i].size(),0.0);
    }

}

//thread 1 (192k sound in/out processing thread)
void JMPXEncoder::SoundcardInOut_dispatcher()
{
    do_SoundcardInOut_dispatcher_cancel=false;
    qDebug()<<"SoundcardInOut_dispatcher started";
    while(true)
    {

        //if we have no data to process then wait
        buffers_mut.lock();
        if(buffers_used_in<1)
        {
            buffers_process.wait(&buffers_mut);
        }
        buffers_mut.unlock();

        //check if reason for waking is to cancel
        if(do_SoundcardInOut_dispatcher_cancel)break;

        //cycle buffers
        buffers_tail_ptr_in%=N_BUFFERS;
        buffers_head_ptr_out%=N_BUFFERS;

        //get size
        int units=buffers_in[buffers_tail_ptr_in].size();
        int nFrames=units/2;

        //make sure buffers are of the right size
        buffers_in[buffers_tail_ptr_in].resize(units,0);
        buffers_out[buffers_head_ptr_out].resize(units,0);

        //create buffer ptr
        double *DataIn=buffers_in[buffers_tail_ptr_in].data();
        double *DataOut=buffers_out[buffers_head_ptr_out].data();

        Update(DataIn,DataOut,nFrames);

        //goto next buffer
        ++buffers_tail_ptr_in;
        ++buffers_head_ptr_out;

        buffers_mut.lock();
        --buffers_used_in;
        ++buffers_used_out;
        buffers_mut.unlock();

    }
    qDebug()<<"SoundcardInOut_dispatcher finished";
}

//thread 2 (192k sound in/out transfer thread)
void JMPXEncoder::SoundcardInOut_Callback(double *DataIn,double *DataOut, int nFrames)
{

    //return if in a cancelling state
    if(do_SoundcardInOut_dispatcher_cancel)return;

    //if there is no room to store the incoming data then ????
    buffers_mut.lock();
    if(buffers_used_in>=N_BUFFERS)
    {
        //??? maybe just panic and return and hope at a later time we will have some room
        buffers_mut.unlock();
        qDebug()<<"SoundcardOut_Callback overrun";
        return;
    }
    buffers_mut.unlock();

    //size of this buffer and every other buffer. this needs to be the same
    int units=2*nFrames;

    //if there is no data for us to return then ????
    buffers_mut.lock();
    if(buffers_used_out<1)
    {
        //??? either we are spooling up or we have an underrun
        //??? set the spooling flag to tell this fuction to return garbarge
        qDebug()<<"SoundcardOut_Callback underrun";
        spooling=true;
    }
    if(spooling&&(buffers_used_out+buffers_used_in)>=N_BUFFERS)spooling=false;
    buffers_mut.unlock();

//    if(spooling)qDebug()<<"spooling usual";

    //cycle buffers
    buffers_head_ptr_in%=N_BUFFERS;
    buffers_tail_ptr_out%=N_BUFFERS;

    //make sure buffers are of the right size
    buffers_in[buffers_head_ptr_in].resize(units,0);
    buffers_out[buffers_tail_ptr_out].resize(units,0);

    double *buffptr_in=buffers_in[buffers_head_ptr_in].data();
    double *buffptr_out=buffers_out[buffers_tail_ptr_out].data();

    //compy memory
    memcpy(buffptr_in,DataIn, sizeof(double)*units);
    memcpy(DataOut,buffptr_out, sizeof(double)*units);

    //goto next buffer
    ++buffers_head_ptr_in;
    if(!spooling)++buffers_tail_ptr_out;

    //tell dispatcher thread to process this and any other callback blocks
    buffers_mut.lock();
    ++buffers_used_in;
    if(!spooling)--buffers_used_out;

//    if((!spooling)&&(buffers_used_out<(N_BUFFERS-1)))qDebug()<<"dual sound card thread caught a buffer. buffers_used_in=="<<buffers_used_in<<"buffers_used_out=="<<buffers_used_out<<" N_BUFFERS="<<N_BUFFERS;

    buffers_process.wakeAll();
    buffers_mut.unlock();

}

//stops thread 3 and 4
void JMPXEncoder::StopSCA_threads()
{
    //stop sound thread and dispatcher thread
    //reset the comm channel between them
    qDebug()<<"stopping SCA callback";
    pJCSound_SCA->Active(false);
    qDebug()<<"SCA callback stoped";
    if(!future_SCA_dispatcher.isFinished())
    {
            qDebug()<<"stopping SCA_dispatcher";
            buffers_mut_sca.lock();
            do_SCA_dispatcher_cancel=true;
            buffers_process_sca.wakeAll();
            buffers_mut_sca.unlock();
            future_SCA_dispatcher.waitForFinished();
            do_SCA_dispatcher_cancel=false;
            qDebug()<<"SCA_dispatcher stoped";
    }
    buffers_head_ptr_in_sca=0;
    buffers_tail_ptr_in_sca=0;
    buffers_used_in_sca=0;
    buffers_used_out_sca=0;
    spooling_sca=false;
    for(int i=0;i<N_BUFFERS;i++)
    {
        buffers_in_sca[i].assign(buffers_in_sca[i].size(),0);
    }

}

//thread 3 (SCA audio in processing thread)
void JMPXEncoder::SCA_dispatcher()
{
    do_SCA_dispatcher_cancel=false;
    qDebug()<<"SCA_dispatcher started";
    while(true)
    {

        //if we have no data to process then wait
        buffers_mut_sca.lock();
        if(buffers_used_in_sca<1)
        {
            buffers_process_sca.wait(&buffers_mut_sca);
        }
        buffers_mut_sca.unlock();

        //check if reason for waking is to cancel
        if(do_SCA_dispatcher_cancel)break;

        //cycle buffers
        buffers_tail_ptr_in_sca%=N_BUFFERS;

        //get size
        int units=buffers_in_sca[buffers_tail_ptr_in_sca].size();
        int nFrames=units/2;

        //make sure buffers are of the right size
        buffers_in_sca[buffers_tail_ptr_in_sca].resize(units,0);

        //create buffer ptr
        qint16 *DataIn=buffers_in_sca[buffers_tail_ptr_in_sca].data();
        qint16 *DataOut=NULL;//nothing comes back

        if(SCA_opus)Update_opusSCA(DataIn,DataOut,nFrames);
         else Update_SCA(DataIn,DataOut,nFrames);

        //goto next buffer
        ++buffers_tail_ptr_in_sca;

        buffers_mut_sca.lock();
        --buffers_used_in_sca;
        ++buffers_used_out_sca;
        buffers_mut_sca.unlock();

    }
    qDebug()<<"SCA_dispatcher finished";
}

//thread 4 (SCA audio in transfer thread)
void JMPXEncoder::SCA_Callback(qint16 *DataIn,qint16 *DataOut, int nFrames)
{

    Q_UNUSED(DataOut);

    //return if in a cancelling state
    if(do_SCA_dispatcher_cancel)return;

    //if there is no room to store the incoming data then ????
    buffers_mut_sca.lock();
    if(buffers_used_in_sca>=N_BUFFERS)
    {
        //??? maybe just panic and return and hope at a later time we will have some room
        buffers_mut_sca.unlock();
        qDebug()<<"SCA_Callback overrun";
        return;
    }
    buffers_mut_sca.unlock();

    //size of this buffer and every other buffer. this needs to be the same
    int units=2*nFrames;

    //if there is no data for us to return then ????
    buffers_mut_sca.lock();
    if(buffers_used_out_sca<1)
    {
        //??? either we are spooling up or we have an underrun
        //??? set the spooling flag to tell this fuction to return garbarge
        qDebug()<<"SCA_Callback underrun";
        spooling_sca=true;
    }
    if(spooling_sca&&(buffers_used_out_sca+buffers_used_in_sca)>=N_BUFFERS)spooling_sca=false;
    buffers_mut_sca.unlock();

//    if(spooling_sca)qDebug()<<"spooling_sca";

    //cycle buffers
    buffers_head_ptr_in_sca%=N_BUFFERS;

    //make sure buffers are of the right size
    buffers_in_sca[buffers_head_ptr_in_sca].resize(units,0);

    qint16 *buffptr_in=buffers_in_sca[buffers_head_ptr_in_sca].data();

    //compy memory
    memcpy(buffptr_in,DataIn, sizeof(qint16)*units);

    //goto next buffer
    ++buffers_head_ptr_in_sca;

    //tell dispatcher thread to process this and any other callback blocks
    buffers_mut_sca.lock();
    ++buffers_used_in_sca;
    if(!spooling_sca)--buffers_used_out_sca;

//    if((!spooling_sca)&&(buffers_used_out_sca<(N_BUFFERS-1)))qDebug()<<"dual SCA sound card thread caught a buffer. buffers_used_in=="<<buffers_used_in_sca<<"buffers_used_out=="<<buffers_used_out_sca<<" N_BUFFERS="<<N_BUFFERS;

    buffers_process_sca.wakeAll();
    buffers_mut_sca.unlock();
}

//part of thread 1 (called by dispatcher)
void JMPXEncoder::Update(double *DataIn,double *DataOut, int nFrames)
{

    callback_mutex.lock();

    //calculate SCA buffer usage
    if(!SCA_buffer.isEmpty())
    {
        SCA_buffer_use_percentage=((double)(SCA_buf_ptr_head-SCA_buf_ptr_tail))/((double)SCA_buffer.size());
        if(SCA_buffer_use_percentage<0)SCA_buffer_use_percentage=1.0+SCA_buffer_use_percentage;
    }
     else SCA_buffer_use_percentage=0;

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
            if((SCA_enabled)&&(!SCA_buffer.isEmpty()))
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
        if((SCA_enabled)&&(!SCA_buffer.isEmpty()))
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

//part of thread 3 (called by dispatcher)
void JMPXEncoder::Update_opusSCA(qint16 *DataIn, qint16 *DataOut, int nFrames)
{
    Q_UNUSED(DataOut);

    callback_mutex.lock();

    //this is not used by us directly but in case the user switches back to SCA. also SCA_buffer.size() can't be zero for us to work
    if(SCA_buffer.isEmpty())
    {

        //setup shared cycle buffer for enough space for 4 calls from the fast callback
        //SCA_buffer is a mono signal buffer
        SCA_buffer.fill(0,4*qMax((int)pJCSound_SCA->bufferFrames,1024));
        SCA_buf_ptr_head=0;
        SCA_buf_ptr_tail=SCA_buffer.size()/2;
        SCA_buffer_use_percentage=0.5;
        SCA_ratechange=0;
        SCA_rate=((double)pJCSound->sampleRate)/48000.0;//SCA rate must be 48k as thats the fastest that opus can handle  //
        //

    }

    double sca_val=0;

    //CHANNELS must be 2

    //down sampling to 48000 if needed. we should have a LPF if this is done but who is likly to be sending frequencies above 24kHz
    //todo add fir LPF
    int stepper=1;
    if(pJCSound_SCA->sampleRate==96000){stepper=2;if(nFrames%2)qDebug()<<"cant devide SCA buffer size by 2!!";}
    if(pJCSound_SCA->sampleRate==192000){stepper=4;if(nFrames%4)qDebug()<<"cant devide SCA buffer size by 4!!";}

    static int in_ptr=0;
    for(int i=0;i<nFrames*CHANNELS;i+=(2*stepper))
    {

        sca_val=((double)DataIn[i])/32767.0;
        if(fabs(sca_val)>scabigval)scabigval=fabs(sca_val);
        sca_val=((double)DataIn[i+1])/32767.0;
        if(fabs(sca_val)>scabigval)scabigval=fabs(sca_val);

        if(in_ptr%2!=i%2)//just incase something goes wrong with L/R sync
        {
            qDebug()<<"LR stuffup"<<in_ptr<<i;
            in_ptr++;
            in_ptr%=FRAME_SIZE*CHANNELS;

        }

        in[in_ptr]=DataIn[i];
        in[in_ptr+1]=DataIn[i+1];
        in_ptr+=2;in_ptr%=FRAME_SIZE*CHANNELS;
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

//part of thread 3 (called by dispatcher)
void JMPXEncoder::Update_SCA(qint16 *DataIn, qint16 *DataOut, int nFrames)
{
    Q_UNUSED(DataOut);

    callback_mutex.lock();

    if(SCA_buffer.isEmpty())
    {

        //setup shared cycle buffer for enough space for 4 calls from the fast callback
        //SCA_buffer is a mono signal buffer
        SCA_buffer.fill(0,4*qMax((int)pJCSound_SCA->bufferFrames,1024));
        SCA_buf_ptr_head=0;
        SCA_buf_ptr_tail=SCA_buffer.size()/2;
        SCA_buffer_use_percentage=0.5;
        SCA_ratechange=0;
        SCA_rate=((double)pJCSound->sampleRate)/48000.0;//SCA rate must be 48k as thats the fastest that opus can handle  //
        //

    }

    //down sampling to 48000 if needed. we should have a LPF if this is done but who is likly to be sending frequencies above 24kHz
    //todo add fir LPF
    int stepper=1;
    if(pJCSound_SCA->sampleRate==96000){stepper=2;if(nFrames%2)qDebug()<<"cant devide SCA buffer size by 2!!";}
    if(pJCSound_SCA->sampleRate==192000){stepper=4;if(nFrames%4)qDebug()<<"cant devide SCA buffer size by 4!!";}

    for(int i=0;i<nFrames*2;i+=(2*stepper))
    {
        SCA_buffer[SCA_buf_ptr_head]=(((double)DataIn[i])/32767.0+((double)DataIn[i+1])/32767.0)*SCA_rate;//sum left and right channels
        SCA_buf_ptr_head++;if(SCA_buf_ptr_head>=SCA_buffer.size())SCA_buf_ptr_head=0;
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

