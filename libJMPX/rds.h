#ifndef RDS_H
#define RDS_H

#include <QObject>
#include <QMutex>
#include <QDateTime>
#include "JDSP.h"

#include <time.h>


//Link layer
class RDSGroup
{
public:
    typedef enum { Block_A=0x0FC, Block_B=0x198, Block_C=0x168,Block_C_prime=350,Block_D=0x1B4,Block_E=0 } offset_type;
    RDSGroup()
    {
        data_chunk.fill((uchar)0,26*4);
        repeat=true;
    }
    void setBlock(quint16 message,offset_type offset);
    QByteArray data_chunk;
    bool repeat;//unused atm
private:
    quint16 crc(quint16 message);
};

//PHY layer
//#define TIMERTEST
#define FASTFIRMETHOD
class RDSDifferentialBiPhaseSymbolGenerator : public QObject
{
    Q_OBJECT
public:
    explicit RDSDifferentialBiPhaseSymbolGenerator(QObject *parent = 0);
    ~RDSDifferentialBiPhaseSymbolGenerator();
    QVector<double> outputsignal;
    void FIRFilterImpulses(int size);
    bool havemorebits(QByteArray &bits);
    void reset();
signals:
    void wantmoregroups();
public slots:   
private:
    SymbolPointer sp;
    RootRaisedCosine rrc;

#ifndef FASTFIRMETHOD
    int symbolbuffsize;
    QVector<int> symbolbuffcntr;
    QVector<double> cosampsymbolbuff;
    int symbolbuffptr;
#endif

#ifdef FASTFIRMETHOD
    FastFIRFilter *fir;
    double pthis;
    double pnext;
#endif

    int onoffcntr;
    int lastch;



#ifdef TIMERTEST
    qint64 nanoSecsum;
    int nanoseccnt;
#endif

protected:
    QByteArray buffer;
    int buffer_head;
    int buffer_tail;
    int buffer_used;
    int buffer_size;
    QMutex buffer_mutex;

};

//-----------start of group implimentations (maybe the implimentations should be moved to rds.c. Personally I found it eaiser to work on them here)

//Linkish or Appish layer?
class RDS0AGroup : public RDSGroup
{
public:
    RDS0AGroup()
    {

        //set defaults
        pi=0x9000;
        stereo=true;
        artificial_head=false;
        compressed=false;
        dynamic_pty=false;
        tp=false;
        ta=false;
        ms=true;
        pty=0;
        set_ps("JMPX RDS");
        set_altfreq1(0xE0);//no alt freqs
        set_altfreq2(0xE0);//no alt freqs

        //init
        c=-1;
    }
    QString get_ps()
    {
        return ((QString)ba).trimmed();
    }
    void set_ps(QString ps)
    {
        ba=ps.leftJustified(8, ' ').toLatin1();
    }
    double get_altfreq1()
    {
        if((alt_freq1<1)||(alt_freq1>204))return alt_freq1;
        double tfreq=alt_freq1;
        tfreq+=875;
        tfreq/=10.0;
        return tfreq;
    }
    double get_altfreq2()
    {
        if((alt_freq2<1)||(alt_freq2>204))return alt_freq2;
        double tfreq=alt_freq2;
        tfreq+=875;
        tfreq/=10.0;
        return tfreq;
    }
    void set_altfreq1(double freq_mhz)
    {
        if(freq_mhz>107.95)
        {
            alt_freq1=round(freq_mhz);
            return;
        }
        int tfreq=(freq_mhz*10.0);
        tfreq-=875;
        if(tfreq<1)tfreq=1;
        alt_freq1=tfreq;
    }
    void set_altfreq2(double freq_mhz)
    {
        if(freq_mhz>107.95)
        {
            alt_freq2=round(freq_mhz);
            return;
        }
        int tfreq=(freq_mhz*10.0);
        tfreq-=875;
        if(tfreq<1)tfreq=1;
        alt_freq2=tfreq;
    }
    void rewind()
    {
        c--;c%=4;
    }
    QByteArray &bits()
    {

        quint16 blk;

        //A
        setBlock(pi,RDSGroup::Block_A);

        //B
        blk=0x0000;
        if(tp)blk|=0x0400;
        blk|=(((quint16)pty)<<5);
        if(ta)blk|=0x0010;
        if(ms)blk|=0x0008;
        c++;c%=4;
        switch(c)
        {
        case 0:
            if(dynamic_pty)blk|=0x0004;
            break;
        case 1:
            if(compressed)blk|=0x0004;
            break;
        case 2:
            if(artificial_head)blk|=0x0004;
            break;
        case 3:
            if(stereo)blk|=0x0004;
            break;
        }
        blk|=((quint16)c);
        setBlock(blk,RDSGroup::Block_B);

        //C
        if((alt_freq1==205)&&(alt_freq2==205))//if both fillers then say there are no afs
        {
            alt_freq1=224;
            alt_freq2=224;
        }
        if(alt_freq1<205)blk=((((quint16)alt_freq1)<<8)|((quint16)alt_freq2));//if first is filler then flip
         else blk=((((quint16)alt_freq2)<<8)|((quint16)alt_freq1));
        setBlock(blk,RDSGroup::Block_C);

        //D
        switch(c)
        {
        case 0:
            blk=((((quint16)ba[0])<<8)|((quint16)ba[1]));
            break;
        case 1:
            blk=((((quint16)ba[2])<<8)|((quint16)ba[3]));
            break;
        case 2:
            blk=((((quint16)ba[4])<<8)|((quint16)ba[5]));
            break;
        case 3:
            blk=((((quint16)ba[6])<<8)|((quint16)ba[7]));
            break;
        }
        setBlock(blk,RDSGroup::Block_D);

        return data_chunk;
    }

