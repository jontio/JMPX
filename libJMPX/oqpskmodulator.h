#ifndef OQPSKMODULATOR_H
#define OQPSKMODULATOR_H

#include <QObject>
#include <QMutex>
#include "JDSP.h"

class OQPSKModulator : public QObject
{
    Q_OBJECT
public:
    struct Settings
    {
        double bitrate;double carrier_freq;double alpha;int max_bit_buffer_size;
        Settings(){bitrate=18500;carrier_freq=8000,alpha=0.75;max_bit_buffer_size=40000;}
    };
    explicit OQPSKModulator(TDspGen *_pDspGen, QObject *parent = 0);
    ~OQPSKModulator();
    void RefreshSettings(double bitrate, double carrier_freq, double alpha, int max_bit_buffer_size);
    void RefreshSettings(OQPSKModulator::Settings settings){RefreshSettings(settings.bitrate,settings.carrier_freq,settings.alpha,settings.max_bit_buffer_size);}
    void RefreshSettings(){RefreshSettings(settings);}
    OQPSKModulator::Settings getSettings(){return settings;}
    double update();
    void StartSpooling();
    void StopSpooling();
    bool isSpooling(){return spooling;}
private:
    TDspGen *pDspGen;
    std::auto_ptr< WaveTable > pWaveTableCarrier;
    std::auto_ptr< WaveTable > pWaveTableSymbol;
    RootRaisedCosine rrc;
    JFastFIRFilter fir_re;
    JFastFIRFilter fir_im;

    OQPSKModulator::Settings settings;

    QVector<int> buffer;
    int buffer_head;
    int buffer_tail;
    int buffer_used;
    int buffer_size;
    QMutex buffer_mutex;

    bool askedformoredata;

    cpx_type symbol_this;
    cpx_type symbol_next;

    bool spooling;

    JFastFIRFilter *dscabpf;

signals:
    void CallForMoreData(int buffer_free);
public slots:
    bool LoadBits(const QVector<int> &bits);
private slots:
    void delayedStartSpooling();

};

#endif // OQPSKMODULATOR_H
