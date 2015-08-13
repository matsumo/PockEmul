


#include <QPainter>
#include <QTime>

#include <QFileDialog>
#include <QThread>
#include <QDebug>

#include "common.h"

#include "pc100.h"
#include "pcxxxx.h"
#include "Log.h"
#include "watchpoint.h"
#include "paperwidget.h"
#include "Keyb.h"
#include "Connect.h"

#define DOWN	0
#define UP		1

//TransMap KeyMappc100[]={
//    {1,	"FEED  ",	K_PFEED,34,234,	9},
//    {2,	"POWER ON",	K_POW_ON,34,234,	9},
//    {3,	"POWER OFF",K_POW_OFF,34,234,	9}
//};
//int KeyMappc100Lenght = 3;

static const unsigned char PC100_CODE[64] = {
  ' ','0','1','2','3','4','5','6','7','8','9','A','B','C','D','E',
  '-','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T',
  '.','U','V','W','X','Y','Z','+','x','*','s','p','e','(',')',',',
  '^','%','_','/','=','\'',0x9E,'&','z','?',':','!',']',0x7F,'[','S'
};

static const struct {
  unsigned char code;
  char str[4];
  char intstr[4];
} PC100_STR[] = {
  {0x00, "   ","\x00\x00\x00"},
  {0x11, " = ","\x00\x34\x00"},
  {0x12, " - ","\x00\x10\x00"},
  {0x13, " + ","\x00\x27\x00"},
  {0x16, " / ","\x00\x33\x00"},
  {0x17, " x ","\x00\x28\x00"},
  {0x1A, "xsY","\x28\x2A\x25"}, //x_sqrt_Y
  {0x1B, "Y^x","\x25\x30\x28"}, //Yx_
  {0x21, "CLR","\x0D\x17\x1D"},
  {0x22, "INV","\x14\x19\x22"},
  {0x23, "DPT","\x0E\x1B\x1F"},
  {0x26, "CE ","\x0D\x0F\x00"},
  {0x27, "+/-","\x27\x33\x10"},
  {0x2D, "EE ","\x0F\x0F\x00"},
  {0x31, "e^x","\x2C\x30\x28"}, // ex_
  {0x33, "x^2","\x28\x30\x03"}, // x2_
  {0x36, "1/x","\x00\x00\x00"},
  {0x3C, "sX ","\x00\x00\x00"}, // sqrt_X_
  {0x3D, "X_Y","\x00\x00\x00"}, // X exchange Y ??
  {0x51, "LNX","\x00\x00\x00"},
  {0x53, "PRM","\x00\x00\x00"},
  {0x54, " % ","\x00\x00\x00"},
  {0x56, "COS","\x00\x00\x00"},
  {0x57, "SIN","\x00\x00\x00"},
  {0x5D, "TAN","\x00\x00\x00"},
  {0x61, "SUM","\x00\x00\x00"},
  {0x66, "STO","\x00\x00\x00"},
  {0x67, "pi ","\x00\x00\x00"}, //_pi_
  {0x68, "RCL","\x00\x00\x00"},
  {0x69, "S+ ","\x00\x00\x00"},
  {0x70, "ERR","\x00\x00\x00"},
  {0x71, " { ","\x00\x00\x00"},
  {0x72, " ) ","\x00\x00\x00"},
  {0x73, "LRN","\x00\x00\x00"},
  {0x74, "RUN","\x00\x00\x00"},
  {0x76, "HLT","\x00\x00\x00"},
  {0x78, "STP","\x00\x00\x00"},
  {0x7A, "GTO","\x00\x00\x00"},
  {0x7C, "IF ","\x00\x00\x00"},
  {0, {0}, {0}}
};

Cpc100::Cpc100(CPObject *parent):Cprinter(parent)
{								//[constructor]
    setfrequency( 0);
    pc100buf	= 0;
    pc100display= 0;
    //bells		= 0;
    charTable = 0;
    margin = 25;
    ToDestroy	= false;
    BackGroundFname	= P_RES(":/ti59/pc100c.png");
    setcfgfname("pc100");

    settop(10);
    setposX(0);

    pTIMER		= new Ctimer(this);
//    KeyMap      = KeyMappc100;
//    KeyMapLenght= KeyMappc100Lenght;
    pKEYB		= new Ckeyb(this,"pc100.map");
    setDXmm(287);
    setDYmm(265);
    setDZmm(104);

    setDX(999);
    setDY(923);



    ctrl_char = false;
    t = 0;
    c = 0;
}

Cpc100::~Cpc100() {
    delete pc100buf;
    delete pc100display;
    delete pCONNECTOR;
    delete charTable;
//    delete bells;
}

#define KEY(c)	( pKEYB->keyPressedList.contains(TOUPPER(c)) || pKEYB->keyPressedList.contains(c) || pKEYB->keyPressedList.contains(TOLOWER(c)))

void Cpc100::ComputeKey(KEYEVENT ke, int scancode, QMouseEvent *event)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)


    print = KEY(K_PRINT);
    trace = KEY(K_PRT_TRACE);
    adv = KEY(K_PFEED);

