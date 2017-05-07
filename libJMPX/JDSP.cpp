//---------------------------------------------------------------------------


#include "JDSP.h"

//---------------------------------------------------------------------------

SymbolPointer::SymbolPointer()
{
    setFreq(2375,192000);
}

void  SymbolPointer::setFreq(double freqHZ,double samplerate)
{
        symbolstep=(freqHZ)*((double)WTSIZE)/(samplerate);
        symbolptr=0;
}

//----------------------


 WaveTable::WaveTable(TDspGen *_pDspGen, double _Freq) :
        pDspGen(_pDspGen)
{
        RefreshSettings(_Freq);
}



WaveTable::~WaveTable()
{
 //
}

bool  WaveTable::IfPassesPointNextTime()
{
    FractionOfSampleItPassesBy=WTptr+(WTstep-WTSIZE);
    if(FractionOfSampleItPassesBy<0)return false;
    FractionOfSampleItPassesBy/=WTstep;
    return true;
}

void  WaveTable::RefreshSettings(double _Freq)
{
    Freq=_Freq;
    WTstep=(Freq)*(WTSIZE)/((double)(pDspGen->SampleRate));
    WTptr=0;
    intWTptr=0;
}

void  WaveTable::SetFreq(double _freq)
{
    Freq=_freq;
    if(Freq<0)Freq=0;
    WTstep=(Freq)*((double)WTSIZE)/((double)(pDspGen->SampleRate));
}

double WaveTable::GetFreqHz()
{
    return Freq;
}

void  WaveTable::IncreseFreqHz(double freq_hz)
{
    freq_hz+=Freq;
    SetFreq(freq_hz);
}

void WaveTable::WTnextFrame()
{
	WTptr+=WTstep;
	while(!signbit(WTptr-(double)WTSIZE))WTptr-=(double)(WTSIZE);
	if(signbit(WTptr))WTptr=0;
    intWTptr=(int)WTptr;
}

void WaveTable::WTnextFrame(double offset_in_hz)
{
    WTstep=(offset_in_hz+Freq)*(WTSIZE)/((double)(pDspGen->SampleRate));
    WTnextFrame();
}

double   WaveTable::WTSinValue()
{
#ifndef WT_Interpolate
    return pDspGen->SinWT[intWTptr];
#endif

#ifdef WT_Interpolate
    //simple interpolation coule be used if wanted
    double prompt=pDspGen->SinWT[intWTptr];
    int i=intWTptr+1;i%=WTSIZE;
    double late=pDspGen->SinWT[i];
    double rem=WTptr-((double)intWTptr);
    return (prompt*(1.0-rem)+late*rem);
#endif
}

cpx_type WaveTable::WTCISValue()
{
#ifndef WT_Interpolate
    int tintWTptr=intWTptr+WTSIZE/4;
    return pDspGen->CISWT[tintWTptr];
#endif

#ifdef WT_Interpolate
    //simple interpolation coule be used if wanted
    double tWTptr=WTptr+((double)WTSIZE)/4.0;
    while(!signbit(tWTptr-(double)WTSIZE))tWTptr-=(double)(WTSIZE);
    if(signbit(tWTptr))tWTptr=0;
    double tintWTptr=(int)tWTptr;
    cpx_type prompt=pDspGen->CISWT[tintWTptr];
    int i=tintWTptr+1;i%=WTSIZE;
    cpx_type late=pDspGen->CISWT[i];
    double rem=tWTptr-((double)tintWTptr);
    return (prompt*(1.0-rem)+late*rem);
#endif
}

double   WaveTable::WTSin2Value()
{
    double tmpwtptr=WTptr*2.0;
    while(!signbit(tmpwtptr-(double)WTSIZE))tmpwtptr-=(double)(WTSIZE);
    if(signbit(tmpwtptr))tmpwtptr=0;
#ifndef WT_Interpolate
    return pDspGen->SinWT[(int)tmpwtptr];
#endif

#ifdef WT_Interpolate
    //simple interpolation coule be used if wanted
    int tintWTptr=(int)tmpwtptr;
    double prompt=pDspGen->SinWT[tintWTptr];
    int i=tintWTptr+1;i%=WTSIZE;
    double late=pDspGen->SinWT[i];
    double rem=tmpwtptr-((double)tintWTptr);
    return (prompt*(1.0-rem)+late*rem);
#endif
}

