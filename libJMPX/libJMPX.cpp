#include "libJMPX.h"
#include <QDebug>

JMPXEncoder::JMPXEncoder(QObject *parent):
    JMPXInterface(parent),
	pTDspGen( new TDspGen(&ASetGen)),
	pWaveTable( new WaveTable(pTDspGen.get()))
{
	stereo=true;
    RDS_enabled=true;
	decl=0;
    lbigval=0.0;
    rbigval=0.0;
    outbigval=0.0;
    sigstats.lvol=0.0;
    sigstats.rvol=0.0;
    sigstats.outvol=0.0;

    pJCSound = new TJCSound(this);
    pJCSound->sampleRate=192000;
    pJCSound->bufferFrames=8096;

    rds = new RDS(this);

    compositeclipper=true;
    monolevel=0.9;
    level38k=1.0;
    pilotlevel=0.07;
    rdslevel=0.06;

}

JMPXEncoder::~JMPXEncoder()
{
        pJCSound->Active(false);
}

void JMPXEncoder::Active(bool Enabled)
{
        if(Enabled==pJCSound->IsActive())return;
        if(Enabled)
        {
            disconnect(pJCSound,SIGNAL(SoundEvent(double*,double*,int)),this,SLOT(Update(double*,double*,int)));

            ASetGen.SampleRate=pJCSound->sampleRate;
            ASetGen.Freq=19000.0;

            pTDspGen->ResetSettings();
            pWaveTable->RefreshSettings();
            rds->reset();

            rt_dynamic.clear();
            rds->set_rt(rt_default);

            connect(pJCSound,SIGNAL(SoundEvent(double*,double*,int)),this,SLOT(Update(double*,double*,int)),Qt::DirectConnection);//DirectConnection!!!
        }
        pJCSound->Active(Enabled);
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
}

//WARING called from another thread for speed reasons.
//There are some thread safty concerns here but nothing catastrophic should happen
void JMPXEncoder::Update(double *DataIn,double *DataOut, int Size)
{

    if(Size<2)return;

    double rval;
	double lval;

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

            lval=clipper.Update(lval);
            rval=clipper.Update(rval);

            DataOut[i]=monolevel*0.608*(lval+rval);//sum at 0Hz
            if(RDS_enabled)DataOut[i]+=rdslevel*pWaveTable->WTSin3Value()*rds->outputsignal[i/2];//RDS at 57kHz
            if(compositeclipper)DataOut[i]=clipper.Update(DataOut[i]);

            if(fabs(DataIn[i])>lbigval)lbigval=fabs(DataIn[i]);
            if(fabs(DataIn[i+1])>rbigval)rbigval=fabs(DataIn[i+1]);
			if(fabs(DataOut[i])>outbigval)outbigval=fabs(DataOut[i]);
			decl++;
			if(!(decl%=5000))
			{
                sigstats.lvol=(1-0.2)*sigstats.lvol+(0.2)*lbigval;
                sigstats.rvol=(1-0.2)*sigstats.rvol+(0.2)*rbigval;
                sigstats.outvol=(1-0.2)*sigstats.outvol+(0.2)*outbigval;
				decl=0;
                lbigval=0.0;
                rbigval=0.0;
                outbigval=0.0;
			}

            DataOut[i+1]=DataOut[i];

		}
		return;
	}

    for(int i=0;i<(Size-1);i+=2)
	{
        pWaveTable->WTnextFrame();

        lval=DataIn[i];
        rval=DataIn[i+1];

        lval=lpreemp.Update(lval*0.5);
        rval=rpreemp.Update(rval*0.5);

        lval=clipper.Update(lval);
        rval=clipper.Update(rval);

        DataOut[i]=monolevel*0.608*(lval+rval);//sum at 0Hz
        DataOut[i]+=level38k*0.608*(pWaveTable->WTSin2Value())*(lval-rval);//diff at 38kHz
        DataOut[i]+=pilotlevel*pWaveTable->WTSinValue();//19kHz pilot
        if(RDS_enabled)DataOut[i]+=rdslevel*pWaveTable->WTSin3Value()*rds->outputsignal[i/2];//RDS at 57kHz
        if(compositeclipper)DataOut[i]=clipper.Update(DataOut[i]);

        if(fabs(DataIn[i])>lbigval)lbigval=fabs(DataIn[i]);
		if(fabs(DataIn[i+1])>rbigval)rbigval=fabs(DataIn[i+1]);
		if(fabs(DataOut[i])>outbigval)outbigval=fabs(DataOut[i]);
		decl++;
		if(!(decl%=5000))
		{
            sigstats.lvol=(1-0.2)*sigstats.lvol+(0.2)*lbigval;
            sigstats.rvol=(1-0.2)*sigstats.rvol+(0.2)*rbigval;
            sigstats.outvol=(1-0.2)*sigstats.outvol+(0.2)*outbigval;
			decl=0;
            lbigval=0.0;
            rbigval=0.0;
            outbigval=0.0;
		}

        DataOut[i+1]=DataOut[i];
	}
}

//class factory
JMPXInterface* createObject(QObject *parent)
{
    return new JMPXEncoder(parent);
}

