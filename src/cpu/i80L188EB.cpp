
#include "i80L188EB.h"
#include "pit8253.h"
#include "pcxxxx.h"
#include "Inter.h"
#include "Log.h"
#include "dialoganalog.h"

Ci80L188EB::Ci80L188EB(CPObject *parent):Ci80x86(parent)
{
    p8253 = new C8253PIT(this);
    pserial0 = new Ci80uart(this);
    pserial1 = new Ci80uart(this);

}

Ci80L188EB::~Ci80L188EB()
{
    delete p8253;
}

bool Ci80L188EB::init()
{
    Ci80x86::init();

    return true;
}

bool Ci80L188EB::exit()
{
    Ci80x86::exit();

    return true;
}

void Ci80L188EB::step()
{
    quint64 _states = pPC->pTIMER->state;
    Ci80x86::step();
    p8253->step(pPC->pTIMER->state - _states);

    pserial0->step(pPC->pTIMER->state - _states);
//    pserial1->step(pPC->pTIMER->state - _states);


}


void Ci80L188EB::Reset()
{
    Ci80x86::Reset();
    pserial0->Reset();
    pserial1->Reset();

}




Ci80uart::Ci80uart(Ci80L188EB *parent)
{
    i80l188ebcpu = parent;
}

void Ci80uart::Reset()
{
    SxRBUF = SxTBUF = BxCNT = BxCMP = SxCON = 0;
    SxSTS = 0x08;
    TXD = true;
    risingTXD = false;
    SxTBUFPending = TShiftRegPending = false;
    receivedBit = transmitedBit = samplingCNT = 0;
}

#define MODE (SxCON & 0x07)

void Ci80uart::step(quint64 states)
{
    if (BxCMP == 0) return;

    for (int i = 0 ; i< states ; i++) {
        BxCNT++;
//        if (MODE == 0) {
//            // Synchronous mode
//            // Clock on TXD
//            // RXD is used for transmit and received bits
//            if (BxCNT<2) {
//                TXD = false;
//            }
//            else {
//                TXD = true;
//                risingTXD = true;
//            }
//        }
        if (BxCNT >= (((BxCMP&0x7FFF) +1)*8/*/(MODE==0?1:8)*/)) {

//            if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(99);
            BxCNT = 0;
#if 0
            if (REN()) {
                // RECEIVE MODE
                // the RI bit must be zero tto start reception
                if (RI()) {
                    if (risingTXD) {
                        // Capture RXD
                        RShiftReg <<= 1;
                        RShiftReg &= 0xfffe;
                        RShiftReg |= (RXD?1:0);
                        receivedBit++;
                        if (receivedBit==8) {
                            receivedBit = 0;
                            SxRBUF = RShiftReg & 0xff;
                            SxSTS |= MASK_RI;   // set RI
                            // send an interrupt?
                        }
                    }
                }
            }
#endif
            if (TShiftRegPending) {
                //AddLog(LOG_SIO,QString("cnt:%1").arg(samplingCNT));
//                if (samplingCNT >= 8 ) {
//                    samplingCNT = 0;

//                }

//                if (samplingCNT>0) {
//                    samplingCNT++;
//                }
//                else
                {
                    samplingCNT++;
                    // Send bit on RXD

                    if ((transmitedBit==0) && (MODE&1)) {
                        // send byte start
                        TXD = false;
                        transmitedBit++;
                        TParity = false;
                    }
                    else if ((transmitedBit<=8) && (MODE&1)) {
                        //            TShiftReg &= 0xfffe;
                        TXD = (TShiftReg & 0x01 ? true: false);
                        TParity ^= TXD;
                        AddLog(LOG_SIO,QString("Send bit:")+(TXD?"1":"0"));
                        TShiftReg >>=1;
                        transmitedBit++;
                    } else if ((transmitedBit==9) && (MODE&2)) {
                        // 9th bit
                        if (PEN()) {
                            TXD = (EVN()?TParity:!TParity);
                        }
                        else {
                            TXD = TB8();
                        }
                        transmitedBit++;
                    }
                    else if ((transmitedBit>=9) && (MODE&1)) {
                        // stop bit
                        TXD = true;
                        SxSTS |= MASK_TI;
                        transmitedBit = 0;
                        TShiftRegPending = false;
                        if (SxTBUFPending) {
                            TShiftReg = SxTBUF;
                            SxTBUFPending = false;
                            TShiftRegPending = true;
                        }
                        else {
                            SxSTS |= MASK_TXE;
                        }
                    }
                }
            }
        }
    }
}

void Ci80uart::write_SxCON(quint16 value)
{

    mode = value & 0x07;
}

void Ci80uart::set_SxTBUFL(quint8 data)
{
    SxTBUF = data;
    if (SxSTS & MASK_TXE) {
        // TShiftReg free, copy data to it
        TShiftReg = data;
        TShiftRegPending = true;
//        SxSTS |= MASK_TI;
        SxSTS &= ~MASK_TXE;
        SxTBUFPending = false;
    }
    else {
        SxTBUFPending = true;
        SxSTS &= ~MASK_TI;
        SxSTS &= ~MASK_TXE;
    }
}
void Ci80uart::set_SxTBUFH(quint8 data)
{
    SxTBUF = data;
    SxTBUFPending = true;
    SxSTS &= ~MASK_TI;
    SxSTS &= ~MASK_TXE;
}
