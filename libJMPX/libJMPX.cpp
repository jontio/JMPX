#include "libJMPX.h"
#include <QDebug>

JMPXEncoder::JMPXEncoder(QObject *parent):
    JMPXInterface(parent),
	pTDspGen( new TDspGen(&ASetGen)),
    pWaveTable( new WaveTable(pTDspGen.get())),
    pFMModulator( new FMModulator(&ASetGen))
{
	stereo=true;
    RDS_enabled=true;
    SCA_enabled=true;
	decl=0;
    scabigval=0.0;
    lbigval=0.0;
    rbigval=0.0;
    outbigval=0.0;
    sigstats.scavol=0.0;
    sigstats.lvol=0.0;
    sigstats.rvol=0.0;
    sigstats.outvol=0.0;

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

    rdsbpf = new JFastFIRFilter;
    rdsbpf->setKernel(JFilterDesign::BandPassHanning(57000-2400,57000+2400,pJCSound->sampleRate,512-1));


}

JMPXEncoder::~JMPXEncoder()
{
        pJCSound->Active(false);
        pJCSound_SCA->Active(false);
        delete rdsbpf;
}

void JMPXEncoder::Active(bool Enabled)
{
        if(Enabled==pJCSound->IsActive())return;
        if(Enabled)
        {
            disconnect(pJCSound,SIGNAL(SoundEvent(double*,double*,int)),this,SLOT(Update(double*,double*,int)));
            disconnect(pJCSound_SCA,SIGNAL(SoundEvent(double*,double*,int)),this,SLOT(Update_SCA(double*,double*,int)));

            ASetGen.SampleRate=pJCSound->sampleRate;
            ASetGen.Freq=19000.0;

            pTDspGen->ResetSettings();
            pWaveTable->RefreshSettings();

            //SCA modulator setup
            pFMModulator->RefreshSettings(SCA_CarrierFrequency,SCA_MaxInputFrequency,SCA_MaxDeviation);

            rds->reset();

            rt_dynamic.clear();
            rds->set_rt(rt_default);

            //4.8khz rds bpf
            rdsbpf->setKernel(JFilterDesign::BandPassHanning(57000-2400,57000+2400,pJCSound->sampleRate,512-1));

            connect(pJCSound,SIGNAL(SoundEvent(double*,double*,int)),this,SLOT(Update(double*,double*,int)),Qt::DirectConnection);//DirectConnection!!!
            connect(pJCSound_SCA,SIGNAL(SoundEvent(double*,double*,int)),this,SLOT(Update_SCA(double*,double*,int)),Qt::DirectConnection);//DirectConnection!!!

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
void JMPXEncoder::Update(double *DataIn,double *DataOut, int Size)
{

    if(Size<2)return;

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
    rds->FIRFilterImpulses(Size/2);// div by 2 cos Size is the number of sample left and right so div by 2 --> number of mono samples

    //low pass filter. about 16.5Khz is standard
    //fast fir
    stereofir.Update(DataIn,Size);

	if(!stereo)
	{
		for(int i=0;i<(Size-1);i+=2)
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
            }
            //SCA end

            oval=DataOut[i];
            if(compositeclipper)DataOut[i]=clipper.Update(DataOut[i]);

            if(fabs(sca_val)>scabigval)scabigval=fabs(sca_val);
            if(fabs(DataIn[i])>lbigval)lbigval=fabs(lival);
            if(fabs(DataIn[i+1])>rbigval)rbigval=fabs(rival);
            if(fabs(DataOut[i])>outbigval)outbigval=fabs(oval);
			decl++;
            if(!(decl%=1250))//about 154 times a second
			{
                sigstats.outvol=outPeak.update(outbigval);
                sigstats.scavol=scaPeak.update(scabigval);
                sigstats.lvol=lPeak.update(lbigval);
                sigstats.rvol=rPeak.update(rbigval);
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

    for(int i=0;i<(Size-1);i+=2)
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
            //SCA input
            //upsample from 48000 to 192000
            sca_val=0;
            SCA_ratechange+=1.0;
            if(SCA_ratechange>=SCA_rate)
            {
                SCA_ratechange-=SCA_rate;
                SCA_ratechange+=0.2*(SCA_buffer_use_percentage-0.5);//adjust phase of sampleing so as to keep asynchronous sound cards synchronised

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
        }
        //SCA end

        double oval=DataOut[i];
        if(compositeclipper)DataOut[i]=clipper.Update(DataOut[i]);

        if(fabs(sca_val)>scabigval)scabigval=fabs(sca_val);
        if(fabs(DataIn[i])>lbigval)lbigval=fabs(lival);
        if(fabs(DataIn[i+1])>rbigval)rbigval=fabs(rival);
        if(fabs(DataOut[i])>outbigval)outbigval=fabs(oval);

		decl++;
        if(!(decl%=1250))//about 154 times a second
		{
            sigstats.outvol=outPeak.update(outbigval);
            sigstats.scavol=scaPeak.update(scabigval);
            sigstats.lvol=lPeak.update(lbigval);
            sigstats.rvol=rPeak.update(rbigval);
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


//SCA input callback
//WARING called from another thread for speed reasons.
void JMPXEncoder::Update_SCA(double *DataIn,double *DataOut, int Size)
{
    Q_UNUSED(DataOut);
    callback_mutex.lock();
    for(int i=0;i<Size;i+=2)
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