double   WaveTable::WTSin3Value()
{
    double tmpwtptr=WTptr*3.0;
    while(!signbit(tmpwtptr-(double)WTSIZE))tmpwtptr-=(double)(WTSIZE);
    if(signbit(tmpwtptr))tmpwtptr=0;
#ifndef WT_Interpolate
    return pDspGen->SinWT[(int)tmpwtptr];
#endif

#ifdef WT_Interpolate
    //simple interpolation coule be used if wanted
    int tintWTptr=(int)tmpwtptr;
    double prompt=pDspGen->SinWT[tintWTptr];
    int i=tintWTptr+1;i%=WTSIZE;
    double late=pDspGen->SinWT[i];
    double rem=tmpwtptr-((double)tintWTptr);
    return (prompt*(1.0-rem)+late*rem);
#endif
}

//----------

double sinc_normalized(double val)
{
    if (val==0)return 1.0;
    return (sin(M_PI*val)/(M_PI*val));
}

std::vector<kffsamp_t> JFilterDesign::LowPassHanning(double FrequencyCutOff, double SampleRate, int Length)
{
    std::vector<kffsamp_t> h;
    if(Length<1)return h;
    if(!(Length%2))Length++;
    int j=1;
    for(int i=(-(Length-1)/2);i<=((Length-1)/2);i++)
    {
        double w=0.5*(1.0-cos(2.0*M_PI*((double)j)/((double)(Length))));
        h.push_back(w*(2.0*FrequencyCutOff/SampleRate)*sinc_normalized(2.0*FrequencyCutOff*((double)i)/SampleRate));
        j++;
    }

    return h;

/* in matlab this function is
idx = (-(Length-1)/2:(Length-1)/2);
hideal = (2*FrequencyCutOff/SampleRate)*sinc(2*FrequencyCutOff*idx/SampleRate);
h = hanning(Length)' .* hideal;
*/

}

std::vector<kffsamp_t> JFilterDesign::HighPassHanning(double FrequencyCutOff, double SampleRate, int Length)
{
    std::vector<kffsamp_t> h;
    if(Length<1)return h;
    if(!(Length%2))Length++;

    std::vector<kffsamp_t> h1;
    std::vector<kffsamp_t> h2;
    h2.assign(Length,0);
    h2[(Length-1)/2]=1.0;

    h1=LowPassHanning(FrequencyCutOff,SampleRate,Length);
    if((h1.size()==(size_t)Length)&&(h2.size()==(size_t)Length))
    {
        for(int i=0;i<Length;i++)h.push_back(h2[i]-h1[i]);
    }

    return h;
}

std::vector<kffsamp_t> JFilterDesign::BandPassHanning(double LowFrequencyCutOff,double HighFrequencyCutOff, double SampleRate, int Length)
{
    std::vector<kffsamp_t> h;
    if(Length<1)return h;
    if(!(Length%2))Length++;

    std::vector<kffsamp_t> h1;
    std::vector<kffsamp_t> h2;

    h2=LowPassHanning(HighFrequencyCutOff,SampleRate,Length);
    h1=LowPassHanning(LowFrequencyCutOff,SampleRate,Length);

    if((h1.size()==(size_t)Length)&&(h2.size()==(size_t)Length))
    {
        for(int i=0;i<Length;i++)h.push_back(h2[i]-h1[i]);
    }

    return h;
}


//----------

FMModulator::FMModulator(TDspGen *_pDspGen) :
    pDspGen(_pDspGen),
    pWaveTableCarrier(new WaveTable(pDspGen,67500.0))
{
    fir = new JFastFIRFilter;
    outputbpf = new JFastFIRFilter;
    RefreshSettings(67500,7000,3500);
}