//    if (KEY(K_PRT_TRACE)) {
//        trace = ! trace;
//    }
}


void Cpc100::SaveAsText(void)
{
    mainwindow->releaseKeyboard();

    QString s = QFileDialog::getSaveFileName(
                    mainwindow,
                    tr("Choose a filename to save under"),
                    ".",
                   tr("Text File (*.txt)"));

    QFile file(s);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    file.write(TextBuffer);

    mainwindow->grabKeyboard();
        AddLog(LOG_PRINTER,TextBuffer.data());
}

void Cpc100::drawChar(quint8 code)
{
    QPainter painter;
    painter.begin(pc100buf);
    int x = ((code>>4) & 0x0F)*6;
    int y = (code & 0x0F) * 8;
    painter.drawImage(	QPointF( margin + (7 * posX),top),
                        *charTable,
                        QRectF( x , y , 5,7));
    posX++;
    painter.end();
}

void Cpc100::Refreshpc100(quint8 data)
{
    QPainter painter;

    if (data <= 0x7F) {
        for (int i = 0; *PC100_STR[i].str; i++) {
            if (data == PC100_STR[i].code) {
                for (int ind = 3; ind; ) {
                    lineBuffer.prepend(PC100_STR[i].intstr[--ind]);
                    qWarning()<<lineBuffer;
                }
                TextBuffer += PC100_STR[i].str;
            }
        }
    }
    else if (data < 0xF0) {
        lineBuffer.prepend(data&0x3F);
        TextBuffer += PC100_CODE[data&0x3F];
        qWarning()<<lineBuffer;
    }
    else if (data == 0xF3) {
        top+=10;
        posX=0;

    }
    else if (data == 0xF1)
    {
        lineBuffer.clear();
        posX=0;
    }

    else if (data == 0xF2)
    {
        painter.begin(pc100buf);
        for (int i=0;(i<20)&&(i<lineBuffer.size());i++) {
            quint8 _c = lineBuffer.at(i);
            int x = (_c & 0x0F)*6;
            int y = ((_c>>4) & 0x0F) * 8;
            painter.drawImage(	QPointF( margin + (7 * posX),top),
                                *charTable,
                                QRectF( x , y , 5,7));
            posX++;
        }
        painter.end();
        top+=10;
        posX=0;
        lineBuffer.clear();
    }
    if (posX >= 20) {
        posX=0;
//        top+=10;
    }


    painter.begin(pc100display);

    painter.drawImage(QRectF(0,MAX(149-top,0),330,MIN(top,149)),*pc100buf,QRectF(0,MAX(0,top-149),340,MIN(top,149)));

// Draw printer head
//    painter.fillRect(QRect(0 , 147,407,2),QBrush(QColor(0,0,0)));
//    painter.fillRect(QRect(21 + (7 * posX) , 147,14,2),QBrush(QColor(255,255,255)));

    painter.end();

    Refresh_Display = true;

    paperWidget->setOffset(QPoint(0,top));
    paperWidget->updated = true;

}


/*****************************************************/
/* Initialize PRINTER								 */
/*****************************************************/
void Cpc100::clearPaper(void)
{
    // Fill it blank
    pc100buf->fill(PaperColor.rgba());
    pc100display->fill(QColor(255,255,255,0).rgba());
    settop(10);
    setposX(0);
    // empty TextBuffer
    TextBuffer.clear();
    paperWidget->updated = true;
}


bool Cpc100::init(void)
{
    CPObject::init();

    setfrequency( 0);

    pCONNECTOR	= new Cconnector(this,
                                 9,
                                 0,
                                 Cconnector::TI_8,
                                 "Printer connector",
                                 true,
                                 QPoint(386,238),
                                 Cconnector::EAST);	publish(pCONNECTOR);
    WatchPoint.add(&pCONNECTOR_value,64,11,this,"Printer connector");
    AddLog(LOG_PRINTER,tr("PRT initializing..."));

    if(pKEYB)	pKEYB->init();
    if(pTIMER)	pTIMER->init();

    // Create CE-126 Paper Image
    // The final paper image is 207 x 149 at (277,0) for the ce125
    pc100buf	= new QImage(QSize(207, 1000),QImage::Format_ARGB32);
    pc100display= new QImage(QSize(207, 149),QImage::Format_ARGB32);


//TODO Update the chartable with upd16343 char table
    charTable = new QImage(P_RES(":/ti59/pc100table.bmp"));

//	bells	 = new QSound("ce.wav");

    QRect _r = pKEYB->getKey(K_PAPER_POS).Rect;
    qWarning()<<"Paperpos:"<<_r;
    setPaperPos(_r);

// Create a paper widget

    paperWidget = new CpaperWidget(PaperPos(),pc100buf,this);
    paperWidget->updated = true;
    paperWidget->show();

    // Fill it blank
    clearPaper();

    run_oldstate = 0;

    print = trace = adv = false;

    return true;
}



