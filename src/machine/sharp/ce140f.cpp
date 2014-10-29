/********************************************************************************************************
 * PROGRAM      : test
 * DATE - TIME  : samedi 28 octobre 2006 - 12h40
 * AUTHOR       :  (  )
 * FILENAME     : Ce126.cpp
 * LICENSE      : GPL
 * COMMENTARY   :
 ********************************************************************************************************/

//TODO: Finish to emulate all non essential functions

#include <QPainter>
#include <QTime>
#include <QFileDialog>
#include <QThread>
#include <QPixmap>
#include <QBitmap>
#include <QDebug>

#include "common.h"

#include "ce140f.h"
#include "Inter.h"
#include "Connect.h"
#include "dialoganalog.h"
#include "Keyb.h"

#define DOWN	0
#define UP		1


Cce140f::Cce140f(CPObject *parent):CPObject(parent)
{								//[constructor]
    setfrequency( 0);
    BackGroundFname	= P_RES(":/ext/ce-140f.png");
    setcfgfname("ce140f");

    pTIMER		= new Ctimer(this);
    pKEYB		= new Ckeyb(this,"ce140f.map");
    setDX(620);
    setDY(488);
    setDXmm(118);
    setDYmm(145);
    setDZmm(39);
    ctrl_char = false;
    t = 0;
    c = 0;
    busyLed = false;
}

Cce140f::~Cce140f() {
    delete pCONNECTOR;
    delete pCONNECTOR_Ext;
}

bool Cce140f::UpdateFinalImage(void) {
    CPObject::UpdateFinalImage();
// Busy led
    if (busyLed) {
        QPainter painter;
        painter.begin(FinalImage);

        painter.fillRect(411,390,20,7,QColor(Qt::green));
        painter.end();
    }

    return true;

}

void Cce140f::ComputeKey(KEYEVENT ke,int scancode)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)


}


bool Cce140f::init(void)
{
    CPObject::init();
    setfrequency( 0);
    pCONNECTOR	   = new Cconnector(this,11,0,Cconnector::Sharp_11,"Connector 11 pins",true,QPoint(597,310));	publish(pCONNECTOR);
    pCONNECTOR_Ext = new Cconnector(this,11,1,Cconnector::Sharp_11,"Connector 11 pins Ext.",false,QPoint(6,295));	publish(pCONNECTOR_Ext);
    WatchPoint.add(&pCONNECTOR_value,64,11,this,"Standard 11pins connector");
    WatchPoint.add(&pCONNECTOR_Ext_value,64,11,this,"Ext 11pins connector");

    AddLog(LOG_PRINTER,tr("CE-140F initializing..."));

    if(pKEYB)	pKEYB->init();
    if(pTIMER)	pTIMER->init();

//	SET_PIN(PIN_ACK,DOWN);
AddLog(LOG_PRINTER,tr("Initial value for PIN_BUSY %1").arg(GET_PIN(PIN_BUSY)?"1":"0"));
    Previous_PIN_BUSY = GET_PIN(PIN_BUSY);
    Previous_PIN_MT_OUT1 = GET_PIN(PIN_MT_OUT1);
    code_transfer_step = 0;
    device_code = 0;
    wait_data_function=0;
    halfdata = false;
    halfdata_out = false;
    //time.start();

    run_oldstate = -1;
    lastRunState = 0;

    MT_OUT2	= false;
    BUSY    = false;
    D_OUT	= false;
    MT_IN	= false;
    MT_OUT1	= false;
    D_IN	= false;
    ACK		= false;
    SEL2	= false;
    SEL1	= false;
    return true;
}

void Cce140f::contextMenuEvent ( QContextMenuEvent * event )
{
    QMenu *menu= new QMenu(this);

    BuildContextMenu(menu);
    menu->addSeparator();
    menu->addAction(tr("Define Directory"),this,SLOT(definePath()));
    menu->popup(event->globalPos () );
    event->accept();
}

void Cce140f::definePath(void){
//    MSG_ERROR("test")
    QString path = QFileDialog::getExistingDirectory (this, tr("Directory"));
    if ( path.isNull() == false )
    {
        directory.setPath(path);
    }
}

