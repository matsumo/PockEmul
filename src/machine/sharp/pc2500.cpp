//BUG:  the RUN statement is not working. DEF"a" or RUN 10 are working
//BUG:   c'est un probleme de bankswitch
//FIXME: DEF key is not working
#include <QtGui>
#include "pc2500.h"
#include "sc61860.h"
#include "Inter.h"
#include "cextension.h"
#include "Lcdc.h"
#include "Keyb.h"
#include "Connect.h"
#include "Keyb2500.h"
#include "paperwidget.h"
#include "ce515p.h"

#define KEY(c)	( TOUPPER(pKEYB->LastKey) == TOUPPER(c) )

Cpc2500::Cpc2500(CPObject *parent)	: Cpc1350(this)
{								//[constructor]
    Q_UNUSED(parent)

    setcfgfname("pc2500");

    SessionHeader	= "PC2500PKM";
    Initial_Session_Fname ="pc2500.pkm";

    BackGroundFname	= P_RES(":/pc2500/pc2500.png");
    LcdFname		= P_RES(":/pc2500/2500lcd.png");
    SymbFname		= P_RES(":/pc2500/2500symb.png");
    memsize			= 0x18000;
//		NbSlot		= 3;

    SlotList.clear();
    SlotList.append(CSlot(8 , 0x0000 ,	P_RES(":/pc2500/cpu-2500.bin")	, "pc-2500/cpu-2500.bin"	, CSlot::ROM , "CPU ROM"));
    SlotList.append(CSlot(24, 0x2000 ,	""								, "pc-2500/R1-2500.ram"		, CSlot::RAM , "RAM"));
    SlotList.append(CSlot(32, 0x8000 ,	P_RES(":/pc2500/r1-2500.bin")	, "pc-2500/r1-2500.bin"	, CSlot::ROM , "BAS ROM"));
    SlotList.append(CSlot(32, 0x10000 ,	P_RES(":/pc2500/r2-2500.bin")	, "pc-2500/r2-2500.bin"	, CSlot::ROM , "BUSINESS ROM"));

    KeyMap		= KeyMap2500;
    KeyMapLenght= KeyMap2500Lenght;

    pLCDC		= new Clcdc_pc2500(this);
    pKEYB		= new Ckeyb(this,"pc2500.map",scandef_pc2500);
    pCPU		= new CSC61860(this);


    pTIMER		= new Ctimer(this);
    pce515p     = new Cce515p(this);
    pce515p->pTIMER = pTIMER;

    setDXmm(297);//Pc_DX_mm = 297;
    setDYmm(210);//Pc_DY_mm = 210;
    setDZmm(46);//Pc_DZ_mm = 46;

    setDX(960);//Pc_DX = 960;
    setDY(673);//Pc_DY = 673;

    Lcd_X		= 560;
    Lcd_Y		= 70;
    Lcd_DX		= 150;
    Lcd_DY		= 32;
    Lcd_ratio_X	= 2;
    Lcd_ratio_Y	= 2;

    Lcd_Symb_X	= 560;
    Lcd_Symb_Y	= 60;
    Lcd_Symb_DX	= 300;
    Lcd_Symb_DY	= 5;
    //Lcd_Symb_ratio = 2;

    printMode = false;
    capslock = false;
    pce515p->margin = 10;
}

Cpc2500::~Cpc2500()
{
    delete pTAPECONNECTOR;
    delete pce515p;
}

bool Cpc2500::UpdateFinalImage(void) {
    CpcXXXX::UpdateFinalImage();

    // Draw switch by 180° rotation
    QPainter painter;

    // PRINTER SWITCH
    painter.begin(FinalImage);
    painter.drawImage(580,239,BackgroundImageBackup->copy(580,239,59,15).mirrored(!printMode,false));


    // CAPS LOCK
    if (capslock) {
//        painter.setPen( Qt::green);
        painter.fillRect(139,645,5,5,QColor(Qt::green));
    }
    painter.end();

    return true;
}

void Cpc2500::resizeEvent ( QResizeEvent * ) {
    float ratio = (float)this->width()/this->getDX() ;

    QRect rect = pce515p->paperWidget->baseRect;
    pce515p->paperWidget->setGeometry(rect.x()*ratio,
                                      rect.y()*ratio,
                                      rect.width()*ratio,
                                      rect.height()*ratio);
}