    bool stereo;
    bool artificial_head;
    bool compressed;
    bool dynamic_pty;
    bool tp;
    bool ta;
    bool ms;
    quint8 pty;
    quint16 pi;
private:
    int c;
    quint8 alt_freq1;
    quint8 alt_freq2;
    QByteArray ba;
};

//Linkish or Appish layer?
class RDS2AGroup : public RDSGroup
{
public:
    RDS2AGroup()
    {

        //set defaults
        pi=0x9000;
        tp=false;
        pty=0;
        set_rt("Radio text test");

        //init
        c=-1;
        abflag=false;
    }
    QString get_rt()
    {
        return ((QString)ba).trimmed();
    }
    void set_rt(QString rt)
    {
        rt.append(0x0D);
        QByteArray tba=rt.leftJustified(64, ' ').toLatin1();
        if(tba==ba)return;//only change if need be
        ba=tba;
        abflag=!abflag;
        c=-1;
    }
    void rewind()
    {
        c--;c%=16;
    }
    QByteArray &bits()
    {

        quint16 blk;

        //A
        setBlock(pi,RDSGroup::Block_A);

        //B
        blk=0x2000;
        if(tp)blk|=0x0400;
        blk|=(((quint16)pty)<<5);
        if(abflag)blk|=0x0010;
        c++;c%=16;
        blk|=((quint16)c);
        setBlock(blk,RDSGroup::Block_B);

        //C
        blk=((((quint16)ba[4*c])<<8)|((quint16)ba[4*c+1]));
        setBlock(blk,RDSGroup::Block_C);

        //D
        blk=((((quint16)ba[4*c+2])<<8)|((quint16)ba[4*c+3]));
        setBlock(blk,RDSGroup::Block_D);

        if((ba[4*c]==(char)0x0D)|(ba[4*c+1]==(char)0x0D)|(ba[4*c+2]==(char)0x0D)|(ba[4*c+3]==(char)0x0D))c=-1;

        return data_chunk;
    }


    bool tp;
    quint8 pty;
    quint16 pi;
private:
    int c;
    QByteArray ba;
    bool abflag;
};

