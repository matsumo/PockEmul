// *********************************************************************
//    Copyright (c) 1989-2002  Warren Furlow
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// *********************************************************************

// *********************************************************************
// HP41.cpp : implementation file
// *********************************************************************

#include <math.h>
#include <QDebug>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QPainter>

#include "hp15c.h"
#include "hpnut.h"


#include "Lcdc_hp15c.h"
#include "Keyb.h"
#include "Inter.h"
#include "watchpoint.h"
#include "Connect.h"
#include "clink.h"


#include "mainwindowpockemul.h"
extern MainWindowPockemul *mainwindow;



/****************************/
// Constructor
/****************************/


Chp15c::Chp15c(CPObject *parent,Models mod):CpcXXXX(parent)
{

    setfrequency( (int) 215000);  // 80µs per cycle
    memsize		= 0x10000;
    InitMemValue	= 0x00;
    SlotList.clear();

    switch (mod) {
    case HP11:
        setcfgfname(QString("hp11c"));
        SessionHeader	= "HP11CPKM";
        Initial_Session_Fname ="hp11c.pkm";
        BackGroundFname	= P_RES(":/hp11c/hp11c.png");
        //    TopFname = P_RES(":/hp15c/top.png");
        BackFname = P_RES(":/hp11c/hp11cback.png");
        LeftFname = P_RES(":/hp15c/hp15cleft.png");
        RightFname = P_RES(":/hp15c/hp15cright.png");
        SlotList.append(CSlot(12 , 0x0000 ,P_RES(":/hp11c/hp11c.bin"), ""	, CSlot::ROM , "ROM"));
        break;

    case HP12:
        setcfgfname(QString("hp12c"));
        SessionHeader	= "HP12CPKM";
        Initial_Session_Fname ="hp12c.pkm";
        BackGroundFname	= P_RES(":/hp12c/hp12c.png");
        //    TopFname = P_RES(":/hp15c/top.png");
        BackFname = P_RES(":/hp12c/hp12cback.png");
        LeftFname = P_RES(":/hp15c/hp15cleft.png");
        RightFname = P_RES(":/hp15c/hp15cright.png");
        SlotList.append(CSlot(12 , 0x0000 ,P_RES(":/hp12c/hp12c.bin"), ""	, CSlot::ROM , "ROM"));
        break;

    case HP16:
        setcfgfname(QString("hp16c"));
        SessionHeader	= "HP16CPKM";
        Initial_Session_Fname ="hp16c.pkm";
        BackGroundFname	= P_RES(":/hp16c/hp16c.png");
        //    TopFname = P_RES(":/hp15c/top.png");
        BackFname = P_RES(":/hp16c/hp16cback.png");
        LeftFname = P_RES(":/hp15c/hp15cleft.png");
        RightFname = P_RES(":/hp15c/hp15cright.png");
        SlotList.append(CSlot(12 , 0x0000 ,P_RES(":/hp16c/hp16c.bin"), ""	, CSlot::ROM , "ROM"));
        break;

    default:
        setcfgfname(QString("hp15c"));
        SessionHeader	= "HP15CPKM";
        Initial_Session_Fname ="hp15c.pkm";
        BackGroundFname	= P_RES(":/hp15c/hp15c.png");
        //    TopFname = P_RES(":/hp15c/top.png");
        BackFname = P_RES(":/hp15c/hp15cback.png");
        LeftFname = P_RES(":/hp15c/hp15cleft.png");
        RightFname = P_RES(":/hp15c/hp15cright.png");
        SlotList.append(CSlot(28 , 0x0000 ,P_RES(":/hp15c/hp15c.bin"), ""	, CSlot::ROM , "ROM"));
        break;
    }

    setDXmm(130);
    setDYmm(79);
    setDZmm(15);

    setDX(465);
    setDY(283);

    pTIMER		= new Ctimer(this);
    pLCDC		= new Clcdc_hp15c(this,QRect(90,35,210,33),
                                  QRect());
    pCPU		= new CHPNUT(this, mod==HP15 ? 80 : 40);
    nutcpu = (CHPNUT*)pCPU;
    pKEYB		= new Ckeyb(this,"hp15c.map");

    turnOnNext = false;
}