/*****************************************************/
/* Exit PRINTER										 */
/*****************************************************/
bool Cce140f::exit(void)
{
    AddLog(LOG_PRINTER,"CE-140F Closing...");
    AddLog(LOG_PRINTER,"done.");
    CPObject::exit();
    return true;
}


//********************************************************/
//* Check for E-PORT and Get data						 */
//********************************************************/
// PIN_MT_OUT2	1
// PIN_GND		2
// PIN_VGG		3
// PIN_BUSY		4
// PIN_D_OUT	5
// PIN_MT_IN	6
// PIN_MT_OUT1	7
// PIN_D_IN		8
// PIN_ACK		9
// PIN_SEL2		10
// PIN_SEL1		11
//********************************************************/



#define		WAIT ( pPC->frequency / 10000*6)

#define RECEIVE_MODE	1
#define SEND_MODE		2
#define TEST_MODE		3

extern int LogLevel;
//
// no bit start
// no bit stop
void Cce140f::pulldownsignal(void)
{

}

BYTE Cce140f::Pop_out8(void) {
    return data_out.takeFirst();
}

BYTE Cce140f::Pop_out4(void) {
    BYTE t=0;
    if (halfdata_out) {
        t = data_out.takeFirst();
        halfdata_out = false;

        AddLog(LOG_PRINTER,tr("byte from floppy : %1 - %2").arg(t,2,16).arg(QChar(t)) );
        t = (t >> 4);
    }
    else {
        halfdata_out = true;
        t = data_out.first() & 0x0F;
    }
    //AddLog(LOG_PRINTER,tr("4bits from floppy : %1").arg((t >> 4),2,16) );
    return t;
}

void Cce140f::Push8(BYTE b) {
    data.append(b);
}

void Cce140f::Push4(BYTE b) {
    if (halfdata) {
        int t = data.last()+(b<<4);
        data.pop_back();
        data.append(t);
        halfdata = false;
        AddLog(LOG_PRINTER,tr("byte to floppy : %1 - %2").arg(t,2,16).arg(QChar(t)) );
    }
    else {
        data.append(b);
        halfdata = true;
    }
}

bool Cce140f::Get_Connector(Cbus *_bus) {
    MT_OUT2	= GET_PIN(PIN_MT_OUT2);
    BUSY    = GET_PIN(PIN_BUSY);
    D_OUT	= GET_PIN(PIN_D_OUT);
    MT_IN	= GET_PIN(PIN_MT_IN);
    MT_OUT1	= GET_PIN(PIN_MT_OUT1);
    D_IN	= GET_PIN(PIN_D_IN);
    //ACK		= GET_PIN(PIN_ACK);
    SEL2	= GET_PIN(PIN_SEL2);
    SEL1	= GET_PIN(PIN_SEL1);

    return true;
}