FMModulator::~FMModulator()
{
    delete fir;
    delete outputbpf;
}

double FMModulator::update(double &insignal)//say signal runs from -1 to 1
{

    //LP filter
    double signal=fir->Update_Single(insignal);

    //preemp
    signal=preemp.Update(signal*0.5);

    //return the signal just before the clipper
    insignal=signal;

    //fm modulate and clip the signal if needed
    pWaveTableCarrier->WTnextFrame(clipper.Update(signal)*settings.max_deviation);

    //return filtered modulated carrier signal
    return outputbpf->Update_Single(pWaveTableCarrier->WTSinValue());
}

void FMModulator::RefreshSettings(double carrier_freq, double max_audio_input_frequency, double max_deviation, double _ouput_bandwidth)
{
    settings.carrier_freq=carrier_freq;
    settings.max_audio_input_frequency=max_audio_input_frequency;
    settings.max_deviation=max_deviation;

    ouput_bandwidth=_ouput_bandwidth;
    pWaveTableCarrier->RefreshSettings(carrier_freq);
    fir->setKernel(JFilterDesign::LowPassHanning(max_audio_input_frequency,pDspGen->SampleRate,1024-1));//1001));
    outputbpf->setKernel(JFilterDesign::BandPassHanning(carrier_freq-ouput_bandwidth/2.0,carrier_freq+ouput_bandwidth/2.0,pDspGen->SampleRate,1024-1));
}

//----------

TimeConstant PreEmphasis::GetTc()
{
    return timeconst;
}

void PreEmphasis::SetTc(TimeConstant _timeconst)
{
    timeconst=_timeconst;
    switch(timeconst)
	{
	case WORLD:
	    a[0] = 5.309858008l;
	    a[1] = -4.794606188l;
	    b[1] = 0.4847481783l;
		break;
	case USA:
        a[0] = 7.681633687l;
        a[1] = -7.170926091l;
        b[1] = 0.4892924010l;
		break;
	default:
        a[0] = 1.0l;
        a[1] = 0.0l;
        b[1] = 0.0l;
		break;
	}


	y[0]=0;
	y[1]=0;
	x[0]=0;
	x[1]=0;
}

PreEmphasis::PreEmphasis()
{
    SetTc(WORLD);
}

double PreEmphasis::Update(double val)
{
	x[1]=val;
	y[1]=a[0]*x[1]+a[1]*x[0]+b[1]*y[0];
	double retval=y[1];
	y[0]=y[1];
	x[0]=x[1];
	return retval;
}

//----------

Clipper::Clipper()
{
    SetCompressionPoint(0.85);
}

void Clipper::SetCompressionPoint(double point)
{
    if(point>0.99)point=0.99;
     else if(point<0.01)point=0.01;
	compressionpoint=point;
    double a=M_PI/(2.0*(1.0-compressionpoint));
	LookupTable.resize(6000);
	for(int i=0;i<(int)LookupTable.size();i++)
	{
        LookupTable[i]=compressionpoint+atan(a*(i*0.001))/a;
	}
}

double Clipper::Update(double val)
{
	if(fabs(val)<compressionpoint)return val;
    double inv=1.0;
    if(val<0.0)
	{
        inv=-1.0;
		val=-val;
	}

    double shiftmultval=(val-compressionpoint)*1000.0;
	int n=(int)(shiftmultval);
	int q=n+1;
	if(q>=(int)LookupTable.size())return inv;
	if(n<0)return compressionpoint*inv;
	val=LookupTable[n]+(shiftmultval-((double)n))*(LookupTable[q]-LookupTable[n]);
	return inv*val;
}

//---slow FIR

FIRFilter::FIRFilter()
{
	BufPtr=0;
}

