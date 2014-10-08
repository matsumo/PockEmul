#include <QDebug>
#include <QMutex>

#include "pc1211.h"
#include "cpu.h"
#include "Keyb.h"
#include "Lcdc_pc1211.h"
#include "tinybasic/tinybasic.h"
#include "Inter.h"
#include "Connect.h"

Cpc1211::Cpc1211(CPObject *parent)	: CpcXXXX(parent)
{
    setfrequency( (int) 576000/3);
    setcfgfname(QString("pc1211"));

    SessionHeader	= "PC1211PKM";
    Initial_Session_Fname ="pc1211.pkm";

    BackGroundFname	= P_RES(":/pc1211/pc-1211.png");
    LcdFname		= P_RES(":/pc1211/pc-1211lcd.png");
    SymbFname		= P_RES(":/pc1211/pc-1211symb.png");

    memsize		= 0x10000;

    KeyMap		= KeyMap1250;
    KeyMapLenght= KeyMap1250Lenght;

    PowerSwitch	= 0;

    setDXmm(175);
    setDYmm(70);
    setDZmm(15);

    setDX(626);
    setDY(252);

    Lcd_X		= 46;
    Lcd_Y		= 50;
    Lcd_DX		= 144;//168;//144 ;
    Lcd_DY		= 8;
    Lcd_ratio_X	= 2 * 1.375;
    Lcd_ratio_Y	= 2 * 1.375;

    Lcd_Symb_X	= 55;//(int) (45 * 1.18);
    Lcd_Symb_Y	= 41;//(int) (35 * 1.18);
    Lcd_Symb_DX	= 380;
    Lcd_Symb_DY	= 5;
    Lcd_Symb_ratio_X	= 1;//1.18;

    pLCDC		= new Clcdc_pc1211(this);
    pKEYB		= new Ckeyb(this,"pc1211.map");
    pCPU = new CTinyBasic(this);
    pTIMER		= new Ctimer(this);
    pBASIC = (CTinyBasic *)pCPU;
    DisplayWaitForRTN = false;
    pLCDC1211 = (Clcdc_pc1211*)pLCDC;



    cursorPos =0;
    lastBreakPress=0;
}

Cpc1211::~Cpc1211()
{
}

bool Cpc1211::init()
{
//    pCPU->init();
    CpcXXXX::init();
    CpcXXXX::TurnON();

    pCONNECTOR	= new Cconnector(this,9,0,Cconnector::Sharp_9,"Connector 9 pins",false,QPoint(0,107));		publish(pCONNECTOR);

    shiftFlag = false;


    return true;
}


bool Cpc1211::printerConnected(void) {
//    qWarning()<<"printer connected:"<<pCONNECTOR->Get_pin(1);
    if (pCONNECTOR->Get_pin(1))
        return true;
    return false;
}

