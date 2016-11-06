#include "rds.h"

#include <QThread>
#include <QElapsedTimer>
#include <QTimer>

//----PHY layer things

RDSDifferentialBiPhaseSymbolGenerator::~RDSDifferentialBiPhaseSymbolGenerator()
{
#ifdef FASTFIRMETHOD
    delete fir;
#endif
}

RDSDifferentialBiPhaseSymbolGenerator::RDSDifferentialBiPhaseSymbolGenerator(QObject *parent) : QObject(parent)
{

    double grossbitrate=2375.0;
    double samplerate=192000;
    double alpha=1.0;
    int firlen=6*samplerate/grossbitrate; //about 6 symbols overlaping produces good roll off. affects cpu usage.

    sp.setFreq(grossbitrate,samplerate);
    rrc.create(grossbitrate,firlen,alpha,samplerate);

#ifdef FASTFIRMETHOD
    rrc.scalepoints(6.1);
    fir = new FastFIRFilter(rrc.Points);
#endif

#ifndef FASTFIRMETHOD
    symbolbuffsize=2*((int)(firlen*grossbitrate/samplerate))+2;
    symbolbuffcntr.resize(symbolbuffsize);
    cosampsymbolbuff.resize(symbolbuffsize);
    symbolbuffcntr.fill(0);
    cosampsymbolbuff.fill(0);
    symbolbuffptr=0;
#endif

    //differential encoder and the funny mirror image tx that RDS does
    onoffcntr=0;
    lastch=0;

    //setup cycle buffer for data to send
    buffer.resize(2400);//2400 -> PHY will ask about every second for about 23 more groups
    buffer_head=0;
    buffer_tail=0;
    buffer_used=0;
    buffer_size=buffer.size();

#ifdef TIMERTEST
    nanoSecsum=0;
    nanoseccnt=0;
#endif

}

void RDSDifferentialBiPhaseSymbolGenerator::reset()
{
    buffer_mutex.lock();

    buffer_head=0;
    buffer_tail=0;
    buffer_used=0;

    buffer_mutex.unlock();
}

bool RDSDifferentialBiPhaseSymbolGenerator::havemorebits(QByteArray &bits)
{
    buffer_mutex.lock();
    if(bits.size()>=(buffer_size-buffer_used-1))
    {
        buffer_mutex.unlock();
        qDebug()<<"bits rejected: not enough buffer space bits.size() ="<<bits.size()<<" and (buffer_size-buffer_used-1) ="<<(buffer_size-buffer_used-1);
        return false;
    }
    for(int i=0;i<bits.size();i++)
    {
        buffer[buffer_head]=bits[i];
        buffer_head++;buffer_head%=buffer_size;
        if(buffer_head==buffer_tail)
        {
            buffer_head--;
            if(buffer_head<0)buffer_head+=buffer_size;
            buffer_mutex.unlock();
            qDebug()<<"RDS buffer overflow";
            return false;
        }
        buffer_used++;
    }
    buffer_mutex.unlock();
    return true;
}