double FIRFilter::Update(double val)
{
	if(Buf.size()!=Cof.size())
	{
		Buf.resize(Cof.size());
		BufPtr=0;
	}
	BufPtr%=Buf.size();
	Buf[BufPtr]=val;
    BufPtr++;BufPtr%=Buf.size();
    double retval=0.0;
	for(unsigned int i=0;i<Cof.size();i++)
	{
		retval+=Buf[BufPtr]*Cof[i];
		BufPtr++;BufPtr%=Buf.size();
    }
	return retval;
}

void FIRFilter::Update(double *data,int Size)
{

    BufPtr%=Buf.size();
    double val;
    unsigned int sz=Cof.size();

    for(int j=0;j<Size;j++)
    {
        Buf[BufPtr]=data[j];
        BufPtr++;BufPtr%=Buf.size();
        val=0;
        for(unsigned int i=0;i<sz;i++)
        {
            val+=Buf[BufPtr]*Cof[i];
            BufPtr++;BufPtr%=Buf.size();
        }
        data[j]=val;
    }

}

void FIRFilter::UpdateInterleavedEven(double *data,int Size)
{
    if(Buf.size()!=Cof.size())
    {
        Buf.resize(Cof.size());
        BufPtr=0;
    }
    BufPtr%=Buf.size();
    unsigned int i;
    unsigned int COfsz=Cof.size();
    int Bufsz=Buf.size();

    for(int j=0;j<Size;j+=2)
    {
        Buf[BufPtr]=data[j];
        BufPtr++;if(BufPtr==Bufsz)BufPtr=0;
        data[j]=0;
        for(i=0;i<COfsz;i++)
        {
            data[j]+=Buf[BufPtr]*Cof[i];
            BufPtr++;if(BufPtr==Bufsz)BufPtr=0;
        }
    }

}

void FIRFilter::UpdateInterleavedOdd(double *data,int Size)
{
    if(Buf.size()!=Cof.size())
    {
        Buf.resize(Cof.size());
        BufPtr=0;
    }
    BufPtr%=Buf.size();
    unsigned int i;
    unsigned int COfsz=Cof.size();
    int Bufsz=Buf.size();

    for(int j=1;j<Size;j+=2)
    {
        Buf[BufPtr]=data[j];
        BufPtr++;if(BufPtr==Bufsz)BufPtr=0;
        data[j]=0;
        for(i=0;i<COfsz;i++)
        {
            data[j]+=Buf[BufPtr]*Cof[i];
            BufPtr++;if(BufPtr==Bufsz)BufPtr=0;
        }
    }

}

//-----

 TDspGen::TDspGen(TSetGen *_pSetGen) :
        pSetGen(_pSetGen)
{
        SinWT.resize(WTSIZE);
        CISWT.resize(WTSIZE);
        int i;
        for(i=0;i<WTSIZE;i++)
        {
            SinWT[i]=(sin(2.0*M_PI*((double)i)/((double)WTSIZE)));
            CISWT[i]=cpx_type(cos(2.0*M_PI*((double)i)/((double)WTSIZE)),sin(2.0*M_PI*((double)i)/((double)WTSIZE)));
        }
        ResetSettings();
}

 TDspGen::~TDspGen()
{
//
}

void  TDspGen::ResetSettings()
{
        //load values
        SampleRate=pSetGen->SampleRate;
}

//---fast FIR

FastFIRFilter::FastFIRFilter(std::vector<kffsamp_t> imp_responce,size_t &_nfft)
{
    cfg=kiss_fastfir_alloc(imp_responce.data(),imp_responce.size(),&_nfft,0,0);
    nfft=_nfft;
    reset();
}

FastFIRFilter::FastFIRFilter(std::vector<kffsamp_t> imp_responce)
{
    size_t _nfft=imp_responce.size()*4;//rule of thumb
    _nfft=pow(2.0,(ceil(log2(_nfft))));
    cfg=kiss_fastfir_alloc(imp_responce.data(),imp_responce.size(),&_nfft,0,0);
    nfft=_nfft;
    reset();
}

