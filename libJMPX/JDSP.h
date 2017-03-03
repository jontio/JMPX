//---------------------------------------------------------------------------

#ifndef JDSP_H
#define JDSP_H

#include "../kiss_fft130/tools/kiss_fastfir.h"
#include "../kiss_fft130/tools/kiss_fftr.h"

#include <memory>
#include <math.h>
#include <vector>
#include <string>
#include <valarray>

#include <QDebug>

#if defined(__linux__)
#include <sys/time.h>
#include <sys/resource.h>
#include "semaphore.h"
#include <pthread.h>
#define MSSLEEP(x)	usleep(x*1000)
#else
#include <windows.h>
#define MSSLEEP(x)	Sleep(x)
#endif

#include <iostream>

#include <sstream>

#include "Definitions.h"



//OK signal
//#define WTSIZE 8000
//#define WTSIZE_1 7999

//Insanely good signal
#define WTSIZE 16000
#define WTSIZE_1 15999
#define WT_Interpolate

using namespace std;

class TDspGen
{
private:
        TSetGen *pSetGen;

public:

        void  ResetSettings();
         TDspGen(TSetGen *_pSetGen);
         ~TDspGen();

        vector<double> SinWT;

        double Freq;
        int SampleRate;
};

//------------------

class SymbolPointer
{
public:
    SymbolPointer();
    void setFreq(double freqHZ,double samplerate);
    inline void nextFrame()
    {
        symbolptr+=symbolstep;
        while(!signbit(symbolptr-(double)WTSIZE))symbolptr-=(double)(WTSIZE);
        if(signbit(symbolptr))symbolptr=0;
    }
    inline bool ifPassesPointNextTime()
    {
        FractionOfSampleItPassesBy=-symbolptr;
        while(signbit(FractionOfSampleItPassesBy))FractionOfSampleItPassesBy+=(double)WTSIZE;
        if(signbit(FractionOfSampleItPassesBy-symbolstep))
        {
            FractionOfSampleItPassesBy=(symbolstep-FractionOfSampleItPassesBy)/symbolstep;
            return true;
        }
        return false;
    }
    double FractionOfSampleItPassesBy;
private:
    double symbolstep,symbolptr;
};

//------------------

class WaveTable
{
public:
        WaveTable(TDspGen *_pDspGen);

        ~WaveTable();
        void WTnextFrame();
        double   WTSinValue();
        double   WTSin2Value();
        double   WTSin3Value();
        void  RefreshSettings();
        void WTnextFrame(double offset_in_hz);
private:
        double WTstep;
        int intWTptr;
        double WTptr;
        TDspGen *pDspGen;
};

//-------------------

class JFilterDesign
{
public:
    JFilterDesign(){}
    static std::vector<kffsamp_t> LowPassHanning(double FrequencyCutOff, double SampleRate, int Length);
    static std::vector<kffsamp_t> HighPassHanning(double FrequencyCutOff, double SampleRate, int Length);
    static std::vector<kffsamp_t> BandPassHanning(double LowFrequencyCutOff,double HighFrequencyCutOff, double SampleRate, int Length);
private:
};

//-------------------

class PreEmphasis
{
private:
	double a[2];
	double b[2];
    double y[2];
	double x[2];
    TimeConstant timeconst;
public:
	PreEmphasis();
	void SetTc(TimeConstant timeconst);
    TimeConstant GetTc();
	double Update(double val);
};

class Clipper
{
private:
	vector<double> LookupTable;
	double compressionpoint;
public:
	Clipper();
	double Update(double val);
	void SetCompressionPoint(double point);

};

//------------