//This should be run from another thread than the thread that made the instance of this class.
//this does the job of the "Differential encoder" and the "Biphase symbol generator"
//in fig 1 of the RDS specs. This is run a lot.
void RDSDifferentialBiPhaseSymbolGenerator::FIRFilterImpulses(int size)
{

#ifdef TIMERTEST
    QElapsedTimer timer;
    timer.start();
#endif

    bool sentwantmoregroups=false;

    buffer_mutex.lock();

    if(outputsignal.size()<size)outputsignal.resize(size);
    int ch;
    double cosamp;
    for(int i=0;i<size;i++)
    {

#ifdef FASTFIRMETHOD
        outputsignal[i]=0;
#endif

        sp.nextFrame();
        //if on next frame we need to send a symbol
        if(sp.ifPassesPointNextTime())
        {

            onoffcntr++;onoffcntr%=2;
            if(onoffcntr)
            {

                //get the next symbol (constelation point) for modulation

                //get another bit from the buffer. If we cant then just make a random one.
                if(buffer_head!=buffer_tail)
                {
                    ch=buffer[buffer_tail];
                    buffer_tail++;buffer_tail%=buffer_size;
                    buffer_used--;

                }
                 else ch=rand()%2; //there are no bits left, eek. just use a reandom bit

                //if we are low on bits ask for more
                if((buffer_size>(2*buffer_used))&&(!sentwantmoregroups))
                {
                    emit wantmoregroups();
                    sentwantmoregroups=true;
                }

                //ch=rand()%2;

                //this is the differential encoder
                ch^=lastch;
                lastch=ch;

            } else ch=1-lastch;//this is like 2 interleaved streams at 1187.5 baud each mirrored. This causes RDS's suppressed carrier. In the RDS specs fig 1 this is the delay td/2 and the op amp.

            //{0,1}->{-1,1} mapping
            if(ch)cosamp=1;
             else cosamp=-1;

#ifdef FASTFIRMETHOD
            pthis=sp.FractionOfSampleItPassesBy;
            pnext=1-pthis;
            pthis*=cosamp;
            pnext*=cosamp;
#endif


#ifndef FASTFIRMETHOD
            //work out intapolation between samples
            double pthis=sp.FractionOfSampleItPassesBy;
            double pnext=1-pthis;
            symbolbuffcntr[symbolbuffptr]=0; //this
            cosampsymbolbuff[symbolbuffptr]=cosamp*pthis;
            symbolbuffptr++;symbolbuffptr%=symbolbuffsize;
            symbolbuffcntr[symbolbuffptr]=-1; //next
            cosampsymbolbuff[symbolbuffptr]=cosamp*pnext;
            symbolbuffptr++;symbolbuffptr%=symbolbuffsize;
#endif

        }

#ifdef FASTFIRMETHOD
        if(pthis)
        {
            outputsignal[i]=pthis;
            pthis=0;
        }
         else if(pnext)
         {
             outputsignal[i]=pnext;
             pnext=0;
         }
#endif


#ifndef FASTFIRMETHOD
        int firsize=rrc.Points.size();
        double cossum=0;
        for(int j=0;j<symbolbuffsize;j++)
        {
            if(symbolbuffcntr[j]<firsize)
            {
                if(symbolbuffcntr[j]>=0)
                {
                    cossum+=5.0*cosampsymbolbuff[j]*rrc.Points[symbolbuffcntr[j]];
                }
                symbolbuffcntr[j]++;
            }
        }

        outputsignal[i]=cossum;
#endif

    }



    buffer_mutex.unlock();


#ifdef FASTFIRMETHOD
    fir->Update(outputsignal.data(),size);
#endif

#ifdef TIMERTEST
    nanoSecsum+=timer.nsecsElapsed();
    nanoseccnt++;
    qDebug() << "Elapsed time ave =" << ((double)nanoSecsum)/((double)nanoseccnt) << "nanoseconds";
#endif

}

//PHY layer stop

//Data layer stuff

//see figure B.2 of the RDS specs for this
//crc are the things in the square blocks with 0 to 9 written on them.
//the plus with the circles around them are + mod2 and are implimented here with xor (^)
//the + mod2 connected to gate A is crc_bit ^ message_bit
//message_bit is the "message input"
//crc_bit is bit 9 of the crc
//Gate A does the crc = crc ^ 0x1B9
quint16 RDSGroup::crc(quint16 message)
{
    quint16 crc = 0;
    int message_bit;
    int crc_bit;
    for(int i=0; i<16; i++)//we are finished when all bits of the message are looked at
    {
        message_bit = (message >> 15) & 1;//bit of message we are working on. 15=block length-1
        crc_bit = (crc >> 9) & 1;//bit of crc we are working on. 9=poly order-1

        crc <<= 1;//shift to next crc bit (have to do this here before Gate A does its thing)
        message <<= 1;//shift to next message bit

        if(crc_bit ^ message_bit)crc = crc ^ 0x1B9;//add to the crc the poly mod2 if crc_bit + block_bit = 1 mod2

    }
    return (crc&0x03FF);//ditch things above 10bits
}