void FastFIRFilter::reset()
{
    remainder.assign(nfft*2,0);
    idx_inbuf=0;
    remainder_ptr=nfft;
}

int FastFIRFilter::Update(kffsamp_t *data,int Size)
{

    //ensure enough storage
    if((inbuf.size()-idx_inbuf)<(size_t)Size)
    {
        inbuf.resize(Size+nfft);
        outbuf.resize(Size+nfft);
    }

    //add data to storage
    memcpy ( inbuf.data()+idx_inbuf, data, sizeof(kffsamp_t)*Size );
    size_t nread=Size;

    //fast fir of storage
    size_t nwrite=kiss_fastfir(cfg, inbuf.data(), outbuf.data(),nread,&idx_inbuf);

    int currentwantednum=Size;
    int numfromremainder=min(currentwantednum,remainder_ptr);

    //return as much as posible from remainder buffer
    if(numfromremainder>0)
    {
        memcpy ( data, remainder.data(), sizeof(kffsamp_t)*numfromremainder );

        currentwantednum-=numfromremainder;
        data+=numfromremainder;

        if(numfromremainder<remainder_ptr)
        {
            remainder_ptr-=numfromremainder;
            memcpy ( remainder.data(), remainder.data()+numfromremainder, sizeof(kffsamp_t)*remainder_ptr );
            //qDebug()<<"remainder left";
        } else remainder_ptr=0;
    }

    //then return stuff from output buffer
    int numfromoutbuf=std::min(currentwantednum,(int)nwrite);
    if(numfromoutbuf>0)
    {
        memcpy ( data, outbuf.data(), sizeof(kffsamp_t)*numfromoutbuf );
        currentwantednum-=numfromoutbuf;
        data+=numfromoutbuf;
    }

    //any left over is added to remainder buffer
    if(((size_t)numfromoutbuf<nwrite)&&(nwrite>0))
    {
        memcpy ( remainder.data()+remainder_ptr, outbuf.data()+numfromoutbuf, sizeof(kffsamp_t)*(nwrite-numfromoutbuf) );
        remainder_ptr+=(nwrite-numfromoutbuf);
    }


    //we should anyways have enough to return but if we dont this happens. this should be avoided else a discontinuity of frames occurs. set remainder to zero and set remainder_ptr to nfft before running to avoid this
    if(currentwantednum>0)
    {
        qDebug()<<"Error: user wants "<<currentwantednum<<" more items from fir filter!";
        remainder_ptr+=currentwantednum;
    }

    //return how many items we changed
    return Size-currentwantednum;

}

FastFIRFilter::~FastFIRFilter()
{
    free(cfg);
}

//-----------

FastFIRFilterInterleavedStereo::FastFIRFilterInterleavedStereo(std::vector<kffsamp_t> &imp_responce,size_t &nfft)
{
    left=new FastFIRFilter(imp_responce,nfft);
    right=new FastFIRFilter(imp_responce,nfft);
}

FastFIRFilterInterleavedStereo::~FastFIRFilterInterleavedStereo()
{
    delete left;
    delete right;
}

//needs tidy up
void FastFIRFilterInterleavedStereo::Update(kffsamp_t *data,int nFrames)
{
    int Size=2*nFrames;
    if((Size%2)!=0)return;//must be even
    if(leftbuf.size()*2!=(size_t)Size)//ensure storage
    {
        leftbuf.resize(Size/2);
        rightbuf.resize(Size/2);
    }
    int j=0;
    for(int i=0;i<(Size-1);i+=2)//deinterleave
    {
        leftbuf[j]=data[i];
        rightbuf[j]=data[i+1];
        j++;
    }
    //fast fir buffers
    left->Update(leftbuf.data(),leftbuf.size());
    right->Update(rightbuf.data(),rightbuf.size());
    j=0;
    for(int i=0;i<(Size-1);i+=2)//interleave
    {
        data[i]=leftbuf[j];
        data[i+1]=rightbuf[j];
        j++;
    }
}

//------------

//---- RRC Filter kernel