/****************************/
// destructor
/****************************/
Chp15c::~Chp15c()
  {

  }


bool Chp15c::init()
{
//    pCPU->logsw = true;



    CpcXXXX::init();

    ((Clcdc_hp15c*)pLCDC)->voyager_display_init_ops(nutcpu->reg,((Clcdc_hp15c*)pLCDC)->info);

    WatchPoint.remove(this);


    pTIMER->resetTimer(0);

   return true;
}

extern int ask(QWidget *parent,QString msg,int nbButton);
#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))

void Chp15c::TurnON()
{
    CpcXXXX::TurnON();
    pCPU->Reset();
    ((Clcdc_hp15c*)pLCDC)->voyager_op_display_toggle (((CHPNUT*)pCPU)->reg, 0);
}

void Chp15c::TurnOFF()
{
    ((Clcdc_hp15c*)pLCDC)->voyager_op_display_toggle (((CHPNUT*)pCPU)->reg, 0);
    ASKYN _save = mainwindow->saveAll;
    mainwindow->saveAll = YES;
    CpcXXXX::TurnOFF();
    mainwindow->saveAll = _save;
}

void Chp15c::BuildContextMenu(QMenu *menu)
{
    menu->addAction(tr("Turn ON on next key pressed"),this,SLOT(TurnNext()));
    CpcXXXX::BuildContextMenu(menu);

}

void Chp15c::ComputeKey(CPObject::KEYEVENT ke, int scancode)
{
//    if (!Power && turnOnNext) {
//        qWarning()<<"Turn on next";
//        pKEYB->keyPressedList.append(K_OF);
//        pKEYB->LastKey = K_OF;
//        TurnON();
//        //turnOnNext = false;
//        // Start a 200ms timer to release the ON Key
//        pTIMER->resetTimer(1);
//    }

    if (ke==KEY_PRESSED) {
        if (pKEYB->keyPressedList.count()==1)
            firstkey = scancode;
            nutcpu->nut_press_key (nutcpu->reg, getKey());
    }
    if (ke==KEY_RELEASED) {
        if (pKEYB->keyPressedList.isEmpty()) {
            nutcpu->nut_release_key(nutcpu->reg);
            firstkey = 0;
        }
        if (pKEYB->keyPressedList.count()==1) {
            nutcpu->nut_release_key(nutcpu->reg);
            firstkey = scancode;
            nutcpu->nut_press_key (nutcpu->reg, getKey());
        }
    }
}

bool Chp15c::run()
{
    static int lcd_count = 15;
    lcd_count--;
    if (lcd_count == 0) {
        lcd_count=15;
        ((Clcdc_hp15c*)pLCDC)->voyager_display_update(nutcpu->reg,((Clcdc_hp15c*)pLCDC)->info);
    }

    pLCDC->updated = true;

    if (pTIMER->usElapsedId(0)>=10000) {
        pTIMER->resetTimer(0);
//        TimerProc();
    }


    CpcXXXX::run();

    pTIMER->state+=56;
    return true;

}



UINT8 Chp15c::in(UINT8 address)
{
    Q_UNUSED(address)

    return 0;
}

UINT8 Chp15c::out(UINT8 address, UINT8 value)
{
    Q_UNUSED(address)
    Q_UNUSED(value)

    return 0;
}




/****************************/
// return true if RAM exists at the specified RAM address
/****************************/
bool Chp15c::Chk_Adr(UINT32 *d, UINT32 data)
{
    Q_UNUSED(d)
    Q_UNUSED(data)

    return false;

}

bool Chp15c::Chk_Adr_R(UINT32 *d, UINT32 *data)
{
    Q_UNUSED(d)
    Q_UNUSED(data)

    return true;
}