void RDSGroup::setBlock(quint16 message,offset_type offset)
{
    int bitpointer=0;
    switch(offset)
    {
    case Block_A:
        bitpointer=26*0;
        break;
    case Block_B:
        bitpointer=26*1;
        break;
    case Block_C:
        bitpointer=26*2;
        break;
    case Block_C_prime:
        bitpointer=26*2;
        break;
    case Block_D:
        bitpointer=26*3;
        break;
    case Block_E:
        //???
        qDebug()<<"Not sure what to do with E offset";
        return;
        break;
    default:
        qDebug()<<"Unknown offset type";
        return;
    }

    //calc crc + offset of message
    quint16 crc_plus_offset=crc(message)^(quint16)offset;

    //add message
    for(int i=0;i<16;i++)
    {
        data_chunk[bitpointer] = ((message>>15)&1);
        message<<=1;
        bitpointer++;
    }

    //add crc + offset
    for(int i=0;i<10;i++)
    {
        data_chunk[bitpointer] = ((crc_plus_offset>>9)&1);
        crc_plus_offset<<=1;
        bitpointer++;
    }

}

//Data layer stop


//App layer

RDS::RDS(QObject *parent) : RDSDifferentialBiPhaseSymbolGenerator(parent)
{

    grp0Awantedbandwidthusage=-1;
    grp2Awantedbandwidthusage=-1;
    grp5Awantedbandwidthusage=-1;

    //default rates. no 5a groups
    //grp0Awantedbandwidthusage 80%
    //grp2Awantedbandwidthusage 20%
    //grp5Awantedbandwidthusage 00%
    set_grouppercentages(0.8,0.2,0.0);

    clocktimeoffset=0;

    ct_enabled=true;

    rbds=false;

    connect(this,SIGNAL(wantmoregroups()),this,SLOT(wantmoregroups_slot()),Qt::QueuedConnection);

}

void RDS::set_grouppercentages(double _grp0Awantedbandwidthusage, double _grp2Awantedbandwidthusage, double _grp5Awantedbandwidthusage)
{
    if((_grp0Awantedbandwidthusage==grp0Awantedbandwidthusage)&&(_grp2Awantedbandwidthusage==grp2Awantedbandwidthusage)&&(_grp5Awantedbandwidthusage==grp5Awantedbandwidthusage))return;
    grp0Awantedbandwidthusage=_grp0Awantedbandwidthusage;
    grp2Awantedbandwidthusage=_grp2Awantedbandwidthusage;
    grp5Awantedbandwidthusage=_grp5Awantedbandwidthusage;

    double scalling=1.0/(grp0Awantedbandwidthusage+grp2Awantedbandwidthusage+grp5Awantedbandwidthusage);
    grp0Awantedbandwidthusage*=scalling;
    grp2Awantedbandwidthusage*=scalling;
    grp5Awantedbandwidthusage*=scalling;
    txtrunccnt=0;
    grp0Atxsum=0;
    grp2Atxsum=0;
    grp5Atxsum=0;
}

void RDS::reset()
{
    txtrunccnt=0;
    grp0Atxsum=0;
    grp2Atxsum=0;
    grp5Atxsum=0;
    RDSDifferentialBiPhaseSymbolGenerator::reset();
}