bool Cce140f::Set_Connector(Cbus *_bus) {
    //MT_OUT2	= GET_PIN(PIN_MT_OUT2);
    //BUSY    = GET_PIN(PIN_BUSY);
    bool extD_OUT	= pCONNECTOR_Ext->Get_pin(PIN_D_OUT);
    bool extMT_IN	= pCONNECTOR_Ext->Get_pin(PIN_MT_IN);
    //MT_OUT1	= GET_PIN(PIN_MT_OUT1);
    bool extD_IN	= pCONNECTOR_Ext->Get_pin(PIN_D_IN);
    bool extACK		= pCONNECTOR_Ext->Get_pin(PIN_ACK);
//    bool extSEL2	= pCONNECTOR_Ext->Get_pin(PIN_SEL2);
//    bool extSEL1	= pCONNECTOR_Ext->Get_pin(PIN_SEL1);



    pCONNECTOR_Ext->Set_pin(PIN_MT_OUT2,MT_OUT2);
    pCONNECTOR_Ext->Set_pin(PIN_BUSY,BUSY);
    pCONNECTOR_Ext->Set_pin(PIN_D_OUT,D_OUT);
    pCONNECTOR_Ext->Set_pin(PIN_MT_IN,MT_IN);
    pCONNECTOR_Ext->Set_pin(PIN_MT_OUT1,MT_OUT1);
    pCONNECTOR_Ext->Set_pin(PIN_D_IN,D_IN);
    //pCONNECTOR_Ext->Set_pin(PIN_ACK,ACK || extACK);
    pCONNECTOR_Ext->Set_pin(PIN_SEL2,false);
    pCONNECTOR_Ext->Set_pin(PIN_SEL1,false);

    pCONNECTOR->Set_pin(PIN_MT_OUT2,MT_OUT2);
    pCONNECTOR->Set_pin(PIN_BUSY,BUSY);
    pCONNECTOR->Set_pin(PIN_D_OUT,D_OUT || extD_OUT);
    pCONNECTOR->Set_pin(PIN_MT_IN,MT_IN || extMT_IN);
    pCONNECTOR->Set_pin(PIN_MT_OUT1,MT_OUT1);
    pCONNECTOR->Set_pin(PIN_D_IN,D_IN || extD_IN);
    pCONNECTOR->Set_pin(PIN_ACK,ACK || extACK);
    pCONNECTOR->Set_pin(PIN_SEL2,SEL2);
    pCONNECTOR->Set_pin(PIN_SEL1,SEL1);

    return true;
}