//Linkish or Appish layer?
class RDS4AGroup : public RDSGroup
{
public:
    RDS4AGroup()
    {
        //set defaults
        pi=0x9000;
        tp=false;
        pty=0;
    }
    QByteArray &bits()//loads nearest minute
    {

        quint16 blk;

        //load time of nearest minute
        QDateTime datetime_local=QDateTime::currentDateTime();
        if(datetime_local.time().second()>30)datetime_local=datetime_local.addSecs(60);
        QDateTime datetime_utc=datetime_local.toUTC();
        datetime_local.setTimeSpec(Qt::UTC);
        datetime_utc.setTimeSpec(Qt::UTC);
        qint32 utc_offset=((datetime_utc.secsTo(datetime_local))/1800);//30min offsets
        qint64 mjd=(datetime_utc.date().toJulianDay()-1)%100000;//from what I understand the -1 seems to be a problem with that JD started at noon not midnight and computer people thing it was midnight. at midnight its 0.5 , Qt rounds up to 1 while RDS takes the integer and goes to 0. Therefore -1 is needed to go from the Qt version to the RDS version
        quint16 hour=datetime_utc.time().hour();
        quint16 min=datetime_utc.time().minute();

        //A
        setBlock(pi,RDSGroup::Block_A);

        //B
        blk=0x4000;
        if(tp)blk|=0x0400;
        blk|=(((quint16)pty)<<5);
        blk|=(((quint16)(mjd>>15))&0x0003);
        setBlock(blk,RDSGroup::Block_B);

        //C
        blk=0;
        blk|=((quint16)(mjd<<1));
        blk|=((hour>>4)&0x0001);
        setBlock(blk,RDSGroup::Block_C);

        //D
        blk=0;
        blk|=((hour<<12)&0xF000);
        blk|=((min<<6)&0x0FC0);
        if(utc_offset<0)blk|=0x0020;
        utc_offset=abs(utc_offset);
        blk|=(((quint16)utc_offset)&0x001F);
        setBlock(blk,RDSGroup::Block_D);

        return data_chunk;

    }


    bool tp;
    quint8 pty;
    quint16 pi;
private:
};

//-----------end of group implimentations

//App layer
class RDS : public RDSDifferentialBiPhaseSymbolGenerator
{
    Q_OBJECT
public:

    enum {AF_NO_AF_exists=0xE0,AF_FILLER_code=0xCD};

    typedef enum {PTY_No_programme_type_or_undefined
    ,PTY_News
    ,PTY_Current_affairs
    ,PTY_Information
    ,PTY_Sport
    ,PTY_Education
    ,PTY_Drama
    ,PTY_Culture
    ,PTY_Science
    ,PTY_Varied
    ,PTY_Pop_music
    ,PTY_Rock_music
    ,PTY_Easy_listening
    ,PTY_Light_classical
    ,PTY_Serious_classical
    ,PTY_Other_music
    ,PTY_Weather
    ,PTY_Finance
    ,PTY_Childrens_programmes
    ,PTY_Social_affairs
    ,PTY_Religion
    ,PTY_Phone_in
    ,PTY_Travel
    ,PTY_Leisure
    ,PTY_Jazz_music
    ,PTY_Country_music
    ,PTY_National_music
    ,PTY_Oldies_music
    ,PTY_Folk_music
    ,PTY_Documentary
    ,PTY_Alarm_test
    ,PTY_Alarm} pty_type;

