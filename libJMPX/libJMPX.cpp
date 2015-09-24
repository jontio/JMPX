#include "libJMPX.h"

#include <QObject>
#include <QDebug>

#include <QFile>

JMPXEncoder::JMPXEncoder(QObject *parent):
    JMPXInterface(parent),
	pTDspGen( new TDspGen(&ASetGen)),
	pWaveTable( new WaveTable(pTDspGen.get()))
{
	stereo=true;
	decl=0;
	lbigval=0.0l;
	rbigval=0.0l;
	outbigval=0.0l;
	sigstats.lvol=0.0l;
	sigstats.rvol=0.0l;
	sigstats.outvol=0.0l;

    pJCSound = new TJCSound(this);
    pJCSound->sampleRate=192000;
    pJCSound->bufferFrames=8096;

    //old method (slow fir)
    //Cof_16500Hz_192000bps_119taps lpfc;
    //ltfir.Cof=lpfc.Cof;
    //rtfir.Cof =lpfc.Cof;

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

            lpreemp.SetTc(PreEmphasis::WORLD);
            rpreemp.SetTc(PreEmphasis::WORLD);

            connect(pJCSound,SIGNAL(SoundEvent(double*,double*,int)),this,SLOT(Update(double*,double*,int)),Qt::DirectConnection);//DirectConnection!!!
        }
        pJCSound->Active(Enabled);
}

void JMPXEncoder::EnableStereo(bool enable)
{
	stereo=enable;
}

TSigStats* JMPXEncoder::GetSignalStats()
{
	return &sigstats;
}

void JMPXEncoder::SetPreEmphasis(TimeConstant timeconst)
{
	lpreemp.SetTc((PreEmphasis::TimeConstant)timeconst);
	rpreemp.SetTc((PreEmphasis::TimeConstant)timeconst);
}

//WARING called from another thread for speed reasons.
//There are some thread safty concerns here but nothing catastrophic should happen
void JMPXEncoder::Update(double *DataIn,double *DataOut, int Size)
{

    double rval;
	double lval;

    //low pass filter first about 16.5Khz is standard

    //old method (slow fir)
    //ltfir.UpdateInterleavedEven(DataIn,Size);
    //rtfir.UpdateInterleavedOdd(DataIn,Size);

    //new method (fast fir)
    stereofir.Update(DataIn,Size);

	if(!stereo)
	{
		for(int i=0;i<(Size-1);i+=2)
		{	

            lval=DataIn[i];
            rval=DataIn[i+1];

            lval=lpreemp.Update(lval*0.5);
            rval=rpreemp.Update(rval*0.5);

            lval=clipper.Update(lval);
            rval=clipper.Update(rval);

			DataOut[i]=0.5*(lval+rval);
			DataOut[i+1]=DataOut[i];

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

		DataOut[i]=0.45*(lval+rval);
        DataOut[i]+=0.09*pWaveTable->WTSinValue();
        DataOut[i]+=0.45*(pWaveTable->WTSin2Value())*(lval-rval);

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

