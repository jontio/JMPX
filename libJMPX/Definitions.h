#ifndef DEFINITIONS_H
#define DEFINITIONS_H

using namespace std;

struct TSigStats
{
	double lvol;
	double rvol;
	double outvol;
};

enum TimeConstant {WORLD,USA,NONE};

struct SDeviceInfo
{
	unsigned int dev;
	const char* name;
	unsigned int* SampleRates;
	unsigned int NumberOfSampleRates;
    unsigned int inchannelcount;
    unsigned int outchannelcount;
};

struct SDevices
{
	SDeviceInfo* Device;
	unsigned int NumberOfDevices;
};

//general settings
class TSetGen
{
public:
         TSetGen()
        {
                Freq=4800;            // frequency
                SampleRate=44100;
        }
        double Freq;            // frequency
        int SampleRate;
};


#endif  //DEFINITIONS_H