#define CE126LATENCY (pTIMER->pPC->getfrequency()/3200)
//#define CE126LATENCY (getfrequency()/3200)
bool Cce140f::run(void)
{

    Get_Connector();

    bool bit = false;
    ce140f_Mode=RECEIVE_MODE;

//    pTAPECONNECTOR->Set_pin(3,(rmtSwitch ? GET_PIN(PIN_SEL1):true));       // RMT
//    pTAPECONNECTOR->Set_pin(2,GET_PIN(PIN_MT_OUT1));    // Out
//    SET_PIN(PIN_MT_IN,pTAPECONNECTOR->Get_pin(1));      // In


    pCONNECTOR_value = pCONNECTOR->Get_values();
    pCONNECTOR_Ext_value = pCONNECTOR_Ext->Get_values();
    //pCONNECTOR->Set_pin(PIN_ACK,pCONNECTOR->Get_pin(PIN_ACK) || pCONNECTOR_Ext->Get_pin(PIN_ACK));


#if 0
// Try to introduce a latency
    qint64			deltastate = 0;

    if (run_oldstate == -1) run_oldstate = pTIMER->state;
    deltastate = pTIMER->state - run_oldstate;
    if (deltastate < CE126LATENCY ) return true;
    run_oldstate	= pTIMER->state;
#endif


//	AddLog(LOG_PRINTER,tr("%1").arg(dump1));

    bool PIN_BUSY_GoDown = ( ( BUSY == DOWN ) && (Previous_PIN_BUSY == UP)) ? true:false;
    bool PIN_BUSY_GoUp   = ( ( BUSY == UP ) && (Previous_PIN_BUSY == DOWN)) ? true:false;
//    bool PIN_BUSY_Change = (GET_PIN(PIN_BUSY) != Previous_PIN_BUSY ) ? true:false;
//    bool PIN_MT_OUT1_GoDown = ( ( GET_PIN(PIN_MT_OUT1) == DOWN) &&	(Previous_PIN_MT_OUT1 == UP)) ? true:false;
//    bool PIN_MT_OUT1_GoUp = ( (Previous_PIN_MT_OUT1 == DOWN) && (GET_PIN(PIN_MT_OUT1) == UP )) ? true:false;
//    bool PIN_MT_OUT1_Change = (GET_PIN(PIN_MT_OUT1) != Previous_PIN_MT_OUT1 ) ? true:false;
//    bool PIN_D_OUT_Change = (GET_PIN(PIN_D_OUT) != Previous_PIN_D_OUT ) ? true:false;

    if (code_transfer_step >0) {
        lastRunState = pTIMER->state;
    }

    switch (code_transfer_step) {
    case 0 :    if ((MT_OUT1 == UP) && (D_OUT==UP)) {
                    lastState = pTIMER->state; //time.restart();
                    code_transfer_step=1;
                    if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(8);
                }
                break;
    case 1 :    if ((MT_OUT1 == UP) && (D_OUT==UP)) {
                    if (pTIMER->msElapsed(lastState) > 40) {
                        // Code transfer sequence started
                        // Raise ACK
                        code_transfer_step = 2;
                        ACK = UP;
                        if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(9);
                    }
                }
                else {
                    code_transfer_step=0;
                    //if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(7);
                }
                break;
    case 2:     if (BUSY == UP )	//check for BUSY
                {
                    if(D_OUT == UP)
                    {
                        bit = true;;
                    } else
                    {
                        bit = false;
                    }
                    t>>=1;
                    if (bit) t|=0x80;
                    if((c=(++c)&7)==0)
                    {
                        AddLog(LOG_PRINTER,tr("device code ce-140f : %1").arg(t,2,16) );
                        device_code = t;
                        //Printer(t);
                        ACK = DOWN;
                        if (device_code==0x41) {
                            code_transfer_step=4;
                        }
                        else
                            code_transfer_step = 0;
                        if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(37);
                        //t=0; c=0;
                    }
                    else {
                        ACK = DOWN;
                        code_transfer_step=3;
                        lastState=pTIMER->state;
                        if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(38);
                    }
                }
                break;
    case 3:     if (pTIMER->msElapsed(lastState)>2) {
                    code_transfer_step=2;
                    // wait 2 ms and raise ACK
                    ACK = UP;
                    if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(36);
                }
                break;
    case 4:     if ((BUSY == DOWN)&&(MT_OUT1 == DOWN)) {
                    ACK = UP;
                    code_transfer_step=5;
                    lastState=pTIMER->state;//time.restart();
                    t=0; c=0;
                    if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(35);
                }
                break;
    case 5:     if (pTIMER->msElapsed(lastState)>9) {
                    ACK = DOWN;
                    code_transfer_step=0;
                    if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(34);


                }
                break;
            }

    if ( (device_code == 0x41) && (code_transfer_step==0)) {
        if (PIN_BUSY_GoUp && (ACK==DOWN)) {
            lastRunState = pTIMER->state;
            // read the 4 bits

            t = SEL1 + (SEL2<<1) + (D_OUT<<2) + (D_IN<<3);

            //AddLog(LOG_PRINTER,tr("4bits to floppy : %1").arg(t,2,16) );
            Push4(t);
            ACK = UP;
            lastState=pTIMER->state; //time.restart();

            if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(30);
        }
        else
        if (PIN_BUSY_GoDown && (ACK==UP)) {
            lastRunState = pTIMER->state;
            ACK = DOWN;
            lastState=pTIMER->state;//time.restart();
            if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(31);
        }


    else

        if ( !data_out.empty() &&
             (pTIMER->msElapsed(lastState)>5) &&
             (BUSY==DOWN) &&
             (ACK==DOWN)) {
            lastRunState = pTIMER->state;
            BYTE t = Pop_out4();

            SEL1 = (t&0x01);
            SEL2 = ((t&0x02)>>1);
            D_OUT= ((t&0x04)>>2);
            D_IN = ((t&0x08)>>3);

            //NOTE  i have to pull down ext connector when data output
            pCONNECTOR_Ext->Set_pin(PIN_D_OUT,false);
            pCONNECTOR_Ext->Set_pin(PIN_MT_IN,false);
            pCONNECTOR_Ext->Set_pin(PIN_D_IN,false);
            pCONNECTOR_Ext->Set_pin(PIN_ACK,false);

            ACK = UP;
            if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(32);
        }

    else
        if ( (ACK==UP) && PIN_BUSY_GoUp) {
            lastRunState = pTIMER->state;
            ACK = DOWN;
            if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(33);
        }
    else
        if ( (pTIMER->msElapsed(lastState)>50) && !data.empty()) {
            lastRunState = pTIMER->state;
            if (mainwindow->dialoganalogic) mainwindow->dialoganalogic->setMarker(40);
            processCommand();
            data.clear();
        }

    }


    Previous_PIN_BUSY = BUSY;
    Previous_PIN_MT_OUT1 = MT_OUT1;
    Previous_PIN_D_OUT = D_OUT;

    if (pTIMER->msElapsed(lastRunState)<3000) {

        if (!busyLed) {
            busyLed = true;
            Refresh_Display = true;
            update();
        }
    }
    else {
        if (busyLed) {
            busyLed = false;
            Refresh_Display = true;
            update();
        }
    }
    Set_Connector();
    pCONNECTOR_value = pCONNECTOR->Get_values();
    pCONNECTOR_Ext_value = pCONNECTOR_Ext->Get_values();
    return true;
}