BYTE	Cpc2500::Get_PortA(void)
{
//    BYTE data = Cpc13XX::Get_PortA();
    BYTE data = pKEYB->Read(IO_A);
    IO_A = data;
    BYTE ks = pKEYB->Get_KS();
    if ((ks & 0x10) && printMode)       data |= 0x80;
    if ((ks & 0x20) && pKEYB->isShift)  data |= 0x80;
    if (ks & 0x40)                      data |= 0x80;         // JAPAN ?
//  if (ks & 0x80)                      data |= 0x80;         // Power OFF

    return (data);
}

void	Cpc2500::Set_PortF(BYTE data)
{
    IO_F = data;
    ProtectMemory = GET_PORT_BIT(PORT_F,1);
}

BYTE Cpc2500::Get_PC(UINT32 adr)
{
    //Chk_Adr_R(&adr,bREAD);
    if ( (adr >= 0x8000) && (adr<=0xFFFF) && (RomBank & 0x02) ) {
        adr += 0x8000;
    }
    return(mem[adr]);
}

WORD Cpc2500::Get_16rPC(UINT32 adr)
{
    UINT32	a;

    if ( (adr >= 0x8000) && (adr<=0xFFFF) && (RomBank & 0x02) ) {
        adr += 0x8000;
    }
    a=adr+1;
    //Chk_Adr_R(&adr,bREAD);
    //Chk_Adr_R(&a,bREAD);
    //if (pCPU->fp_log) fprintf(pCPU->fp_log,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    return((mem[adr]<<8)+mem[a]);
}

bool Cpc2500::Chk_Adr(UINT32 *d,UINT32 data)
{
    if (ProtectMemory) {
        if (pCPU->fp_log) fprintf(pCPU->fp_log,"ECRITURE EXT [%04x]=%02x (%c)\n",(uint)*d,(BYTE)data,(int)data);
        if ( (*d>=0x8000) && (*d<=0xFFFF) ){

            *d += 0x8000 ;
            return(1);
        }
        return (1);
    }

    if ( (*d>=0x7000) && (*d<=0x7BFF) ) {
        if (pCPU->fp_log) fprintf(pCPU->fp_log,"ECRITURE [%04x]=%02x (%c)\n",(uint)*d,(BYTE)data,(int)data);
    }
    if ( (*d>=0x7100) && (*d<=0x71FF) )	{
        RomBank = data;
        return(1);
    }
    if ( (*d>=0x7700) && (*d<=0x77FF) )	{
        capslock = data;
        return(1);
    }
    if ( (*d>=0x7000) && (*d<=0x79FF) )	{pLCDC->SetDirtyBuf(*d-0x7000);return(1);}
    if ( (*d>=0x7A00) && (*d<=0x7AFF) )
    {
        pKEYB->Set_KS( (pKEYB->Get_KS() & 0xF0) | ((BYTE) data & 0x0F ));
        return(1);
    }
    if ( (*d>=0x7B00) && (*d<=0x7BFF) )
    {
        pKEYB->Set_KS( (pKEYB->Get_KS() & 0x0F) | (((BYTE) data & 0x0F )<<4));
        return(1);
    }
    if (pCPU->fp_log) fprintf(pCPU->fp_log,"ECRITURE [%04x]=%02x (%c)\n",(uint)*d,(BYTE)data,(int)data);

    if ( (*d>=0x2000) && (*d<=0x3FFF) && EXTENSION_CE201M_CHECK ) { *d+=0x2000;}										// 8Kb Ram Card Image
    if ( (*d>=0x2000) && (*d<=0x3FFF) )	{ return( EXTENSION_CE202M_CHECK | EXTENSION_CE203M_CHECK); }							// 16K
    if ( (*d>=0x4000) && (*d<=0x5FFF) )	{ return( EXTENSION_CE201M_CHECK | EXTENSION_CE202M_CHECK | EXTENSION_CE203M_CHECK); }	// 32K
    if ( (*d>=0x6000) && (*d<=0x7FFF) ) return(1);																				// Internal RAM area(6000-8000)

    return(0);

}