void RDS::wantmoregroups_slot()
{

    //work out how much is needed and how much has already been buffered
    buffer_mutex.lock();
    int tbuffer_used=buffer_used;
    buffer_mutex.unlock();
    int numberwanted=((buffer_size-tbuffer_used-2)/104);
    if(numberwanted<=0)return;
    int numbergot=(tbuffer_used/=104);

    //time till next min
    QTime time=QTime::currentTime();
    QTime time2;
    time2.setHMS(time.hour(),time.minute()+1,0);
    int msecs_start=time.msecsTo(time2);//seconds till the next min for the first group we will return from this function

    //estimate time till clock insertion
    int groupstillsendclockgroup=((double)msecs_start)/1000.0*1187.5/104.0;
    groupstillsendclockgroup-=numbergot;//number already waiting
    groupstillsendclockgroup-=70;//some buffering is done with the sound card
    groupstillsendclockgroup-=clocktimeoffset;//to allow the user to fine tune clock sending time

    //keep estimate time to within 0 to 684 groups
    while(groupstillsendclockgroup<0)groupstillsendclockgroup+=685;
    groupstillsendclockgroup%=685;

    //in case we misjudged the time till clock insertion and passed it by last time
    bool groupsfromsendclocksign=true;
    if((685-groupstillsendclockgroup)<groupstillsendclockgroup)groupsfromsendclocksign=false;
    static bool lastgroupsfromsendclocksign=groupsfromsendclocksign;
    if((lastgroupsfromsendclocksign&&!groupsfromsendclocksign)&&txtrunccnt>40)
    {
        //qDebug()<<"misjudged time till clock insertion. sending clock now!!";
        groupstillsendclockgroup=0;
    }
    lastgroupsfromsendclocksign=groupsfromsendclocksign;

    //qDebug()<<msecs_start<<" send clock in "<<groupstillsendclockgroup<<" groups";
    //qDebug()<<"numberwanted"<<numberwanted;

    while(numberwanted>0)
    {

        //send the clock 4A group at right time
        if((groupstillsendclockgroup==0)&&ct_enabled)
        {
            //qDebug()<<"sent clock";
            havemorebits(grp4A.bits());
            numberwanted--;
            groupstillsendclockgroup--;
            lastgroupsfromsendclocksign=false;//reset clock time insertion estimate misjudgedment
            continue;
        }

        //select what group to send based on bandwidth usage
        selected_group_type selection=SEL_0AGroup;
        txtrunccnt+=1.0;
        if((grp0Atxsum/txtrunccnt)<grp0Awantedbandwidthusage)selection=SEL_0AGroup;
        if((grp2Atxsum/txtrunccnt)<grp2Awantedbandwidthusage)selection=SEL_2AGroup;
        if((grp5Atxsum/txtrunccnt)<grp5Awantedbandwidthusage)selection=SEL_5AGroup;

        switch(selection)
        {
        case SEL_0AGroup:
            if(!havemorebits(grp0A.bits())){grp0A.rewind();return;}
            grp0Atxsum+=1.0;
            break;
        case SEL_2AGroup:
            if(!havemorebits(grp2A.bits())){grp2A.rewind();return;}
            grp2Atxsum+=1.0;
            break;
        case SEL_5AGroup:
            if(!havemorebits(grp5A.bits())){grp5A.rewind();return;}
            grp5Atxsum+=1.0;
            break;
        default:
            qDebug()<<"No group selected for txing";
            return;
        }

        if(txtrunccnt>100.0)
        {
            grp0Atxsum/=2.0;
            grp2Atxsum/=2.0;
            grp5Atxsum/=2.0;
            txtrunccnt/=2.0;
        }

        //an example of what could be done in the future
        /*grouppointer++;grouppointer%=grps.size();
        havemorebits(grps[grouppointer].data_chunk);

        //if group is not to be repeated then ditch it
        if(!grps[grouppointer].repeat)
        {
            grps.removeAt(grouppointer);
            grouppointer--;
            if(grouppointer<0)grouppointer=grps.size()-1;
        }*/

        numberwanted--;
        groupstillsendclockgroup--;

    }

}


