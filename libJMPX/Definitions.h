#ifndef DEFINITIONS_H
#define DEFINITIONS_H

struct TSigStats
{
    double scavol;
    double lvol;
	double rvol;
	double outvol;
    double opusbufferuseagepercent;
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
                SampleRate=44100;
        }
        int SampleRate;
};


#endif  //DEFINITIONS_H
