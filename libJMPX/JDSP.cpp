//---------------------------------------------------------------------------


#include "JDSP.h"

//---------------------------------------------------------------------------



 WaveTable::WaveTable(TDspGen *_pDspGen) :
        pDspGen(_pDspGen)
{
        RefreshSettings();
}



WaveTable::~WaveTable()
{
 //
}

void  WaveTable::RefreshSettings()
{
        WTstep=(pDspGen->Freq)*(WTSIZE)/((double)(pDspGen->SampleRate));
        WTptr=0;
        intWTptr=0;
}

void WaveTable::WTnextFrame()
{
	WTptr+=WTstep;
	while(!signbit(WTptr-(double)WTSIZE))WTptr-=(double)(WTSIZE);
	if(signbit(WTptr))WTptr=0;
	intWTptr=(int)WTptr;intWTptr%=WTSIZE;
}

double   WaveTable::WTSinValue()
{
	return pDspGen->SinWT[intWTptr];
}

double   WaveTable::WTSin2Value()
{
        double tmpwtptr=WTptr*2.0;
    	while(!signbit(tmpwtptr-(double)WTSIZE))tmpwtptr-=(double)(WTSIZE);
    	if(signbit(tmpwtptr))tmpwtptr=0;
		return pDspGen->SinWT[(int)tmpwtptr];
}
//----------

void PreEmphasis::SetTc(TimeConstant timeconst)
{
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

Clipper::Clipper()
{
	SetCompressionPoint(0.85l);
}

void Clipper::SetCompressionPoint(double point)
{
	if(point>0.99l)point=0.99l;
	 else if(point<0.01l)point=0.01l;
	compressionpoint=point;
	double a=M_PI/(2.0l*(1.0l-compressionpoint));
	LookupTable.resize(6000);
	for(int i=0;i<(int)LookupTable.size();i++)
	{
		LookupTable[i]=compressionpoint+atan(a*(i*0.001l))/a;
	}
}

double Clipper::Update(double val)
{
	if(fabs(val)<compressionpoint)return val;
	double inv=1.0l;
	if(val<0.0l)
	{
		inv=-1.0l;
		val=-val;
	}

	double shiftmultval=(val-compressionpoint)*1000.0l;
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
        int i;
        for(i=0;i<WTSIZE;i++){SinWT[i]=(sin(2.0*M_PI*((double)i)/((double)WTSIZE)));}
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
        Freq=pSetGen->Freq;
}



//---------------


//---fast FIR


FastFIRFilter::FastFIRFilter(std::vector<kffsamp_t> &imp_responce,size_t &_nfft)
{
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
            qDebug()<<"remainder left";
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

void FastFIRFilterInterleavedStereo::Update(kffsamp_t *data,int Size)
{
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
