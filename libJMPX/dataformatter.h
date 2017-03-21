#ifndef DATAFORMATTER_H
#define DATAFORMATTER_H

#include <QObject>
#include <QVector>

#include "../viterbi-xukmin/viterbi.h"

class PuncturedCode
{
public:
    void depunture_soft_block(QByteArray &block,int pattern, bool reset=true);
    void punture_soft_block(QByteArray &block,int pattern, bool reset=true);
    void punture_block(QVector<int> &block, int pattern, bool reset=true);
    PuncturedCode();
private:
    int punture_ptr;
    int depunture_ptr;
};

class CRC16 //this seems to be called GENIBUS not CCITT
{
public:
    quint16 crc;
    CRC16(){}
    quint16 calcusingbits(int *bits,int numberofbits)
    {
        crc = 0xFFFF;
        int crc_bit;
        for(int i=0; i<numberofbits; i++)//we are finished when all bits of the message are looked at
        {
            //crc_bit = (crc >> 15) & 1;//bit of crc we are working on. 15=poly order-1
            //crc <<= 1;//shift to next crc bit (have to do this here before Gate A does its thing)
            //if(crc_bit ^ bits[i])crc = crc ^ 0x1021;//add to the crc the poly mod2 if crc_bit + block_bit = 1 mod2 (0x1021 is the ploy with the first bit missing so this means x^16+x^12+x^5+1)

            //differnt endiness
            crc_bit = crc & 1;//bit of crc we are working on. 15=poly order-1
            crc >>= 1;//shift to next crc bit (have to do this here before Gate A does its thing)
            if(crc_bit ^ bits[i])crc = crc ^ 0x8408;//(0x8408 is reversed 0x1021)add to the crc the poly mod2 if crc_bit + block_bit = 1 mod2 (0x1021 is the ploy with the first bit missing so this means x^16+x^12+x^5+1)

        }
        return ~crc;
    }
    quint16 calcusingbytes(char *bytes,int numberofbytes)
    {
        crc = 0xFFFF;
        int crc_bit;
        int message_bit;
        int message_byte;
        for(int i=0; i<numberofbytes; i++)//we are finished when all bits of the message are looked at
        {
            message_byte=bytes[i];
            for(int k=0;k<8;k++)
            {

                //message_bit=(message_byte>>7)&1;
                //message_byte<<=1;
                //crc_bit = (crc >> 15) & 1;//bit of crc we are working on. 15=poly order-1
                //crc <<= 1;//shift to next crc bit (have to do this here before Gate A does its thing)
                //if(crc_bit ^ message_bit)crc = crc ^ 0x1021;//add to the crc the poly mod2 if crc_bit + block_bit = 1 mod2 (0x1021 is the ploy with the first bit missing so this means x^16+x^12+x^5+1)

                //differnt endiness
                message_bit=message_byte&1;
                message_byte>>=1;
                crc_bit = crc & 1;//bit of crc we are working on. 15=poly order-1
                crc >>= 1;//shift to next crc bit (have to do this here before Gate A does its thing)
                if(crc_bit ^ message_bit)crc = crc ^ 0x8408;//(0x8408 is reversed 0x1021)add to the crc the poly mod2 if crc_bit + block_bit = 1 mod2 (0x1021 is the ploy with the first bit missing so this means x^16+x^12+x^5+1)

            }
        }
        return ~crc;
    }
    quint16 calcusingbytesotherendines(char *bytes,int numberofbytes)
    {
        crc = 0xFFFF;
        int crc_bit;
        int message_bit;
        int message_byte;
        for(int i=0; i<numberofbytes; i++)//we are finished when all bits of the message are looked at
        {
            message_byte=bytes[i];
            for(int k=0;k<8;k++)
            {

                message_bit=(message_byte>>7)&1;
                message_byte<<=1;
                crc_bit = (crc >> 15) & 1;//bit of crc we are working on. 15=poly order-1
                crc <<= 1;//shift to next crc bit (have to do this here before Gate A does its thing)
                if(crc_bit ^ message_bit)crc = crc ^ 0x1021;//add to the crc the poly mod2 if crc_bit + block_bit = 1 mod2 (0x1021 is the ploy with the first bit missing so this means x^16+x^12+x^5+1)

                //differnt endiness
                //message_bit=message_byte&1;
                //message_byte>>=1;
                //crc_bit = crc & 1;//bit of crc we are working on. 15=poly order-1
                //crc >>= 1;//shift to next crc bit (have to do this here before Gate A does its thing)
                //if(crc_bit ^ message_bit)crc = crc ^ 0x8408;//(0x8408 is reversed 0x1021)add to the crc the poly mod2 if crc_bit + block_bit = 1 mod2 (0x1021 is the ploy with the first bit missing so this means x^16+x^12+x^5+1)

            }
        }
        return ~crc;
    }
    quint16 calcusingbytes(QByteArray data)
    {
        return calcusingbytes(data.data(),data.size());
    }
    quint16 calcusingbytesotherendines(QByteArray data)
    {
        return calcusingbytesotherendines(data.data(),data.size());
    }
};