void Cce140f::processCommand(void) {
    if (data.isEmpty()) return;
    QString s ="";
    for (int i =0;i< data.size();i++) {
        s.append(QChar(data.at(i)));
    }
    AddLog(LOG_PRINTER,tr("floppy command : %1").arg(s));

    // Verify checksum
    int checksum = 0;
    for (int i =0;i< data.size()-1;i++) {
        checksum = (data.at(i)+checksum) & 0xff;
    }
    AddLog(LOG_PRINTER,tr("floppy command checksum = %1 compared to %2").arg(checksum,2,16).arg(data.last(),2,16));

    if (wait_data_function >0) {
        data.prepend(wait_data_function);
        wait_data_function = 0;
    }

    switch (data.first()) {
    //        case 0x03: process_OPEN(0);break;
    case 0x04: process_CLOSE(0);break;
    case 0x05: process_FILES();break;
    case 0x06: process_FILES_LIST(0);break;
    case 0x07: process_FILES_LIST(1);break;
    case 0x08: process_INIT(0x08);break;
    case 0x09: process_INIT(0x09);break;
    case 0x0A: process_KILL(0x0A);break;
        //    case 0x0B: process_NAME(0x0B);break;
        //    case 0x0C: process_SET(0x0C);break;
        //    case 0x0D: process_COPY(0x0D);break;
    case 0x0E: process_LOAD(0x0E);break;
    case 0x0F: process_LOAD(0x0F);break;
    case 0x10: process_SAVE(0x10);break;
    case 0x11: process_SAVE(0x11);break;
    case 0x12: process_LOAD(0x12);break;
        //    case 0x13: process_INPUT(0x13);break;
        //    case 0x14: process_INPUT(0x14);break;
//    case 0x15: process_PRINT(0x15);break;
    case 0x16: process_SAVE(0x16);break;       // SAVE ASCII
    case 0x17: process_LOAD(0x17);break;
        //    case 0x1A: process_EOF(0x1A);break;
        //    case 0x1C: process_LOC(0x1C);break;
    case 0x1D: process_DSKF(); break;
        //    case 0x1F: process_INPUT(0x1f);break;
        //    case 0x20: process_INPUT(0x20);break;
    case 0xFE: process_SAVE(0xfe);break;    // Handle ascii saved data stream
    case 0xFF: process_SAVE(0xff);break;    // Handle saved data stream
    default: emit msgError("CE-140F : Unknown command.");
    }
}

void Cce140f::process_DSKF(void) {
    AddLog(LOG_PRINTER,tr("Process DSKF(%1)").arg(data.at(1)));
    data_out.append(0x00);

    data_out.append(0x01);
    data_out.append(0x02);
    data_out.append(0x04);
    data_out.append(0x07);

//    data_out.append(0x49);
}

void Cce140f::process_FILES(void) {
    AddLog(LOG_PRINTER,tr("Process FILES(%1)").arg(data.at(1)));
    data_out.append(0x00);
    checksum = 0;

    // Send nb files
    QString s ="";
    for (int i =3;i< 15;i++) {
        s.append(QChar(data.at(i)));
    }
//    s="*.BAS";
    AddLog(LOG_PRINTER,s.replace(" ",""));
    fileList = directory.entryList( QStringList() << s.replace(" ",""),QDir::Files);
    data_out.append(CheckSum(fileList.size()));
    fileCount = -1;

    // Send CheckSum
    data_out.append(checksum);

}
QString Cce140f::cleanFileName(QString s) {
    QString r = "X:";
    r = r + s.left(s.indexOf(".")).leftJustified(8,' ',true) + s.mid(s.indexOf(".")).rightJustified(4,' ',true);
    return r;
}