bool Cpc2500::init(void) {
    Cpc1350::init();
    pce515p->init();
    pTAPECONNECTOR	= new Cconnector(this,2,2,Cconnector::Jack,"Line in / Rec",false);	publish(pTAPECONNECTOR);
    pSIOCONNECTOR->setSnap(QPoint(960,480));

    remove(pCONNECTOR); // delete pCONNECTOR;

    WatchPoint.remove(&pCONNECTOR_value);    // Remove the pc130 11pins connector from the analogic monitor
    WatchPoint.add(&pTAPECONNECTOR_value,64,2,this,"Line In / Rec");
    return true;
}

bool Cpc2500::Chk_Adr_R(UINT32 *d,UINT32 *data)
{
    Q_UNUSED(data)

    if (                   (*d<=0x1FFF) ) { return(0);}
    if ( (*d >= 0x2000) && (*d<=0x3FFF) && EXTENSION_CE201M_CHECK ) { *d+=0x2000; return true;}  // 8Kb Ram Card Image
    if ( (*d >= 0x8000) && (*d<=0xFFFF) && (RomBank & 0x02) ) { *d += 0x8000; return true; }

    if (ProtectMemory) {
        if ( (*d>=0x8000) && (*d<=0xFFFF) )	{
            *d += 0x8000 ;
            if (pCPU->fp_log) {
                fprintf(pCPU->fp_log,"READ EXT ROM [%04x] : %c (%02x)\n",(uint)(*d-0x8000),mem[*d],mem[*d]);
            }
            return (1);
        }
        return (1);
    }

    if (pCPU->fp_log) fprintf(pCPU->fp_log,"LECTURE [%04x]=%02x (%c)\n",(uint)*d,mem[*d],mem[*d]);

    return true;
}

bool Cpc2500::run(void)
{
    if (pKEYB->LastKey == K_PRINT_ON) {
        printMode = true;
    }
    if (pKEYB->LastKey == K_PRINT_OFF) {
        printMode = false;
    }
    if (pKEYB->LastKey == K_PFEED) {
        pce515p->PaperFeed();
        //pKEYB->LastKey = 0;
    }
    Set_Port_Bit(PORT_B,8,1);
    pTAPECONNECTOR_value = pTAPECONNECTOR->Get_values();

    Cpc1350::run();

    //pce515p->pCONNECTOR->Set_values(pCONNECTOR->Get_values());
    pce515p->set_SD(Get_Port_bit(PORT_F,3));
    pce515p->run();

    return true;
}

#define SIO_GNDP	1
#define SIO_SD 		2
#define SIO_RD		3
#define SIO_RS		4
#define SIO_CS		5

#define SIO_GND		7
#define SIO_CD		8
#define SIO_VC1		10
#define SIO_RR		11
#define SIO_PAK		12
#define SIO_VC2		13
#define SIO_ER		14
#define SIO_PRQ		15

bool Cpc2500::Set_Connector(void)
{
    pTAPECONNECTOR->Set_pin(2,pCPU->Get_Xout());

    // MANAGE SERIAL CONNECTOR
    // TO DO
    pSIOCONNECTOR->Set_pin(SIO_SD	,GET_PORT_BIT(PORT_F,4));	//	pSIO->Set_SD((IO_F & 0x08) ? 1 : 0);
    pSIOCONNECTOR->Set_pin(SIO_RR	,GET_PORT_BIT(PORT_B,3));	//( READ_BIT(IO_B,2) );
    pSIOCONNECTOR->Set_pin(SIO_RS	,GET_PORT_BIT(PORT_B,2));	//( READ_BIT(IO_B,1) );
    pSIOCONNECTOR->Set_pin(SIO_ER	,GET_PORT_BIT(PORT_B,1));	//( READ_BIT(IO_B,0) );

    return(1);
}

bool Cpc2500::Get_Connector(void)
{
    // MANAGE STANDARD CONNECTOR
    Set_Port_Bit(PORT_B,8,0);	// DIN	:	IB8
    Set_Port_Bit(PORT_B,7,1);	// ACK	:	IB7
    pCPU->Set_Xin(pTAPECONNECTOR->Get_pin(1));

    // MANAGE SERIAL CONNECTOR
    // TO DO
    Set_Port_Bit(PORT_B,4,pSIOCONNECTOR->Get_pin(SIO_RD));
    Set_Port_Bit(PORT_B,5,pSIOCONNECTOR->Get_pin(SIO_CS));
    Set_Port_Bit(PORT_B,6,pSIOCONNECTOR->Get_pin(SIO_CD));
    // PAK
    return(1);
}