class TSlip
{
public:
    TSlip();
    QByteArray &EscapePacket(uchar packet_type, const QByteArray &data);
    void NewRxData();
    bool GotRXPacket(const QByteArray &data);

    QByteArray RxPacket;
    uchar RxPacket_type;

    enum ESC_CHARS {
        END     =   0xC0,    /* indicates end of packet */
        ESC     =   0xDB,    /* indicates byte stuffing */
        ESC_END =   0xDC,    /* ESC ESC_END means END data byte */
        ESC_ESC =   0xDE,    /* ESC ESC_ESC means ESC data byte */
    };

    quint32 goodBytes_cnt;
    quint32 badBytes_cnt;

private:

    bool escapeing;
    const QByteArray *privateRXdataPtr;
    int rxi;
    bool LastGotRXPacketWasTrue;

    QByteArray tmppkt;


    CRC16 crc16;
};


class AeroLInterleaver
{
public:
    AeroLInterleaver();
    void setSize(int N);
    QVector<int> &interleave(QVector<int> &block);
    QVector<int> &deinterleave(QVector<int> &block);
    QVector<int> &deinterleave(QVector<int> &block,int cols);//deinterleaves with a fewer number of cols than the block has
    QVector<int> &getBlock(){return matrix;}
private:
    QVector<int> matrix;
    int M;
    int N;
    QVector<int> interleaverowpermute;
    QVector<int> interleaverowdepermute;
};

class AeroLScrambler
{
public:
    AeroLScrambler()
    {
        reset();
    }
    void reset()
    {
        int tmp[]={1,1,0,1,0,0,1,0,1,0,1,1,0,0,1,-1};
        state.clear();
        for(int i=0;tmp[i]>=0;i++)state.push_back(tmp[i]);
    }
    void update(QVector<int> &data)
    {
        for(int j=0;j<data.size();j++)
        {
            int val0=state[0]^state[14];
            data[j]^=val0;
            for(int i=state.size()-1;i>0;i--)state[i]=state[i-1];
            state[0]=val0;
        }
    }
private:
    QVector<int> state;
};

class DataFormatter
{
public:
    enum Mode {none=-1,mode0=0,mode1=1,mode2=2,mode3=3,mode4=4};

    typedef enum modeCodes //hamming distance of 8
    {
        mode_code_15=0b1000000000000000,
        mode_code_14=0b0000000011111111,
        mode_code_13=0b1000111100001111,
        mode_code_12=0b0000111111110000,
        mode_code_11=0b1011001100110011,
        mode_code_10=0b0011001111001100,
        mode_code_9=0b1011110000111100,
        mode_code_8=0b0011110011000011,
        mode_code_7=0b1101010101010101,
        mode_code_6=0b0101010110101010,
        mode_code_5=0b1101101001011010,
        mode_code_4=0b0101101010100101,
        mode_code_3=0b1110011001100110,
        mode_code_2=0b0110011010011001,
        mode_code_1=0b1110100101101001,
        mode_code_0=0b0110100110010110
    }modeCodes;

    enum PacketType {PacketType_RDS=0,PacketType_OPUS=1};

    DataFormatter();
    ~DataFormatter();
    void setMode(DataFormatter::Mode mode);
    DataFormatter::Mode getMode(){return mode;}

    void setMaxbufferSize(int size){max_tmpbuffer_size=size;}//in bytes

    int getNumberOfBitsInFrame(){return NumberOfBitsInFrame;}
    int getBufferfreesize(){return qMax(max_tmpbuffer_size-tmpbuffer.size()-1,0);}

    double getBufferUsagePercentage(){return ((double)tmpbuffer.size())/((double)max_tmpbuffer_size);}

    bool pushPacket(uchar packet_type, const QByteArray &packet);
    bool pushPacket(uchar packet_type,const uchar *packet, int size)
    {
        return pushPacket(packet_type,QByteArray((char*)packet,size));
    }

    QVector<int> &getFrame();

    void clearBuffer(){tmpbuffer.clear();}


private:

    QVector<int> &createFrame(QByteArray &frame_msg);
    int getNumberOfBytesInUnencodedMessage(){return NumberOfBytesInUnencodedMessage;}

    QVector<int> frame;
    QVector<int> block;
    AeroLInterleaver leaver;
    AeroLScrambler scrambler;
    ViterbiCodec *convolcodec;
    PuncturedCode pc;
    QVector<int> preamble;

    Mode mode;

    modeCodes modecode;
    QVector<int> modecodesbits[16];

    int max_tmpbuffer_size;

    TSlip slip;
    QByteArray tmpbuffer;

    int NumberOfBytesInUnencodedMessage;
    int NumberOfBitsInBlock;//info only
    int NumberOfBitsInFrame;//info and header and uw
    int NumberOfBitsInHeader;
    int NumberOfFramesToReserve;

    bool setPreamble(quint64 bitpreamble,int len);
};

#endif // DATAFORMATTER_H