/*****************************************************/
/* Exit PRINTER										 */
/*****************************************************/
bool Cpc100::exit(void)
{
    AddLog(LOG_PRINTER,"PRT Closing...");
    AddLog(LOG_PRINTER,"done.");
    Cprinter::exit();
    return true;
}


/*****************************************************/
/* CE-126P PRINTER emulation						 */
/*****************************************************/

void Cpc100::Printer(qint8 d)
{
    if(ctrl_char && d==0x20) {
        ctrl_char=false;
        Refreshpc100(d);
    }
    else
    {
        if(d==0xf || d==0xe || d==0x03)
            ctrl_char=true;
        else
        {
            Refreshpc100(d);
        }
    }
}


#define		WAIT ( pPC->frequency / 10000*6)

#define RECEIVE_MODE	1
#define SEND_MODE		2
#define TEST_MODE		3



bool Cpc100::Get_Connector(Cbus *_bus) {
    Q_UNUSED(_bus)

    return true;
}

bool Cpc100::Set_Connector(Cbus *_bus) {
    Q_UNUSED(_bus)

    SET_PIN(10,1);
    SET_PIN(11,print ? 1 : 0);
    SET_PIN(12,trace ? 1 : 0);
    SET_PIN(13,adv   ? 1 : 0);

    return true;
}


#define pc100LATENCY (pTIMER->pPC->getfrequency()/3200)

bool Cpc100::run(void)
{

    Get_Connector();

#if 1
// Try to introduce a latency
    quint64	deltastate = 0;

    if (run_oldstate == 0) run_oldstate = pTIMER->state;
    deltastate = pTIMER->state - run_oldstate;
    if (deltastate < (quint64)pc100LATENCY ) return true;
    run_oldstate	= pTIMER->state;
#endif

    quint8 c = pCONNECTOR->Get_values();

    if (c>0)
    {
        AddLog(LOG_PRINTER,QString("Recieve:%1 = (%2)").arg(c,2,16,QChar('0')).arg(QChar(c)));
        SET_PIN(9,1);
//        Printer(c);
        Refreshpc100(c);
    }




    pCONNECTOR_value = pCONNECTOR->Get_values();


    pc100buf = checkPaper(pc100buf,top);

    Set_Connector();

    return true;
}

/*
 *Nom du type: PC-2021
Nom du produit: imprimante thermique 40 chiffres
Prix: 23800
Date de livraison: 83/12
Méthode d'impression: transfert thermique (matricielle thermique)
Disponible format de papier A5: × A4: × A3: × B5: × B4: ×
Disponible feuille de coupe de type de papier: × continue: × copie: × roll: ○
　　　　　　　　　  Carte postale: × OHP: × papier thermique: ○
fil de la tête: 8
Interface: Centro série
Nombre de colonnes d'impression pica: 40
　　　　　　　Kanji: plus de
Dot Pixels pica: 7 × 5
　　　　　　　  Kanji: plus de
Pica vitesse d'impression: 40
　　　　　　　Kanji: plus de
Direction nouvelle ligne: avant
largeur de saut de ligne:
Nouvelle vitesse de la ligne:
Direction Impression: un moyen
Tampon:
1er niveau Kanji: plus de
Le deuxième niveau de kanji: plus de
Couleur:
Remarques Couleur: Non
Copie couleur pension:
Ankh italique:
　　　　　　　       Gothique:
　　　　　　　       Courrier:
Taille de kanji: Caractère Dimensions: 1,02 mm x 2,05 mm
Standard chinois de la police: Type de caractère: 160 caractères (alphanumérique, minuscules, katakana, symbole de kana)
　　　　　　　Options:
méthode d'alimentation du papier feuille d'emploi: plus de
　　　　　　　Alimentation feuille à feuille: plus de
　　　　　　　Tracteur chargeur: sur
　　　　　　　Poussez tracteur: plus de
　　　　　　　Pin alimentation: plus de
　　　　　　　Guide Carte postale: sur
　　　　　　　support de papier rouleau: norme
Type continu papier (1): x
　　　　　　　-Continu (2): x
　　　　　　　Continue: ×
　　　　　　　Copie: ×
　　　　　　　Rouler: largeur de 80mm
　　　　　　　Carte postale: ×
　　　　　　　OHP: ×
　　　　　　　Papier thermique: ○
Caractère externe nombre de caractères d'enregistrement:
Télécharger nombre de caractères:
Les conditions de température de fonctionnement: 0-40
　　　　　　　　Stockage: -25 à 60
conditions d'humidité de fonctionnement: 30-85
　　　　　　　　Stockage: 5-90
Consommation électrique (W) d'exploitation: 0,4
　　　　　　　　Stockage: 1.8
Poids (kg): 0,44
VCCI correspond:
Dimensions (mm) (W): 110 (D): 130 (H): 40
Article ci-joint: rouleau de papier thermique (PC-2021-P1)
　　　: Piles alcalines AA (4 pcs.)
　　　: Manuel, certificat
*/