    typedef enum {PTY_RBDS_No_program_type_or_undefined
    ,PTY_RBDS_News
    ,PTY_RBDS_Information
    ,PTY_RBDS_Sports
    ,PTY_RBDS_Talk
    ,PTY_RBDS_Rock
    ,PTY_RBDS_Classic_rock
    ,PTY_RBDS_Adult_hits
    ,PTY_RBDS_Soft_rock
    ,PTY_RBDS_Top_40
    ,PTY_RBDS_Country
    ,PTY_RBDS_Oldies
    ,PTY_RBDS_Soft
    ,PTY_RBDS_Nostalgia
    ,PTY_RBDS_Jazz
    ,PTY_RBDS_Classical
    ,PTY_RBDS_Rhythm_and_blues
    ,PTY_RBDS_Soft_rhythm_and_blues
    ,PTY_RBDS_Language
    ,PTY_RBDS_Religious_music
    ,PTY_RBDS_Religious_talk
    ,PTY_RBDS_Personality
    ,PTY_RBDS_Public
    ,PTY_RBDS_College
    ,PTY_RBDS_Spanish_Talk
    ,PTY_RBDS_Spanish_Music
    ,PTY_RBDS_Hip_Hop
    ,PTY_RBDS_Unassigned_1
    ,PTY_RBDS_Unassigned_2
    ,PTY_RBDS_Weather
    ,PTY_RBDS_Emergency_test
    ,PTY_RBDS_Emergency} pty_rbds_type;

    explicit RDS(QObject *parent = 0);
    void set_grouppercentages(double grp0Awantedbandwidthusage,double grp2Awantedbandwidthusage);
    void reset();

    quint16 get_pi(){return grp0A.pi;}
    QString get_ps(){return grp0A.get_ps();}
    QString get_rt(){return grp2A.get_rt();}
    pty_type get_pty(){return (pty_type)grp0A.pty;}
    double get_altfreq1(){return grp0A.get_altfreq1();}
    double get_altfreq2(){return grp0A.get_altfreq2();}
    bool get_stereo(){return grp0A.stereo;}
    bool get_artificial_head(){return grp0A.artificial_head;}
    bool get_compressed(){return grp0A.compressed;}
    bool get_dynamic_pty(){return grp0A.dynamic_pty;}
    bool get_tp(){return grp0A.tp;}
    bool get_ta(){return grp0A.ta;}
    bool get_ms(){return grp0A.ms;}
    bool get_rt_enable(){if(grp2Awantedbandwidthusage>0)return true;return false;}

    void set_pi(quint16 pi){grp0A.pi=pi;grp2A.pi=pi;grp4A.pi=pi;}
    void set_ps(QString ps){grp0A.set_ps(ps);}
    void set_rt(QString rt){grp2A.set_rt(rt);}
    void set_pty(pty_type pty){grp0A.pty=(quint8)pty;grp2A.pty=(quint8)pty;grp4A.pty=(quint8)pty;}
    void set_altfreq1(double freq_mhz){grp0A.set_altfreq1(freq_mhz);}
    void set_altfreq2(double freq_mhz){grp0A.set_altfreq2(freq_mhz);}
    void set_stereo(bool set){grp0A.stereo=set;}
    void set_artificial_head(bool set){grp0A.artificial_head=set;}
    void set_compressed(bool set){grp0A.compressed=set;}
    void set_dynamic_pty(bool set){grp0A.dynamic_pty=set;}
    void set_tp(bool set){grp0A.tp=set;grp2A.tp=set;grp4A.tp=set;}
    void set_ta(bool set){grp0A.ta=set;}
    void set_ms(bool set){grp0A.ms=set;}
    void set_rt_enable(bool set)
    {
        if(set)set_grouppercentages(0.8,0.2);
         else set_grouppercentages(1.0,0.0);
    }


    //todo change these to getters and setters to match other things
    int clocktimeoffset;//to allow the user to adjust the delay caused by the soundcard buffer
    bool ct_enabled;
    bool rbds;

signals:

public slots:
private slots:
    void wantmoregroups_slot();
private:

    typedef enum {SEL_0AGroup,SEL_2AGroup} selected_group_type;

    RDS0AGroup grp0A;
    RDS2AGroup grp2A;
    RDS4AGroup grp4A;

    double grp0Atxsum;
    double grp0Awantedbandwidthusage;
    double grp2Atxsum;
    double grp2Awantedbandwidthusage;
    double txtrunccnt;

};

#endif // RDS_H
