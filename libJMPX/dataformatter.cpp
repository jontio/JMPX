#include "dataformatter.h"
#include <assert.h>
#include <QDebug>

void PuncturedCode::depunture_soft_block(QByteArray &block,int pattern,bool reset)
{
    assert(pattern>=2);
    QByteArray tblock;tblock.reserve(block.size()+block.size()/pattern+1);
    if(reset)depunture_ptr=0;
    for(int i=0;i<block.size();i++)
    {
        depunture_ptr++;
        tblock.push_back(block.at(i));
        if(depunture_ptr>=pattern-1)tblock.push_back(128);
        depunture_ptr%=(pattern-1);
    }

    block=tblock;

}
void PuncturedCode::punture_soft_block(QByteArray &block,int pattern,bool reset)
{
    assert(pattern>=2);
    QByteArray tblock;tblock.reserve(block.size());
    if(reset)punture_ptr=0;
    for(int i=0;i<block.size();i++)
    {
        punture_ptr++;
        if(punture_ptr<(pattern))tblock.push_back(block.at(i));
        punture_ptr%=pattern;
    }
    block=tblock;
}
PuncturedCode::PuncturedCode()
{
    punture_ptr=0;
    depunture_ptr=0;
}
void PuncturedCode::punture_block(QVector<int> &block,int pattern,bool reset)
{
    assert(pattern>=2);
    QVector<int> tblock;tblock.reserve(block.size());
    if(reset)punture_ptr=0;
    for(int i=0;i<block.size();i++)
    {
        punture_ptr++;
        if(punture_ptr<(pattern))tblock.push_back(block.at(i));
        punture_ptr%=pattern;
    }
    block=tblock;
}