class Cof_16500Hz_192000bps_119taps
{
public:
    std::vector<double> Cof;
    Cof_16500Hz_192000bps_119taps()
    {
        Cof.resize(119);
        Cof[0] = -0.000167638;Cof[1] = -0.000268748;Cof[2] = -9.56764e-05;Cof[3] = 0.000115229;Cof[4] = 0.000281049;Cof[5] = 0.000338302;Cof[6] = 0.000268326;Cof[7] = 0.000103853;Cof[8] = -8.59957e-05;Cof[9] = -0.00022598;Cof[10] = -0.000267365;Cof[11] = -0.000208623;Cof[12] = -9.46927e-05;Cof[13] = 5.66977e-06;Cof[14] = 3.5084e-05;Cof[15] = -2.04599e-05;Cof[16] = -0.00011854;Cof[17] = -0.000174899;Cof[18] = -0.00010276;Cof[19] = 0.000137925;Cof[20] = 0.000503055;Cof[21] = 0.000859024;Cof[22] = 0.00101918;Cof[23] = 0.000815281;Cof[24] = 0.000180721;Cof[25] = -0.000787676;Cof[26] = -0.0018204;Cof[27] = -0.00253487;Cof[28] = -0.00255603;Cof[29] = -0.00166446;Cof[30] = 7.89081e-05;Cof[31] = 0.00228406;Cof[32] = 0.00429358;Cof[33] = 0.00535571;Cof[34] = 0.00486947;Cof[35] = 0.00262937;Cof[36] = -0.00101614;Cof[37] = -0.00516394;Cof[38] = -0.00854399;Cof[39] = -0.00986765;Cof[40] = -0.00825062;Cof[41] = -0.00358529;Cof[42] = 0.00326712;Cof[43] = 0.010558;Cof[44] = 0.0160283;Cof[45] = 0.0175149;Cof[46] = 0.0136407;Cof[47] = 0.00439635;Cof[48] = -0.00857179;Cof[49] = -0.022094;Cof[50] = -0.0320499;Cof[51] = -0.0342823;Cof[52] = -0.0256475;Cof[53] = -0.00494015;Cof[54] = 0.026558;Cof[55] = 0.0650793;Cof[56] = 0.104962;Cof[57] = 0.139727;Cof[58] = 0.163403;Cof[59] = 0.171798;Cof[60] = 0.163403;Cof[61] = 0.139727;Cof[62] = 0.104962;Cof[63] = 0.0650793;Cof[64] = 0.026558;Cof[65] = -0.00494015;Cof[66] = -0.0256475;Cof[67] = -0.0342823;Cof[68] = -0.0320499;Cof[69] = -0.022094;Cof[70] = -0.00857179;Cof[71] = 0.00439635;Cof[72] = 0.0136407;Cof[73] = 0.0175149;Cof[74] = 0.0160283;Cof[75] = 0.010558;Cof[76] = 0.00326712;Cof[77] = -0.00358529;Cof[78] = -0.00825062;Cof[79] = -0.00986765;Cof[80] = -0.00854399;Cof[81] = -0.00516394;Cof[82] = -0.00101614;Cof[83] = 0.00262937;Cof[84] = 0.00486947;Cof[85] = 0.00535571;Cof[86] = 0.00429358;Cof[87] = 0.00228406;Cof[88] = 7.89081e-05;Cof[89] = -0.00166446;Cof[90] = -0.00255603;Cof[91] = -0.00253487;Cof[92] = -0.0018204;Cof[93] = -0.000787676;Cof[94] = 0.000180721;Cof[95] = 0.000815281;Cof[96] = 0.00101918;Cof[97] = 0.000859024;Cof[98] = 0.000503055;Cof[99] = 0.000137925;Cof[100] = -0.00010276;Cof[101] = -0.000174899;Cof[102] = -0.00011854;Cof[103] = -2.04599e-05;Cof[104] = 3.5084e-05;Cof[105] = 5.66977e-06;Cof[106] = -9.46927e-05;Cof[107] = -0.000208623;Cof[108] = -0.000267365;Cof[109] = -0.00022598;Cof[110] = -8.59957e-05;Cof[111] = 0.000103853;Cof[112] = 0.000268326;Cof[113] = 0.000338302;Cof[114] = 0.000281049;Cof[115] = 0.000115229;Cof[116] = -9.56764e-05;Cof[117] = -0.000268748;Cof[118] = -0.000167638;
    }
};

//---------slow fir

class FIRFilter
{
private:
    vector<double> Buf;
	int BufPtr;
public:
    vector<double> Cof;
	FIRFilter();
	double Update(double val);
    void Update(double *data,int Size);
    void UpdateInterleavedEven(double *data,int Size);
    void UpdateInterleavedOdd(double *data,int Size);
    void setBuf(double val)
    {
        Buf.resize(Cof.size());
        for(unsigned int i=0;i<Buf.size();i++)Buf[i]=val;
    }
};

//---------fast fir

class FastFIRFilter
{
public:
    FastFIRFilter(std::vector<kffsamp_t> imp_responce, size_t &nfft);
    FastFIRFilter(std::vector<kffsamp_t> imp_responce);
    int Update(kffsamp_t *data,int Size);
    void reset();
    ~FastFIRFilter();
    size_t nfft;
    kiss_fastfir_cfg cfg;
    size_t idx_inbuf;

    std::vector<kffsamp_t> inbuf;
    std::vector<kffsamp_t> outbuf;
    std::vector<kffsamp_t> remainder;
    int remainder_ptr;

};

//---------stereo fast fir

class FastFIRFilterInterleavedStereo
{
public:
    FastFIRFilterInterleavedStereo(std::vector<kffsamp_t> &imp_responce,size_t &nfft);
    ~FastFIRFilterInterleavedStereo();
    void Update(kffsamp_t *data,int Size);
    void reset()
    {
        left->reset();
        right->reset();
    }
private:
    FastFIRFilter *left;
    FastFIRFilter *right;
    std::vector<kffsamp_t> leftbuf;
    std::vector<kffsamp_t> rightbuf;

};