bool Cpc1211::run()
{


//    qWarning("RUN");
    CTinyBasic *pBASIC = (CTinyBasic *)pCPU;

    pTIMER->state+=10;




    if (!printerConnected()) {
        pBASIC->printMode = CTinyBasic::DISPLAY;
    }

    if (pKEYB->LastKey==K_BRK) {
        if (pBASIC->CheckRunnig()) {
            pBASIC->breakFlag = true;
        }
        else {
            if (lastBreakPress==0) {
                lastBreakPress=pTIMER->state;
                qWarning()<<"init break timer";
            }
            if ((pTIMER->msElapsed(lastBreakPress)>500) && (pTIMER->msElapsed(lastBreakPress)<2000)) {
                // Check if Printer is connected
                qWarning()<<"Second break";
                if (printerConnected()) {
                   pBASIC->printMode = CTinyBasic::PRINTER;
                    qWarning()<<"PRINT MODE ON";
                }
            }
            else {
                pBASIC->printMode = CTinyBasic::DISPLAY;
            }

        }
        pKEYB->LastKey = 0;
    }
    if (pTIMER->msElapsed(lastBreakPress)>2000) lastBreakPress=0;


    if (pBASIC->pauseFlag) {
//        qWarning()<<pTIMER->msElapsed(pBASIC->waitState);
        if (pTIMER->msElapsed(pBASIC->waitState)<1300)
            return true;
        qWarning()<<"end PAUSE";
        pBASIC->pauseFlag = false;
        pBASIC->waitForRTN = false;
        inputBuffer.clear();
        cursorPos=0;
        pBASIC->inputMode = false;
        pBASIC->outputBuffer.clear();
    }

    Refresh_Display = true;

    switch (pKEYB->LastKey) {
    case K_SHT: shiftFlag = !shiftFlag;
        pKEYB->LastKey = 0;
        break;
    }
    if (pKEYB->isShift || shiftFlag) {
        switch (pKEYB->LastKey) {
        case K_LA: pKEYB->LastKey = K_DEL; break;
        case K_RA: pKEYB->LastKey = K_INS; break;
        case K_UA: pKEYB->LastKey = 0x19; break;
        case K_SQR:pKEYB->LastKey = 0x5E; break;
        case '(': pKEYB->LastKey = '<'; break;
        case ')': pKEYB->LastKey = '>'; break;
        case 'Q': pKEYB->LastKey = '!'; break;
        case 'W': pKEYB->LastKey = '"'; break;
        case 'E': pKEYB->LastKey = '#'; break;
        case 'R': pKEYB->LastKey = '$'; break;
        case 'T': pKEYB->LastKey = '%'; break;
        case 'Y': pKEYB->LastKey = 0x5C; break;
        case 'U': pKEYB->LastKey = '?'; break;
        case 'I': pKEYB->LastKey = ':'; break;
        case 'O': pKEYB->LastKey = ','; break;
        case 'P': pKEYB->LastKey = ';'; break;

        case 'A':
        case 'S':
        case 'D':
        case 'F':
        case 'G':
        case 'H':
        case 'J':
        case 'K':
        case 'L':
        case '=':
        case 'Z':
        case 'X':
        case 'C':
        case 'V':
        case 'B':
        case 'N':
        case 'M':
        case ' ':
            if (pBASIC->CheckMode(CTinyBasic::DEF)) {
                inputBuffer.clear();
                shiftFlag=false;
                pBASIC->commandBuffer.clear();
                pBASIC->commandBuffer.append("RUN\"").append(pKEYB->LastKey).append("\"\n");
                pBASIC->inputMode = false;
                pKEYB->LastKey = 0;
                return true;
            }
            break;

        }
    }

    if (pBASIC->waitForRTN) {
//        pBASIC->inLIST = false;

//        if (pKEYB->LastKey==K_SHT) pKEYB->LastKey=0; return true;
        switch (pKEYB->LastKey) {
        case 0:
        case K_SHT:return true;
        case '+':
        case '-':
        case '/':
        case '*':
        case 0x5E:
            inputBuffer.clear();
            inputBuffer.append(pBASIC->outputBuffer.left(pBASIC->outputBuffer.indexOf('\n')).trimmed());
            cursorPos=inputBuffer.size();
            pBASIC->outputBuffer.clear();
//            pBASIC->backupCommandBuffer.clear();
            pBASIC->inputMode = true;
            break;
        case K_LA:
        case K_RA:
            qWarning()<<"EDITOR"<< inputBuffer<<"inList:"<<pBASIC->inLIST;
            if (pBASIC->inLIST) {
                inputBuffer.clear();
                inputBuffer.append(pBASIC->outputBuffer.left(pBASIC->outputBuffer.indexOf('\n')).trimmed());
                pBASIC->outputBuffer.clear();
                if (inputBuffer.indexOf(':')<4) {
                    cursorPos=inputBuffer.indexOf(':');
                    inputBuffer[inputBuffer.indexOf(':')]=' ';
                }
            }

            pBASIC->inputMode=true;
            pBASIC->outputBuffer.clear();
            break;
        case K_DA:
        case K_UA: if (pBASIC->runMode==CTinyBasic::PRO) pBASIC->inLIST = true;
            else {
                inputBuffer.clear();
                cursorPos=0;
                pBASIC->inputMode = true;
                pBASIC->outputBuffer.clear();
            }
            break;
        default: inputBuffer.clear();
            cursorPos=0;
            pBASIC->inputMode = false;
            pBASIC->outputBuffer.clear();
            break;
        }

        pBASIC->waitForRTN = false;
        Refresh_Display = true;

    }



    if (pBASIC->inputMode) Editor();

    CpcXXXX::run();

    return true;
}