//--slip
TSlip::TSlip()
{
    escapeing=false;
    RxPacket.reserve(1024);
    LastGotRXPacketWasTrue=true;
    goodBytes_cnt=0;
    badBytes_cnt=0;
}
void TSlip::NewRxData()
{
    rxi=0;
}
bool TSlip::GotRXPacket(const QByteArray &data)
{

    privateRXdataPtr=&data;

    quint16 crc16_rec;

    if(LastGotRXPacketWasTrue)
    {
        RxPacket.resize(0);
        LastGotRXPacketWasTrue=false;
    }
    while(rxi<privateRXdataPtr->count())
    {


        if(((uchar)(privateRXdataPtr->at(rxi)))==END)
        {
            escapeing=false;
            rxi++;
            if(RxPacket.count()>1)
            {

                uchar type=0;
                uchar hibyte=0;
                uchar lowbyte=0;
                int pkt_size=0;

                if(RxPacket.count()>3)type=   (((uchar)(RxPacket.at(RxPacket.count()-3))));
                if(RxPacket.count()>2)hibyte= (((uchar)(RxPacket.at(RxPacket.count()-2))));
                if(RxPacket.count()>1)lowbyte=(((uchar)(RxPacket.at(RxPacket.count()-1))));

                //deal with short hedders
                if(!(lowbyte&0x80))
                {
                    if(RxPacket.count()>3)
                    {
                        pkt_size=RxPacket.size()-3;
                        crc16.calcusingbytes(RxPacket.data(),RxPacket.size()-2);
                        crc16_rec=((((quint16)hibyte)<<8)&0xFF00)|(((quint16)lowbyte)&0x00FF);
                        crc16_rec|=0x0080;
                        crc16.crc|=0x0080;
                        if(crc16.crc==crc16_rec)
                        {
                            //qDebug()<<"type="<<type;
                        }
                    } else {pkt_size=0;crc16_rec=0;crc16.crc=1;}
                }
                 else
                 {
                    pkt_size=RxPacket.size()-1;
                    crc16_rec=(((quint16)lowbyte)&0x00FF);
                    quint16 crcbackup=crc16.calcusingbytes(RxPacket.data(),RxPacket.size()-1);

                    //search types from 1 to 16 inclusive as these ones are allowed to have the 8 byte header
                    for(int k=1;k<=16;k++)
                    {
                        crc16.crc=crcbackup;
                        crc16.calcusingbytes_update(k);
                        crc16_rec&=0x007F;
                        crc16.crc&=0x007F;
                        if(crc16.crc==crc16_rec)
                        {
                            //qDebug()<<k;
                            type=k;
                            break;
                        }
                    }

                 }

                //here it appears we have a normal header
                if(crc16.crc==crc16_rec)
                {
                    goodBytes_cnt+=RxPacket.size();
                    RxPacket_type=type;
                    RxPacket.resize(pkt_size);
                    LastGotRXPacketWasTrue=true;
                    //RxPacket.push_back(char(0));
                    return true;
                } else badBytes_cnt+=RxPacket.size();

            }
            RxPacket.resize(0);
            continue;
        }

        if(escapeing)
        {
            switch((uchar)(privateRXdataPtr->at(rxi)))
            {
            case ESC_END:
                    RxPacket.push_back(END);
                    break;
            case ESC_ESC:
                    RxPacket.push_back(ESC);
                    break;
            /*case END:
                    if(RxPacket.count())
                    {
                        rxi++;
                        LastGotRXPacketWasTrue=true;
                        RxPacket.push_back(char(0));
                        escapeing=false;
                        return true;
                    }
                    break;*/
            default:
                    RxPacket.push_back(privateRXdataPtr->at(rxi));
            }
            escapeing=false;
        }
         else
            switch((uchar)(privateRXdataPtr->at(rxi)))
            {
            /*case END:
                    if(RxPacket.count())
                    {
                        rxi++;
                        LastGotRXPacketWasTrue=true;
                        RxPacket.push_back(char(0));
                        return true;
                    }
                    break;*/
            case ESC:
                    escapeing=true;
                    break;
            default:
                    RxPacket.push_back(privateRXdataPtr->at(rxi));
            }

        rxi++;
     }
    return false;
}
QByteArray &TSlip::EscapePacket(uchar packet_type,const QByteArray &data)
{

    //calc crc. include type. the 8th bit of the 16 bit crc is the compression flag
    uchar hibyte,lowbyte;
    crc16.calcusingbytes(data);
    crc16.calcusingbytes_update(packet_type);
    hibyte=(uchar)((crc16.crc&0xFF00)>>8);
    lowbyte=(uchar)(crc16.crc&0x007F);

    //decide whether or not to use short headder
    //packet types 1 to 16 inclusive are allowed to use this headder type
    bool shortning=true;
    if(packet_type>16||packet_type==0)
    {
        shortning=false;

    } else lowbyte|=0x0080;//flag as short

    tmppkt.reserve(data.size()+50);
    tmppkt.resize(0);

 //   tmppkt.push_back(END);//not needed as log as we send END when idling


    for(int i=0;i<data.count();i++)
    {
        switch((uchar)(data.at(i)))
        {
        case END:
            tmppkt.push_back(ESC);
            tmppkt.push_back(ESC_END);
            break;
        case ESC:
            tmppkt.push_back(ESC);
            tmppkt.push_back(ESC_ESC);
            break;
        default:
            tmppkt.push_back((uchar)data.at(i));
        }
    }

    //if the packet is normal
    if(!shortning)
    {

        switch(packet_type)
        {
        case END:
            tmppkt.push_back(ESC);
            tmppkt.push_back(ESC_END);
            break;
        case ESC:
            tmppkt.push_back(ESC);
            tmppkt.push_back(ESC_ESC);
            break;
        default:
            tmppkt.push_back(packet_type);
        }

        switch(hibyte)
        {
        case END:
            tmppkt.push_back(ESC);
            tmppkt.push_back(ESC_END);
            break;
        case ESC:
            tmppkt.push_back(ESC);
            tmppkt.push_back(ESC_ESC);
            break;
        default:
            tmppkt.push_back(hibyte);
        }

    }

    switch(lowbyte)
    {
    case END:
            tmppkt.push_back(ESC);
            tmppkt.push_back(ESC_END);
            break;
    case ESC:
            tmppkt.push_back(ESC);
            tmppkt.push_back(ESC_ESC);
            break;
    default:
            tmppkt.push_back(lowbyte);
    }

    tmppkt.push_back(END);

    return tmppkt;
}
//