#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))

UINT8 Chp15c::getKey()
{
    UINT8 code = 0;

    if ( (currentView==FRONTview) && (pKEYB->LastKey))
    {

        if (KEY('A'))			code = 0x13;    // A
        if (KEY('B'))			code = 0x33;    // B
        if (KEY('C'))			code = 0x73;    // C
        if (KEY('D'))			code = 0xC3;    // D
        if (KEY('E'))			code = 0x83;    // E
        if (KEY(K_SIGN))		code = 0x82;    // CHS
        if (KEY('7'))			code = 0xC2;    // 7
        if (KEY('8'))			code = 0x72;    // 8
        if (KEY('9'))			code = 0x32;    // 9
        if (KEY('/'))			code = 0x12;    // /
        if (KEY(':'))			code = 0x12;    // /

        if (KEY(K_SST))			code = 0x10;    // SST
        if (KEY(K_GTO))			code = 0x30;    // GTO
        if (KEY(K_SIN))			code = 0x70;    // SIN
        if (KEY(K_COS))			code = 0xC0;    // COS
        if (KEY(K_TAN))			code = 0x80;    // TAN
        if (KEY(K_EE))			code = 0x87;    // EEX
        if (KEY('4'))			code = 0xC7;    // 4
        if (KEY('5'))			code = 0x77;    // 5
        if (KEY('6'))			code = 0x37;    // 6
        if (KEY('*'))			code = 0x17;    // *

        if (KEY(K_RS))			code = 0x11;    // R/S
        if (KEY(K_GSB))			code = 0x31;    // GSB
        if (KEY(K_DA))			code = 0x71;    // R Down
        if (KEY(K_XY))			code = 0xC1;    // XY
        if (KEY(K_LA))			code = 0x81;    // left arrow
        if (KEY(K_RET))			code = 0x84;    // ENTER
        if (KEY('1'))			code = 0xC4;    // 1
        if (KEY('2'))			code = 0x74;    // 2
        if (KEY('3'))			code = 0x34;    // 3
        if (KEY('-'))			code = 0x14;    // -

        if (KEY(K_OF))			code = 0x18;    // ON
        if (KEY('F'))			code = 0x38;    // f
        if (KEY('G'))			code = 0x78;    // g
        if (KEY(K_STO))			code = 0xC8;    // STO
        if (KEY(K_RCL))			code = 0x88;    // RCL
//        if (KEY(K_RET))			code = 0x85;    // ENTER
        if (KEY('0'))			code = 0xC5;    // 0
        if (KEY('.'))			code = 0x75;    // .
        if (KEY(';'))			code = 0x75;    // .
        if (KEY(K_SUM))			code = 0x35;    // sigma
        if (KEY('+'))			code = 0x15;    // +

    }

    return code;

}

bool Chp15c::LoadConfig(QXmlStreamReader *xmlIn)
{
    Q_UNUSED(xmlIn)

//    if (xmlIn->readNextStartElement()) {
//        if (xmlIn->name()=="lcd") {
//            QByteArray ba_reg = QByteArray::fromBase64(xmlIn->attributes().value("registers").toString().toLatin1());
//            memcpy((char *) ((Clcdc_hp15c*)pLCDC)->info,ba_reg.data(),sizeof(voyager_display_reg_t));
//        }
//        xmlIn->skipCurrentElement();
//    }

    return true;
}

bool Chp15c::SaveConfig(QXmlStreamWriter *xmlOut)
{
    Q_UNUSED(xmlOut)

//    xmlOut->writeStartElement("lcd");
//    QByteArray ba_reg((char*)((Clcdc_hp15c*)pLCDC)->info,sizeof(voyager_display_reg_t));
//    xmlOut->writeAttribute("registers",ba_reg.toBase64());
//    xmlOut->writeEndElement();

    return true;
}

void Chp15c::TurnNext()
{

    turnOnNext = true;
}