//----------stereo fast fir for 16.5kHz at 192000 bps
class InterleavedStereo16500Hz192000bpsFilter
{
public:
    InterleavedStereo16500Hz192000bpsFilter()
    {
        Cof_16500Hz_192000bps_119taps lpfc;
        size_t nfft=512;
        fir = new FastFIRFilterInterleavedStereo(lpfc.Cof,nfft);
    }
    ~InterleavedStereo16500Hz192000bpsFilter()
    {
        delete fir;
    }
    void Update(kffsamp_t *data,int Size){fir->Update(data,Size);}
    void reset(){fir->reset();}
private:
    FastFIRFilterInterleavedStereo *fir;
};

//-----RRC Filter kernel

class RootRaisedCosine
{
public:
    RootRaisedCosine();
    RootRaisedCosine(double symbolrate, int firsize, double alpha, double samplerate);
    void create(double symbolrate, int firsize, double alpha, double samplerate);
    void scalepoints(double scale);
    vector<double> Points;
};


//-----RDS Biphase symbol generator

//jfast fir

class JFastFIRFilter
{
public:
    JFastFIRFilter();
    int setKernel(vector<kffsamp_t> imp_responce,int nfft);
    int setKernel(vector<kffsamp_t> imp_responce);
    void Update(kffsamp_t *data,int Size);
    void Update(vector<kffsamp_t> &data);
    double Update_Single(double signal);
    void reset();
    ~JFastFIRFilter();
private:
    size_t nfft;
    kiss_fastfir_cfg cfg;
    size_t idx_inbuf;
    std::vector<kffsamp_t> inbuf;
    std::vector<kffsamp_t> outbuf;
    std::vector<kffsamp_t> remainder;
    int remainder_ptr;

    //for single byte at a time.
    std::vector<double> single_input_output_buf;
    int single_input_output_buf_ptr;
};

//-------

//-----FM modulator for SCA

class FMModulator
{
public:
    FMModulator(TSetGen *_pSetGen);
    ~FMModulator();
    double update(double &insignal);
    void RefreshSettings(double carrier_freq, double max_audio_input_frequency, double max_deviation, double ouput_bandwidth);
    void RefreshSettings(double carrier_freq, double max_audio_input_frequency, double max_deviation)
    {
        //convenience function.
        //sets output bandwith using carson's rule
         RefreshSettings(carrier_freq,max_audio_input_frequency,max_deviation, 2.0*(max_audio_input_frequency+max_deviation));
    }
    void SetTc(TimeConstant timeconst){preemp.SetTc(timeconst);}
    TimeConstant GetTc(){return preemp.GetTc();}
private:
    double max_deviation;
    double ouput_bandwidth;
    TSetGen ASetGen;
    std::auto_ptr< TDspGen > pTDSPGen;
    std::auto_ptr< TDspGen > pTDSPGenCarrier;
    std::auto_ptr< WaveTable > pWaveTableCarrier;
    JFastFIRFilter *fir;

    double maxinfreq;

    Clipper clipper;

    PreEmphasis preemp;

    JFastFIRFilter *outputbpf;
};

//-------------------

class MovingAverage
{
public:
    MovingAverage();
    void setSize(int size);
    double Update(double sig);
    double Val;
private:
    int MASz;
    double MASum;
    std::vector<double> MABuffer;
    int MAPtr;
};

//-------------------

class PeakMeasure
{
public:
    PeakMeasure()
    {
        vol=0;
        holdcnt=0;
        t_vol=0;
        maxholdcnt=20;
        decay=0.01;
        smoothing=0.8;
        constantdecay=0.0001;
        expo_smoothing=false;
        ma.setSize(40);
    }
    void setSettings(int _maxholdcnt,double _decay,double _smoothing,double _constantdecay, int movingavesize, bool _expo_smoothing)
    {
        maxholdcnt=_maxholdcnt;
        decay=_decay;
        smoothing=_smoothing;
        constantdecay=_constantdecay;
        expo_smoothing=_expo_smoothing;
        ma.setSize(movingavesize);
    }
    void zero()
    {
        vol=0;
        holdcnt=0;
        t_vol=0;
    }
    double update(double signal)
    {
        signal=fabs(signal);
        if(signal>t_vol)
        {
            t_vol=signal;
            holdcnt=maxholdcnt;
        }
        if(holdcnt>0)holdcnt--;
        if(!holdcnt)
        {
            t_vol-=decay;
            if(t_vol<0)t_vol=0;
        }
        if(expo_smoothing)vol=smoothing*vol+(1.0-smoothing)*t_vol-constantdecay;
         else vol=ma.Update(t_vol);
        if(vol<0)vol=0;
        return vol;
    }
    double vol;
private:
    bool expo_smoothing;
    int holdcnt;
    double t_vol;
    int maxholdcnt;
    double decay;
    double smoothing;
    double constantdecay;
    MovingAverage ma;
};

//-------------------



#endif  //JDSP_H