//--------- interleaver

AeroLInterleaver::AeroLInterleaver()
{
    M=64;

    interleaverowpermute.resize(M);
    interleaverowdepermute.resize(M);

    //this is 1-1 and onto
    for(int i=0;i<M;i++)
    {
        interleaverowpermute[(i*27)%M]=i;
        interleaverowdepermute[i]=(i*27)%M;
    }
    setSize(6);
}
void AeroLInterleaver::setSize(int _N)
{
    if(_N<1)return;
    N=_N;
    matrix.resize(M*N);
}
QVector<int> &AeroLInterleaver::interleave(QVector<int> &block)
{
    assert(block.size()==(M*N));
    int k=0;
    for(int i=0;i<M;i++)
    {
        for(int j=0;j<N;j++)
        {
            int entry=interleaverowpermute[i]+M*j;
            assert(entry<block.size());
            assert(k<matrix.size());
            matrix[k]=block[entry];
            k++;
        }
    }
    return matrix;
}
QVector<int> &AeroLInterleaver::deinterleave(QVector<int> &block)
{
    assert(block.size()==(M*N));
    int k=0;
    for(int j=0;j<N;j++)
    {
        for(int i=0;i<M;i++)
        {
            int entry=interleaverowdepermute[i]*N+j;
            assert(entry<block.size());
            assert(k<matrix.size());
            matrix[k]=block[entry];
            k++;
        }
    }
    return matrix;
}
QVector<int> &AeroLInterleaver::deinterleave(QVector<int> &block,int cols)
{
    assert(cols<=N);
    assert(block.size()>=(M*cols));
    int k=0;
    for(int j=0;j<cols;j++)
    {
        for(int i=0;i<M;i++)
        {
            int entry=interleaverowdepermute[i]*cols+j;
            assert(entry<block.size());
            assert(k<matrix.size());
            matrix[k]=block[entry];
            k++;
        }
    }
    return matrix;
}

//- end interleaver

DataFormatter::DataFormatter()
{
    //ViterbiCodec is not Qt so needs deleting when finished
    std::vector<int> polynomials;
    polynomials.push_back(109);
    polynomials.push_back(79);
    convolcodec=new ViterbiCodec(7, polynomials);
    convolcodec->setPaddingLength(24);

    setMaxbufferSize(10000);

    //save mode type ransmission
    QVector<int> mode_bits;
    mode_bits.clear();for(int i=15;i>=0;i--)mode_bits.push_back((mode_code_0>>i)&1);modecodesbits[0]=mode_bits;
    mode_bits.clear();for(int i=15;i>=0;i--)mode_bits.push_back((mode_code_1>>i)&1);modecodesbits[1]=mode_bits;
    mode_bits.clear();for(int i=15;i>=0;i--)mode_bits.push_back((mode_code_2>>i)&1);modecodesbits[2]=mode_bits;
    mode_bits.clear();for(int i=15;i>=0;i--)mode_bits.push_back((mode_code_3>>i)&1);modecodesbits[3]=mode_bits;
    mode_bits.clear();for(int i=15;i>=0;i--)mode_bits.push_back((mode_code_4>>i)&1);modecodesbits[4]=mode_bits;


    mode=none;

    setMode(mode2);

    assert(mode!=none);
}

DataFormatter::~DataFormatter()
{
    delete convolcodec;
}

bool DataFormatter::setPreamble(quint64 bitpreamble,int len)
{
    if(len<1||len>64)return false;
    preamble.clear();
    for(int i=len-1;i>=0;i--)
    {
        if((bitpreamble>>i)&1)preamble.push_back(1);
         else preamble.push_back(0);
    }
    return true;
}