RootRaisedCosine::RootRaisedCosine()
{

}

RootRaisedCosine::RootRaisedCosine(double symbolrate, int firsize, double alpha, double samplerate)
{
    create(symbolrate,firsize,alpha,samplerate);
}

void RootRaisedCosine::scalepoints(double scale)
{
    for(uint k=0;k<Points.size();k++)Points[k]*=scale;
}

void  RootRaisedCosine::create(double symbolrate, int firsize, double alpha, double samplerate)
{
    if((firsize%2)==0)firsize-=1;
    Points.resize(firsize);
    double h;
    double squaresum=0;

    unsigned int k=0;
    for(int i=-firsize/2;i<=firsize/2;i++)
    {
        double t=((double)i)/samplerate;

        double delta=fabs(fabs(t)-(1.0/(4.0*alpha*symbolrate)));
        if(i)
        {
            if(delta>0.000001)
            {
                h= \
                  sqrt(symbolrate)* \
                  (sin(M_PI*t*symbolrate*(1.0-alpha))+4.0*alpha*t*symbolrate*cos(M_PI*t*symbolrate*(1.0+alpha))) \
                        / \
                  (M_PI*t*symbolrate*(1.0-(4.0*alpha*t*symbolrate*4.0*alpha*t*symbolrate)));
            }
             else
             {
                h=(alpha*sqrt(symbolrate/2.0))* \
                        ( \
                            (1.0+2.0/M_PI)*sin(M_PI/(4.0*alpha))\
                            +\
                            (1.0-2.0/M_PI)*cos(M_PI/(4.0*alpha))\
                         );
             }
        }
         else h=sqrt(symbolrate)*(1.0-alpha+4.0*alpha/M_PI);

        squaresum+=(h*h);

        if(k>=Points.size())abort();
        Points[k]=h;k++;

       // printf("i=%d t=%f delta=%f h=%f\n",i,t,delta,h*0.050044661);
    }

    double normalizeor=1.0/(sqrt(squaresum));//found in textbooks
    //double normalizeor=1.0/(sqrt(symbolrate)*(1.0-alpha+4.0*alpha/M_PI));//makes the main peak equal to one
    for(k=0;k<Points.size();k++)Points[k]*=normalizeor;

  //  for(k=0;k<Points.size();k++)Points[k]*=0.5*(1.0-cos(2.0*M_PI*k/((double)(Points.size()-1))));

    // printf("normalization = %f\n",1.0/(sqrt(squaresum)));

    /*k=0;
    for(int i=-firsize/2;i<=firsize/2;i++)
    {
        double t=((double)i)/samplerate;
        if(k>=Points.size())abort();
        printf("i=%d t=%f h=%f\n",i,t,Points[k]);
        k++;
    }*/

}

//-----

//---fast FIR (should port old fast fir to new one todo)

JFastFIRFilter::JFastFIRFilter()
{
    vector<kffsamp_t> tvect;
    tvect.push_back(1.0);
    nfft=2;
    cfg=kiss_fastfir_alloc(tvect.data(),tvect.size(),&nfft,0,0);
    reset();
}

int JFastFIRFilter::setKernel(vector<kffsamp_t> imp_responce)
{
    int _nfft=imp_responce.size()*4;//rule of thumb
    _nfft=pow(2.0,(ceil(log2(_nfft))));
    return setKernel(imp_responce,_nfft);
}

int JFastFIRFilter::setKernel(vector<kffsamp_t> imp_responce, int _nfft)
{
    if(!imp_responce.size())return nfft;
    free(cfg);
    _nfft=pow(2.0,(ceil(log2(_nfft))));
    nfft=_nfft;
    cfg=kiss_fastfir_alloc(imp_responce.data(),imp_responce.size(),&nfft,0,0);
    reset();
    return nfft;
}

void JFastFIRFilter::reset()
{
    remainder.assign(nfft*2,0);
    idx_inbuf=0;
    remainder_ptr=nfft;

    single_input_output_buf_ptr=0;
    single_input_output_buf.assign(nfft,0);
}

