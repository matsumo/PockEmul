


#include <QPainter>
#include <QTime>

#include <QFileDialog>
#include <QThread>
#include <QDebug>

#include "common.h"

#include "pc2021.h"
#include "pcxxxx.h"
#include "Log.h"
#include "watchpoint.h"
#include "paperwidget.h"
#include "Keyb.h"
#include "Connect.h"

#define DOWN	0
#define UP		1

TransMap KeyMappc2021[]={
    {1,	"FEED  ",	K_PFEED,34,234,	9},
    {2,	"POWER ON",	K_POW_ON,34,234,	9},
    {3,	"POWER OFF",K_POW_OFF,34,234,	9}
};
int KeyMappc2021Lenght = 3;

Cpc2021::Cpc2021(CPObject *parent):Cprinter(parent)
{								//[constructor]
    setfrequency( 0);
    pc2021buf	= 0;
    pc2021display= 0;
    //bells		= 0;
    charTable = 0;
    margin = 25;
    ToDestroy	= false;
    BackGroundFname	= P_RES(":/ext/pc-2021.png");
    setcfgfname("pc2021");

    settop(10);
    setposX(0);

    pTIMER		= new Ctimer(this);
    KeyMap      = KeyMappc2021;
    KeyMapLenght= KeyMappc2021Lenght;
    pKEYB->setMap("pc2021.map");
    setDXmm(108);
    setDYmm(130);
    setDZmm(43);

    setDX(386);
    setDY(464);

    setPaperPos(QRect(70,-3,275,149));

    ctrl_char = false;
    t = 0;
    c = 0;
    rmtSwitch = false;

    internal_device_code = 0x0f;
}

Cpc2021::~Cpc2021() {
    delete pc2021buf;
    delete pc2021display;
    delete pCONNECTOR;
    delete charTable;
//    delete bells;
}

void Cpc2021::ComputeKey(KEYEVENT ke, int scancode, QMouseEvent *event)
{
    Q_UNUSED(ke)
    Q_UNUSED(scancode)

    if (pKEYB->LastKey == K_PFEED) {
        Refreshpc2021(0x0d);
    }
}


void Cpc2021::SaveAsText(void)
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

//    mainwindow->grabKeyboard();
        AddLog(LOG_PRINTER,TextBuffer.data());
}

void Cpc2021::Refreshpc2021(qint8 data)
{

    QPainter painter;

//if (posX==0) bells->play();

// copy ce126buf to ce126display
// The final paper image is 207 x 149 at (277,0) for the ce125

// grab data char to byteArray
    if ( (data == 0xff) || (data==0x0a)) return;

    TextBuffer += data;

    if (data == 0x0d){
        top+=10;
        posX=0;
//        qWarning()<<"CR PRINTED";
    }
    else
    {
        painter.begin(pc2021buf);
        int x = ((data>>4) & 0x0F)*6;
        int y = (data & 0x0F) * 8;
        painter.drawImage(	QPointF( margin + (7 * posX),top),
                            *charTable,
                            QRectF( x , y , 5,7));
        posX++;
        painter.end();
    }

    if (posX >= 40) {
        posX=0;
        top+=10;
    }

    painter.begin(pc2021display);

    painter.drawImage(QRectF(0,MAX(149-top,0),330,MIN(top,149)),*pc2021buf,QRectF(0,MAX(0,top-149),340,MIN(top,149)));

// Draw printer head
//    painter.fillRect(QRect(0 , 147,407,2),QBrush(QColor(0,0,0)));
//    painter.fillRect(QRect(21 + (7 * posX) , 147,14,2),QBrush(QColor(255,255,255)));

    painter.end();


    paperWidget->setOffset(QPoint(0,top));
    paperWidget->updated = true;

    Refresh_Display = true;

}


/*****************************************************/
/* Initialize PRINTER								 */
/*****************************************************/
void Cpc2021::clearPaper(void)
{
    // Fill it blank
    pc2021buf->fill(PaperColor.rgba());
    pc2021display->fill(QColor(255,255,255,0).rgba());
    settop(10);
    setposX(0);
    // empty TextBuffer
    TextBuffer.clear();
    paperWidget->updated = true;
}


bool Cpc2021::init(void)
{
    CPObject::init();

    setfrequency( 0);

    pCONNECTOR	= new Cconnector(this,
                                 9,
                                 0,
                                 Cconnector::DIN_8,
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
    pc2021buf	= new QImage(QSize(340, 1000),QImage::Format_ARGB32);
    pc2021display= new QImage(QSize(340, 149),QImage::Format_ARGB32);


//TODO Update the chartable with upd16343 char table
    charTable = new QImage(P_RES(":/ext/ce126ptable.bmp"));

//	bells	 = new QSound("ce.wav");

// Create a paper widget

    paperWidget = new CpaperWidget(PaperPos(),pc2021buf,this);
    paperWidget->updated = true;
    paperWidget->show();

    // Fill it blank
    clearPaper();

    run_oldstate = -1;

    return true;
}



/*****************************************************/
/* Exit PRINTER										 */
/*****************************************************/
bool Cpc2021::exit(void)
{
    AddLog(LOG_PRINTER,"PRT Closing...");
    AddLog(LOG_PRINTER,"done.");
    Cprinter::exit();
    return true;
}


/*****************************************************/
/* CE-126P PRINTER emulation						 */
/*****************************************************/

void Cpc2021::Printer(qint8 d)
{
    if(ctrl_char && d==0x20) {
        ctrl_char=false;
        Refreshpc2021(d);
    }
    else
    {
        if(d==0xf || d==0xe || d==0x03)
            ctrl_char=true;
        else
        {
            Refreshpc2021(d);
        }
    }
}


#define		WAIT ( pPC->frequency / 10000*6)

#define RECEIVE_MODE	1
#define SEND_MODE		2
#define TEST_MODE		3



bool Cpc2021::Get_Connector(Cbus *_bus) {
    return true;
}

bool Cpc2021::Set_Connector(Cbus *_bus) {
    return true;
}


#define PC2021LATENCY (pTIMER->pPC->getfrequency()/3200)

bool Cpc2021::run(void)
{

    Get_Connector();

#if 1
// Try to introduce a latency
    quint64	deltastate = 0;

    if (run_oldstate == -1) run_oldstate = pTIMER->state;
    deltastate = pTIMER->state - run_oldstate;
    if (deltastate < PC2021LATENCY ) return true;
    run_oldstate	= pTIMER->state;
#endif

    quint8 c = pCONNECTOR->Get_values();

    if (c>0)
    {
        AddLog(LOG_PRINTER,QString("Recieve:%1 = (%2)").arg(c,2,16,QChar('0')).arg(QChar(c)));
        SET_PIN(9,1);
        Printer(c);
    }

    pCONNECTOR_value = pCONNECTOR->Get_values();


    pc2021buf = checkPaper(pc2021buf,top);

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