void DataFormatter::setMode(DataFormatter::Mode _mode)
{
    NumberOfFramesToReserve=4;

    mode=_mode;
    switch(mode)
    {
    case DataFormatter::mode0:
        //no FEC, short frame
        NumberOfBitsInBlock=9600;
        NumberOfBitsInHeader=16;// for frame info
        NumberOfBytesInUnencodedMessage=NumberOfBitsInBlock/8;//%100
        setPreamble(18159415077994808079ULL,64); //1111110000000011001100111100110011111100110000001100001100001111
        leaver.setSize(NumberOfBitsInBlock/64);
        block.resize(NumberOfBitsInBlock);
        NumberOfBitsInFrame=NumberOfBitsInHeader+NumberOfBitsInBlock+64;
        setMaxbufferSize(NumberOfBitsInFrame*NumberOfFramesToReserve/8);//in bytes
        modecode=mode_code_0;
        break;
    case DataFormatter::mode1:
        //2/3 FEC, short frame
        NumberOfBitsInBlock=9600;
        NumberOfBitsInHeader=16;// for frame info
        NumberOfBytesInUnencodedMessage=(3*NumberOfBitsInBlock/(4*8));//75%
        setPreamble(18159415077994808079ULL,64); //1111110000000011001100111100110011111100110000001100001100001111
        leaver.setSize(NumberOfBitsInBlock/64);
        block.resize(NumberOfBitsInBlock);
        NumberOfBitsInFrame=NumberOfBitsInHeader+NumberOfBitsInBlock+64;
        setMaxbufferSize(NumberOfBitsInFrame*NumberOfFramesToReserve/8);//in bytes
        modecode=mode_code_1;
        break;
    case DataFormatter::mode2:
        //2/3 FEC, long frame
        NumberOfBitsInBlock=19200;
        NumberOfBitsInHeader=16;// for frame info
        NumberOfBytesInUnencodedMessage=(3*NumberOfBitsInBlock/(4*8));//75%
        setPreamble(18159415077994808079ULL,64); //1111110000000011001100111100110011111100110000001100001100001111
        leaver.setSize(NumberOfBitsInBlock/64);
        block.resize(NumberOfBitsInBlock);
        NumberOfBitsInFrame=NumberOfBitsInHeader+NumberOfBitsInBlock+64;
        setMaxbufferSize(NumberOfBitsInFrame*NumberOfFramesToReserve/8);//in bytes
        modecode=mode_code_2;
        break;
    case DataFormatter::mode3:
        //1/2 FEC, short frame
        NumberOfBitsInBlock=9600;
        NumberOfBitsInHeader=16;// for frame info
        NumberOfBytesInUnencodedMessage=(1*NumberOfBitsInBlock/(2*8));//50%
        setPreamble(18159415077994808079ULL,64); //1111110000000011001100111100110011111100110000001100001100001111
        leaver.setSize(NumberOfBitsInBlock/64);
        block.resize(NumberOfBitsInBlock);
        NumberOfBitsInFrame=NumberOfBitsInHeader+NumberOfBitsInBlock+64;
        setMaxbufferSize(NumberOfBitsInFrame*NumberOfFramesToReserve/8);//in bytes
        modecode=mode_code_3;
        break;
    case DataFormatter::mode4:
        //1/2 FEC, long frame
        NumberOfBitsInBlock=19200;
        NumberOfBitsInHeader=16;// for frame info
        NumberOfBytesInUnencodedMessage=(1*NumberOfBitsInBlock/(2*8));//50%
        setPreamble(18159415077994808079ULL,64); //1111110000000011001100111100110011111100110000001100001100001111
        leaver.setSize(NumberOfBitsInBlock/64);
        block.resize(NumberOfBitsInBlock);
        NumberOfBitsInFrame=NumberOfBitsInHeader+NumberOfBitsInBlock+64;
        setMaxbufferSize(NumberOfBitsInFrame*NumberOfFramesToReserve/8);//in byte
        modecode=mode_code_4;
        break;
    default:
        qDebug()<<"DataFormatter::setMode: invalid mode"<<mode;
        return;
    break;
    }
}

