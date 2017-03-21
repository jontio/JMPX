#include "oqpskmodulator.h"
#include <QTimer>

OQPSKModulator::OQPSKModulator(TDspGen *_pDspGen, QObject *parent) :
    QObject(parent),
    pDspGen(_pDspGen),
    pWaveTableCarrier(new WaveTable(pDspGen,1000)),
    pWaveTableSymbol(new WaveTable(pDspGen,200))
{
    spooling=false;    
    dscabpf = new JFastFIRFilter;
    //set default settings
    RefreshSettings(2400,3000,0.75,40000);//2400bps at 3khz carrier and 0.75 excess. 40000 bits of buffer storage
}

OQPSKModulator::~OQPSKModulator()
{
    delete dscabpf;
}

bool OQPSKModulator::LoadBits(const QVector<int> &bits)
{
    buffer_mutex.lock();
    if((buffer_used+bits.size())>=buffer_size)
    {
        qDebug()<<"OQPSKModulator::LoadBits buffer overflow!!: buffer_head="<<buffer_head<<" buffer_tail="<<buffer_tail<<" buffer_used="<<buffer_used<<" buffer_size="<<buffer_size;
        buffer_mutex.unlock();
        askedformoredata=false;
        return false;
    }
    askedformoredata=false;
    for(int i=0;i<bits.size();i++)
    {
        buffer[buffer_head]=bits.at(i);
        buffer_head++;buffer_head%=buffer_size;
        buffer_used++;
    }

    buffer_mutex.unlock();
    return true;
}

void OQPSKModulator::StartSpooling()
{
    buffer_mutex.lock();
    buffer_head=0;
    buffer_tail=0;
    buffer_used=0;
    buffer_size=buffer.size();
    askedformoredata=true;
    int tmpbuffer_size=buffer_size;
    spooling=true;
    buffer_mutex.unlock();
    emit CallForMoreData(tmpbuffer_size);
}

void OQPSKModulator::StopSpooling()
{
    buffer_mutex.lock();
    spooling=false;
    buffer_mutex.unlock();
}

double OQPSKModulator::update()
{


//    pWaveTableCarrier->IncreseFreqHz(0.00005);//drift test
    pWaveTableCarrier->WTnextFrame();
    pWaveTableSymbol->WTnextFrame();
    cpx_type symbol=cpx_type(0,0);//symbols sent as impulses not as constants
    if(pWaveTableSymbol->IfPassesPointNextTime())
    {
        buffer_mutex.lock();

        //load a bit
        int bit;
        if(buffer_used)
        {
            bit=buffer[buffer_tail]%2;
            if(bit<0)bit*=bit;
            buffer_tail++;buffer_tail%=buffer_size;
            buffer_used--;
            if((!askedformoredata)&&((buffer_used*2)<buffer_size))
            {
                askedformoredata=true;
                emit CallForMoreData(buffer_size-buffer_used);
            }
        }else bit=rand()%2;//if this happens then we stall!!! call StartSpooling() to start again

        //create symbol
        static int sel=0;
        sel++;sel%=2;
        if(sel)
        {
            symbol=cpx_type(2.0*(((double)(bit))-0.5),symbol.imag());
        }
         else symbol=cpx_type(symbol.real(),2.0*(((double)(bit))-0.5));

        //symbol_this=symbol*(1.0-pWaveTableSymbol->FractionOfSampleItPassesBy);
        //symbol_next=symbol*pWaveTableSymbol->FractionOfSampleItPassesBy;

        buffer_mutex.unlock();
    }


//    if(abs(symbol_this)>0.001)
//    {
//        symbol=symbol_this;
//        symbol_this=0;
//    }
//     else if(abs(symbol_next)>0.001)
//     {
//         symbol=symbol_next;
//         symbol_next=0;
//     }

    //load carrier
    cpx_type carrier=pWaveTableCarrier->WTCISValue();

    //update complex signal
    cpx_type signal=cpx_type(carrier.real()*fir_re.Update_Single(symbol.real()),carrier.imag()*fir_im.Update_Single(symbol.imag()));

    //return the two arms
    //return (0.5*(signal.real()+signal.imag()));
    return dscabpf->Update_Single(0.5*(signal.real()+signal.imag()));
}

void OQPSKModulator::RefreshSettings(double bitrate,double carrier_freq,double alpha, int max_bit_buffer_size)
{

//    bitrate+=0.01;

    bool wasspooling=spooling;
    spooling=false;

    settings.bitrate=bitrate;
    settings.carrier_freq=carrier_freq;
    settings.alpha=alpha;
    settings.max_bit_buffer_size=max_bit_buffer_size;

    buffer_head=0;
    buffer_tail=0;
    buffer_used=0;
    buffer.fill(0,max_bit_buffer_size);
    buffer_size=buffer.size();
    askedformoredata=false;

    pWaveTableCarrier->RefreshSettings(carrier_freq);
    pWaveTableSymbol->RefreshSettings(bitrate);

    double symbolrate=bitrate/2.0;//OQPSK has two bits per symbol
    int firlen=5*6*pDspGen->SampleRate/symbolrate; //about 6 symbols overlaping produces good roll off. affects cpu usage.

    rrc.create(symbolrate,firlen,alpha,pDspGen->SampleRate);
    rrc.scalepoints(6.1);
    fir_re.setKernel(rrc.Points);
    fir_im.setKernel(rrc.Points);

    double bw=0.5*(1.0+alpha)*bitrate+10;
    double minfreq=qMax(carrier_freq-bw/2.0,1.0);
    double maxfreq=qMin(carrier_freq+bw/2.0,((double)(pDspGen->SampleRate))/2.0-1.0);
    dscabpf->setKernel(JFilterDesign::BandPassHanning(minfreq,maxfreq,pDspGen->SampleRate,256-1));

    symbol_this=0;
    symbol_next=0;

    if(wasspooling)
    {
        askedformoredata=true;
        spooling=true;
        QTimer::singleShot(0,this,SLOT(delayedStartSpooling()));
    }

}

void OQPSKModulator::delayedStartSpooling()
{
    StartSpooling();
}