void JFastFIRFilter::Update(vector<kffsamp_t> &data)
{
    Update(data.data(), data.size());
}

double JFastFIRFilter::Update_Single(double signal)
{
    double out_signal=single_input_output_buf[single_input_output_buf_ptr];
    single_input_output_buf[single_input_output_buf_ptr]=signal;
    single_input_output_buf_ptr++;single_input_output_buf_ptr%=single_input_output_buf.size();
    if(single_input_output_buf_ptr==0)Update(single_input_output_buf.data(),single_input_output_buf.size());
    return out_signal;
}


void JFastFIRFilter::Update(kffsamp_t *data,int Size)
{

    //ensure enough storage
    if((inbuf.size()-idx_inbuf)<(size_t)Size)
    {
        inbuf.resize(Size+nfft);
        outbuf.resize(Size+nfft);
    }

    //add data to storage
    memcpy ( inbuf.data()+idx_inbuf, data, sizeof(kffsamp_t)*Size );
    size_t nread=Size;

    //fast fir of storage
    size_t nwrite=kiss_fastfir(cfg, inbuf.data(), outbuf.data(),nread,&idx_inbuf);

    int currentwantednum=Size;
    int numfromremainder=min(currentwantednum,remainder_ptr);

    //return as much as posible from remainder buffer
    if(numfromremainder>0)
    {
        memcpy ( data, remainder.data(), sizeof(kffsamp_t)*numfromremainder );

        currentwantednum-=numfromremainder;
        data+=numfromremainder;

        if(numfromremainder<remainder_ptr)
        {
            remainder_ptr-=numfromremainder;
            memcpy ( remainder.data(), remainder.data()+numfromremainder, sizeof(kffsamp_t)*remainder_ptr );
        } else remainder_ptr=0;
    }

    //then return stuff from output buffer
    int numfromoutbuf=std::min(currentwantednum,(int)nwrite);
    if(numfromoutbuf>0)
    {
        memcpy ( data, outbuf.data(), sizeof(kffsamp_t)*numfromoutbuf );
        currentwantednum-=numfromoutbuf;
        data+=numfromoutbuf;
    }

    //any left over is added to remainder buffer
    if(((size_t)numfromoutbuf<nwrite)&&(nwrite>0))
    {
        memcpy ( remainder.data()+remainder_ptr, outbuf.data()+numfromoutbuf, sizeof(kffsamp_t)*(nwrite-numfromoutbuf) );
        remainder_ptr+=(nwrite-numfromoutbuf);
    }

    //if currentwantednum>0 then some items were not changed, this should not happen
    //we should anyways have enough to return but if we dont this happens. this should be avoided else a discontinuity of frames occurs. set remainder to zero and set remainder_ptr to nfft before running to avoid this
    if(currentwantednum>0)
    {
        remainder_ptr+=currentwantednum;
    }

}

JFastFIRFilter::~JFastFIRFilter()
{
    free(cfg);
}

//-----------

//---------------------

MovingAverage::MovingAverage()
{
    setSize(100);
}

void MovingAverage::setSize(int size)
{
    MASum=0;
    MABuffer.resize(size,0.0);
    MASz=MABuffer.size();
    MAPtr=0;
    Val=0;
    PrecisionDilutionCorrectCnt=MASz;//correct rounding error every so often
}

double MovingAverage::Update(double sig)
{

    if(PrecisionDilutionCorrectCnt)PrecisionDilutionCorrectCnt--;
     else
     {
        MASum=0;
        for(int i=0;i<MASz;++i)
        {
            MASum+=MABuffer[i];
        }
        PrecisionDilutionCorrectCnt=MASz;
     }

    MASum=MASum-MABuffer[MAPtr];
    MASum=MASum+sig;
    MABuffer[MAPtr]=sig;
    MAPtr++;if(MAPtr>=MASz)MAPtr=0;
    Val=MASum/((double)MASz);
    return Val;
}

//---------------------