void Cce140f::process_FILES_LIST(int cmd) {
    QString fname;
    switch (cmd) {
    case 0:
            AddLog(LOG_PRINTER,tr("Process FILES_LIST"));
            data_out.append(0x00);

            checksum=0;

            fileCount++;
            fname = fileList.at(fileCount);

            AddLog(LOG_PRINTER,"**"+cleanFileName(fname)+"**");
            // Send filenames
            //sendString("X:TEST    .BAS ");
            sendString(cleanFileName(fname));
            sendString(" ");

            // Send CheckSum
            data_out.append(checksum);
            break;

    case 1:

            AddLog(LOG_PRINTER,tr("Process FILES_LIST"));
            data_out.append(0x00);

            checksum=0;
            fileCount--;
            fname = fileList.at(fileCount);

            AddLog(LOG_PRINTER,"**"+cleanFileName(fname)+"**");
            // Send filenames
            //sendString("X:TEST    .BAS ");
            sendString(cleanFileName(fname));
            sendString(" ");

            // Send CheckSum
            data_out.append(checksum);

            break;
    }
}

void Cce140f::process_INIT(int cmd) {
    switch (cmd) {
    case 0x08:
        AddLog(LOG_PRINTER,tr("Process INIT(%1)").arg(data.at(1)));
        data_out.append(0x00);
        break;
    case 0x09:
        AddLog(LOG_PRINTER,tr("Process INIT2(%1)").arg(data.at(1)));
        data_out.append(0x00);
        break;

    }
}

void Cce140f::process_SAVE(int cmd) {
    QString s = "";
    switch (cmd) {

    case 0x10:
            // create file

            for (int i=3;i<15;i++) {
                s.append(QChar(data.at(i)));
            }
            AddLog(LOG_PRINTER,tr("process_SAVE file:%1").arg(s));

            file_save.setFileName(s);

            if (!file_save.open(QIODevice::WriteOnly)) {
                emit msgError(tr("ERROR creating file : %1").arg(s));
            }

            data_out.append(0x00);
            break;
    case 0x11:

            AddLog(LOG_PRINTER,tr("process_SAVE1"));

            // transmet 5 bytes : the size ?
            // 1 : 00
            // 2 : Size 1
            // 3 : Size 2
            // 4 : Size 3
            // 5 : checksum
            file_size = data.at(2) + (data.at(3)<<8) + (data.at(4)<<16);
            AddLog(LOG_PRINTER,tr("process_SAVE file SIZE: %1 ").arg(file_size));
            data_out.append(0x00);
            wait_data_function = 0xff;
            break;
    case 0x16:
            for (int i=1;i<15;i++) {
                s.append(QChar(data.at(i)));
            }
            AddLog(LOG_PRINTER,tr("process_SAVE ASCIIfile:%1").arg(s));

            // transmet 4 bytes : the lenght ?
            data_out.append(0x00);
            wait_data_function = 0xfe;
            break;
    case 0xfe: // received ascii data steam
            //store data
            AddLog(LOG_PRINTER,tr("retreived ascii data stream"));
            data_out.append(0x00);

            break;
    case 0xff: // received data
            // remove command from he stream
            data.removeFirst();
            //store data
            // WARNING: received by 256Bytes paquet
            QDataStream out(&file_save);

            for (int i=0;i<data.size()-1;i++) {
                out << (qint8) data.at(i);
                AddLog(LOG_PRINTER,tr("retreived data stream %1 - %2 (%3)").arg(i).arg(data.at(i),2,16).arg(QChar(data.at(i))))
            }
            file_size -= data.size();
            if (file_size>0) {
                wait_data_function = 0xff;
            }else {
                file_save.close();
            }
            // check the checksum!!!!!
            AddLog(LOG_PRINTER,tr("retreived data stream"));
            data_out.append(0x00);
            data.clear();
            break;
        }
}