void Cpc1211::Editor() {





    if (pKEYB->LastKey==K_SQR) pKEYB->LastKey=0x18;

    if (pKEYB->LastKey>0) shiftFlag = false;

    switch (pKEYB->LastKey) {
    case 0:
    case K_POW_ON:
        pKEYB->LastKey = 0;
        break;

    case K_MOD: pBASIC->switchMode();
        pKEYB->LastKey=0;
        return;
    case K_CLR: inputBuffer.clear();
        cursorPos = 0;
        pKEYB->LastKey=0;
        break;
    case K_DA:
        if (pBASIC->runMode==CTinyBasic::PRO) {
            inputBuffer.clear();
            cursorPos=0;
            if (!pBASIC->inLIST) pBASIC->linenum=0;
            pBASIC->nextStep = CTinyBasic::LIST_NEXT;
            pBASIC->inputMode = false;
            pBASIC->outputBuffer.clear();
        }
        pKEYB->LastKey=0;
        break;
    case K_UA:
        if (pBASIC->runMode==CTinyBasic::PRO) {
            inputBuffer.clear();
            cursorPos=0;
            if (!pBASIC->inLIST) pBASIC->linenum=999;
            pBASIC->nextStep = CTinyBasic::LIST_PREV;
            pBASIC->inputMode = false;
            pBASIC->outputBuffer.clear();
        }
        pKEYB->LastKey=0;
        break;
    case K_LA:
        cursorPos--;
        if (cursorPos<0) cursorPos=0;
        qWarning()<<"Cursor Pos:"<<cursorPos;
        pKEYB->LastKey = 0;
        break;
    case K_RA: if (cursorPos < inputBuffer.size()) cursorPos++;
        pKEYB->LastKey = 0;
        qWarning()<<"Cursor Pos:"<<cursorPos;
        break;
    case K_BRK: pBASIC->breakFlag = true; break;
    case K_RET: {
        pBASIC->commandBuffer.clear();

        // remove all insert char (0x10)
        int i =0;
        while (i<inputBuffer.size()) {
            if (inputBuffer.at(i)==0x10)
                inputBuffer.remove(i,1);
            else i++;
        }

        pBASIC->commandBuffer.append(inputBuffer).append("\n");
        pBASIC->inputMode = false;
//        inputBuffer.clear();
        pKEYB->LastKey = 0;
    }
        break;
    case K_DEL:
        if (cursorPos<inputBuffer.size()) inputBuffer.remove(cursorPos,1);
        pKEYB->LastKey = 0;
        break;
    case K_INS:
        if (cursorPos<inputBuffer.size()) {
            inputBuffer.insert(cursorPos,0x10);
            pKEYB->LastKey = 0;
            break;
        }
    default:
        if (cursorPos==inputBuffer.size()) {
            inputBuffer.append(pKEYB->LastKey);

        }
        else {
            inputBuffer[cursorPos]=pKEYB->LastKey;
        }

        qWarning()<<"Input:"<<inputBuffer;
        cursorPos++;
        pKEYB->LastKey = 0;
    }
}

void Cpc1211::print(unsigned char c) {
    printerBufferMutex.lock();

    if (c >= 0x80) {
        QByteArray ba = pBASIC->keywordsMap[c]+" ";
        printerBuffer.append(ba);
    }
    else
        printerBuffer.append(c);

    if (c=='\n') {
//        qWarning()<<"PRINTER:"<<printerBuffer;
//        printerBuffer.clear();
    }

    printerBufferMutex.unlock();
}

void Cpc1211::sendToPrinter()
{
    printerBufferMutex.lock();

    if (((pCONNECTOR->Get_values()>>1) & 0xFF)==0) {
        // Send new char
        if (!printerBuffer.isEmpty()) {
            char c = printerBuffer.at(0);
            printerBuffer.remove(0,1);
            pCONNECTOR->Set_values(c<<1);
        }
    }


    printerBufferMutex.unlock();
}


void Cpc1211::afficheChar(quint8 c) {

}

bool Cpc1211::exit()
{
    CpcXXXX::exit();

    return true;
}

bool Cpc1211::Set_Connector()
{
    sendToPrinter();
    return true;
}