QVector<int> &DataFormatter::createFrame(QByteArray &frame_msg)
{

    //we have a fixed frame size
    assert(frame_msg.size()==NumberOfBytesInUnencodedMessage);

    //add preamble and header
    frame.resize(0);
    frame.append(preamble);//64 bits

    //append mode type ransmission
    switch(mode)
    {
    case DataFormatter::mode0:
        frame.append(modecodesbits[0]);
        break;
    case DataFormatter::mode1:
        frame.append(modecodesbits[1]);
        break;
    case DataFormatter::mode2:
        frame.append(modecodesbits[2]);
        break;
    case DataFormatter::mode3:
        frame.append(modecodesbits[3]);
        break;
    case DataFormatter::mode4:
        frame.append(modecodesbits[4]);
        break;
    default:
    break;
    }

    //unpack the frame_msg bytes into bits
    QVector<int> msg_buffer_bits;
    msg_buffer_bits.resize(8*frame_msg.size());
    int totalbitptr=0;
    for(int charptr=0;charptr<frame_msg.size();charptr++)
    {
        uchar ch=frame_msg[charptr];
        for(int bitptr=0;bitptr<8;bitptr++)
        {
            msg_buffer_bits[totalbitptr]=(ch>>bitptr)&1;
            totalbitptr++;
        }
    }

    //scramble msg
    scrambler.reset();
    scrambler.update(msg_buffer_bits);

    //convol encode
    switch(mode)
    {
    case DataFormatter::mode0:
        block=msg_buffer_bits;
        break;
    case DataFormatter::mode1:
        block=convolcodec->Encode_Continuous(msg_buffer_bits);
        pc.punture_block(block,3);
        break;
    case DataFormatter::mode2:
        block=convolcodec->Encode_Continuous(msg_buffer_bits);
        pc.punture_block(block,3);
        break;
    case DataFormatter::mode3:
        block=convolcodec->Encode_Continuous(msg_buffer_bits);
        break;
    case DataFormatter::mode4:
        block=convolcodec->Encode_Continuous(msg_buffer_bits);
        break;
    default:
    break;
    }

    //interleave block and add
    frame.append(leaver.interleave(block));


//    //diff enc test
//    static int lastbit_odd=0;
//    for(int i=0;i<frame.size();i+=2)
//    {
//        int bit=frame[i]^lastbit_odd;
//        lastbit_odd=bit;
//        frame[i]=bit;
//    }
//    static int lastbit_even=0;
//    for(int i=1;i<frame.size();i+=2)
//    {
//        int bit=frame[i]^lastbit_even;
//        lastbit_even=bit;
//        frame[i]=bit;
//    }

    //return the the encoded frame
    return frame;
}

//pushpacket and get frame need to be called from the same thread
bool DataFormatter::pushPacket(uchar packet_type,const QByteArray &packet)
{
    if((packet.size()+tmpbuffer.size())>max_tmpbuffer_size)
    {
        qDebug()<<"DataFormatter::pushPacket : no room!";
        return false;
    }
    tmpbuffer+=slip.EscapePacket(packet_type,packet);
    //qDebug()<<((float)tmpbuffer.size())/((float)max_tmpbuffer_size);
    return true;
}
QVector<int> &DataFormatter::getFrame()
{
    //use data in tmpbuffer to create a frame

    //if not enough data is available for the frame then pad whatever data we do have with zeros to create a frame
    if(tmpbuffer.size()<NumberOfBytesInUnencodedMessage)
    {
        tmpbuffer+=QByteArray(NumberOfBytesInUnencodedMessage-tmpbuffer.size(),(uchar)TSlip::END);//send END for ideling char so as to keep slip happy
    }

    QByteArray frame_msg=tmpbuffer.left(NumberOfBytesInUnencodedMessage);
    createFrame(frame_msg);
    tmpbuffer.remove(0,NumberOfBytesInUnencodedMessage);
    return frame;
}