void Cce140f::process_LOAD(int cmd) {
    QString s = "";
    BYTE c=0;

    switch (cmd) {

    case 0x0E:
            // open file

            for (int i=3;i<15;i++) {
                s.append(QChar(data.at(i)));
            }
            AddLog(LOG_PRINTER,tr("process_LOAD file:%1").arg(s));

            file_load.setFileName(s);

            if (!file_load.open(QIODevice::ReadOnly)) {
                emit msgError(tr("ERROR opening file : %1").arg(s));
            }

            file_size = file_load.size();
            AddLog(LOG_PRINTER,tr("file size %1").arg(file_size));
            data_out.append(0x00);
            checksum = 0;
            sendString(" ");

            // Send size : 3 bytes + checksum
            data_out.append(CheckSum(file_size & 0xff));
            data_out.append(CheckSum((file_size >> 8) & 0xff));
            data_out.append(CheckSum((file_size >> 16) & 0xff));
            data_out.append(checksum);
            ba_load = file_load.readAll();
            break;
    case 0x17:
            data_out.append(0x00);
            checksum=0;

            // Send first byte


            //ba_load.remove(0,0x0f);            // remove first byte 'ff'

            //ba_load.chop(1);
            data_out.append(CheckSum(ba_load.at(0)));
            ba_load.remove(0,0x01);
            //data_out.append(CheckSum(0x0f));
            data_out.append(checksum);
            //ba_load.remove(0,0x10);
            //wait_data_function = 0xfd;
            break;
    case 0x12:

            // send data
            data_out.append(0x00);
            checksum=0;

            // Envoyer une ligne complete
            // Until 0x0D
            // EOF = 0x1A
            // Start at 0x10

            do {
                c=ba_load.at(0);
                data_out.append(CheckSum(c));  // 0x1A pour fin de fichier
                ba_load.remove(0,1);
            }
            while ((ba_load.size()>0) && (c!=0x0d));

            if (c!=0x0d) {
                if (ba_load.isEmpty()) {
                data_out.append(CheckSum(0x1A));  // 0x1A pour fin de fichier
                }
            }

            data_out.append(checksum);
            data_out.append(0x00);
            break;
    case 0x0f:
            data_out.append(0x00);
            checksum=0;

            file_size = ba_load.size();

            for (int i=0;i<file_size;i++) {
                c=ba_load.at(i);
                data_out.append(CheckSum(c));
                AddLog(LOG_PRINTER,tr("send to output %1").arg(i))
                if (((i+1)%0x100)==0) {
                   data_out.append(checksum);
                   checksum=0;
                   AddLog(LOG_PRINTER,tr("CHECKSUM "))
                }
                //AddLog(LOG_PRINTER,tr("send to output (%1) : %2 - %3").arg(i+j*0x100).arg(c,2,16).arg(QChar(c)));
            }
            if ((file_size%0x100)) data_out.append(checksum);
            data_out.append(0x00);
            break;

        }
}

void Cce140f::process_KILL(int cmd) {
    QString s = "";
    switch (cmd) {
    case 0x0A:
        // delete file

        for (int i=3;i<15;i++) {
            s.append(QChar(data.at(i)));
        }
        AddLog(LOG_PRINTER,tr("process_KILL file:%1").arg(s));

        QFile::remove(directory.absoluteFilePath(s));
        data_out.append(0x00);
        break;
    }
}

void Cce140f::process_CLOSE(int cmd) {
    QString s = "";
    BYTE arg = data.at(1);
    AddLog(LOG_PRINTER,tr("process_CLOSE qrg=%1").arg(arg));
    switch (arg) {

    case 0: // Close files open
        // si 0xFF close all files
        // si 0-8 close the corresponding file
        data_out.append(0x00);
        break;
    case 0xff: // Close all files
        data_out.append(0xff);
        AddLog(LOG_PRINTER,tr("CLOSE all files"));
        break;
    }

}

void Cce140f::sendString(QString s) {
    for (int i=0;i<s.size();i++){
        data_out.append(CheckSum(s.at(i).toLatin1()));
    }
}

BYTE Cce140f::CheckSum(BYTE b) {
    checksum = (checksum + b) & 0xff;
    return b;
}
