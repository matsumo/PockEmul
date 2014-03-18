//#include <stdlib.h>
//#include <string.h>

#include <QDebug>

#include "common.h"
#include "lh5810.h"
#include "Inter.h"



//-----------------------------------------------------
// TO DO
//-----------------------------------------------------
// ADD THE SERIAL CAPABILITIES FOR TAPE SAVE AND LOAD
//-----------------------------------------------------

CLH5810::CLH5810(CPObject *parent)	: CPObject(parent)				//[constructor]
{
	lh5810.r_g=lh5810.r_msk=lh5810.r_dda=lh5810.r_ddb=lh5810.r_opa=lh5810.r_opb=lh5810.r_opc=lh5810.r_f=0;
	lh5810.r_if=0;
    IRQ=INT=false;
    SDO=SDI=CLI=false;
    modulationSend = false;
    New_L=false;
    New_G=New_F=true;
    RolReg = 0xffff;
    clockOutput = false;
//	OPA=OPB=0;

}

CLH5810::~CLH5810(){			//[destructor]

}


bool	CLH5810::init(void){    //initialize
    Reset();
    return true;
}

bool	CLH5810::exit(void){	//end
    return true;
}

void	CLH5810::Load_Internal(QFile *file){
    char t[16];
    QDataStream in(file);

    in.readRawData(t, 9);
    in.readRawData((char*)&lh5810,sizeof(lh5810));
}

void	CLH5810::save_internal(QFile *file){
    QDataStream out(file);

    out.writeRawData("LH5810STA", 9);					//header
    out.writeRawData((char*)&lh5810,sizeof(lh5810));		//reg
}

void CLH5810::Load_Internal(QXmlStreamReader *xmlIn)
{
    if (xmlIn->readNextStartElement()) {
        if ( (xmlIn->name()=="cpu") &&
             (xmlIn->attributes().value("model").toString() == "lh5810")) {
            QByteArray ba_reg = QByteArray::fromBase64(xmlIn->attributes().value("registers").toString().toLatin1());
            memcpy((char *) &lh5810,ba_reg.data(),sizeof(lh5810));
        }
        xmlIn->skipCurrentElement();
    }
}

void CLH5810::save_internal(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("cpu");
        xmlOut->writeAttribute("model","lh5810");
        QByteArray ba_reg((char*)&lh5810,sizeof(lh5810));
        xmlOut->writeAttribute("registers",ba_reg.toBase64());
    xmlOut->writeEndElement();
}

void	CLH5810::Reset(void)
{
    lh5810.r_g=0;
    lh5810.r_msk=lh5810.r_if=lh5810.r_opa=lh5810.r_opb=lh5810.r_opc=0;
    CLO=false;
    lh5810.r_f=0X00;
    lh5810.r_dda=lh5810.r_ddb=0x00;
    SDO=SDI=CLI=false;
    New_L=false;
    New_G=New_F=true;
    modulationSend = false;
    clockOutput = false;
    RolReg = 0xffff;
}

void CLH5810::start_serial_transmit() {
    RolReg = (0xff00 | lh5810.r_l)<<1 ;
//    bit = 0;
//    serialSend= true;
    bitCount = 0;
    lastPulseState = clockRateState = pPC->pTIMER->state;
//    qWarning()<<"New_L:"<<QString("%1").arg(lh5810.r_l,2,16,QChar('0'));

}

void CLH5810::ResetDivider()
{
   clockRateState=pPC->pTIMER->state;
}

bool CLH5810::step()
{
	INT = false;

    if (New_F) {
        FX = lh5810.r_f & 0x07;
        FY = (lh5810.r_f>>3) & 0x07;
        qWarning()<<"Fx="<<FX<<"   FY="<<FY<<"  val="<<lh5810.r_f;
        New_F = false;

        modulationSend = (lh5810.r_f & 0x40);
        bit = false;

    }

    if (modulationSend) {
//        SetRegBit(IF,3,false);
        bit = RolReg & 0x01;

        quint64 waitState =  ( bit ? (0x40 << FX) : (0x40 << FY))/2;
        if ( (pPC->pTIMER->state - lastPulseState) >= waitState) {
            SDO = !SDO;
//            qWarning()<<"flip="<<(pPC->pTIMER->state - lastPulseState);
            while ((pPC->pTIMER->state - lastPulseState) >= waitState)
                lastPulseState += waitState;
        }
    }

    if (clockOutput) {
        if ( (pPC->pTIMER->state - clockRateState) >= (clockRate/1)) {
            CLO = true;
            bitCount++;
            if (bitCount==9) SetRegBit(IF,3,true);

            RolReg >>= 1;
            RolReg |=0x8000;

            //            qWarning()<<"bit:"<<(RolReg & 0x01)<<"  delta="<<pPC->pTIMER->state - clockRateState;
            while((pPC->pTIMER->state - clockRateState) >= (clockRate/1))
                clockRateState += clockRate/1;
        }
        if (CLO &&
            ( (pPC->pTIMER->state - clockRateState)>(clockRate/10) ) )
        {
            CLO = false;
        }

    }



    if (New_G) {
        switch (lh5810.r_g & 0x07) {
        case 0x00 : clockRate = 1; break;
        case 0x01 : clockRate = 2; break;
        case 0x02 : clockRate = 128; break;
        case 0x03 : clockRate = 256; break;
        case 0x04 : clockRate = 512; break;
        case 0x05 : clockRate = 1024; break;
        case 0x06 : clockRate = 2048; break;
        case 0x07 : clockRate = 4096; break;

        }
        clockRateWait = pPC->getfrequency() / clockRate;
//        qWarning()<<"G= "<<lh5810.r_g<<"   ClockRate set to :"<<clockRateWait;

        clockOutput = lh5810.r_g & 0X10;
        New_G = false;
    }
	// If the L register change, then TD flag of the IF register down
	if (New_L)
	{
        //AddLog(LOG_TAPE,tr("L register change -> %1X").arg(lh5810.r_l,4,16,QChar('0')));
        SetRegBit(IF,3,false);
		New_L=false;
        if (modulationSend)
            qWarning()<<"Serial transmission in progress!!!";
        start_serial_transmit();
	}
	if (IRQ)	lh5810.r_if|=0x01;
    if (LH5810_PB7)	lh5810.r_if|=0x02;

	if (
		( (lh5810.r_msk & 0x01) && IRQ ) ||
        ( (lh5810.r_msk & 0x02) && LH5810_PB7 )
		)
	{
		INT=true;
//        if (pPC->pCPU->fp_log) fprintf(pPC->pCPU->fp_log,"INT\n");
	}
	
	
	return(1);
}


void CLH5810::Regs_Info(UINT8 Type)
{

	switch(Type)
	{
	case 0:			// Monitor Registers Dialog
		sprintf(Regs_String,	"\r\nLH5810 :\r\nG:%.2x  MSK:%.2x  IF:%.2x\r\nDDA:%.2x  DDB:%.2x\r\nOPA:%.2x  OPB:%.2x  OPC:%.2x\r\nF:%.2x  IRQ:%c",
							lh5810.r_g,lh5810.r_msk,lh5810.r_if,lh5810.r_dda,lh5810.r_ddb,
							lh5810.r_opa,lh5810.r_opb,lh5810.r_opc,lh5810.r_f,(IRQ?'1':'0')
							);
		break;
	}

}



