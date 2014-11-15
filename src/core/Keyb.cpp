#include <QDebug>

#include "xmlwriter.h"
 
#include "common.h"
#include "init.h"
#include "pcxxxx.h"
#include "cpu.h"
#include "Log.h"
#include "Inter.h"
#include "Keyb.h"


//extern CPocketThread* PcThread;
INLINE BYTE bit(int ii)
{
    BYTE jj;

    for (jj= 0; jj < 8; jj++)
	{
		if (ii & 0x01) return(jj);
		ii >>= 1;
    }
    return(jj);
}

typedef struct{
	BYTE pc1350,scan;
}	Transkey;

const int RIGHT[11] = { 25 , 32 , 59 , 19, 21, 13, 21, 16, 31,16,16 };
const int BOTTOM[11]= { 16 , 22 , 16 , 21, 10, 13, 13, 31, 31,16,16 };

CKey::CKey()
{
    this->ScanCode	= 0;
    this->enabled = false;
    this->Rect = QRect();
}

CKey::CKey(int scancode, QString description,QRect rect,int masterscancode,QString modifier,View view)
{
    this->Description = description;
    this->ScanCode	= scancode;
    this->MasterScanCode = masterscancode;
    this->Modifier = modifier;
    this->Rect = rect;
    this->view = view;
    this->enabled = true;
}

Ckeyb::Ckeyb(CPObject *parent,QString map,BYTE *scan) //: CPObject(parent)								//[constructor]
{
    pPC = (CpcXXXX *)parent;
    Parent	= parent;
    for(int i=0;i<MAX_KO;i++) pc1350KeyStatus[i]=0;
    for(int j=0;j<200;j++) keym[j]=0;
    access		= 0;							//ko port access?(0:none, 1:access)
    KStrobe		= 0;
    IA_PORT		= 0;
    Kon			= false;
    scandef     = scan;
    fn_KeyMap	= map;
    modified = false;
    handler = new KEYBMAPParser(this);
    LastKey = lastMousePressedKey = 0;
}

Ckeyb::~Ckeyb() {
    delete handler;
}

int Ckeyb::KeyClick(QPoint pts)
{
//    qWarning()<<"keyclick:"<<pts;
    // calculate all distance betwwen pts and keys centers
    // Keep the nearest

    int nearestIndex = -1;
    int smallerDistance = 99999;
    for (int i=0;i<Keys.size();i++)
    {
        if (Keys.at(i).view != pPC->currentView) continue;
        if (!(Keys.at(i).enabled)) continue;
        QRect r = Keys.at(i).Rect;
            r.setCoords(r.x()*mainwindow->zoom/100,r.y()*mainwindow->zoom/100,(r.x()+r.width())*mainwindow->zoom/100,(r.y()+r.height())*mainwindow->zoom/100);
            int tmpDistance = 0;
            if ( r.contains(pts) ) {
                tmpDistance = 0;

            }
            else {
                tmpDistance = (r.center()-pts).manhattanLength();
            }

            if (tmpDistance == smallerDistance) {
                // compare key size and keep the smallest
                QRect r1 = Keys.at(nearestIndex).Rect;
                if (r.width()*r.height() < r1.width()*r1.height()) {
                    nearestIndex = i;
                }
            }
            else if (tmpDistance < smallerDistance) {
                smallerDistance = tmpDistance;
                nearestIndex= i;
            }

    }
//    qWarning()<<"smallerDist:"<<smallerDistance<<nearestIndex<<(30*mainwindow->zoom/100);
    if ((smallerDistance < (30*mainwindow->zoom/100)) && (nearestIndex>=0)) {
        if (!pPC->closed) {
//            qWarning()<<"OPEN-return key:"<<Keys.at(nearestIndex).ScanCode;
            return Keys.at(nearestIndex).ScanCode;
        }
        else
        {
//            qWarning()<<"CLOSE";
            if (Keys.at(nearestIndex).ScanCode == K_CLOSE) return Keys.at(nearestIndex).ScanCode;
        }
    }
    return(0);
}

QString Ckeyb::KeyString(QPoint pts)
{
	// Keys iterator 
	QList<CKey>::iterator i;
 	for (i = Keys.begin(); i != Keys.end(); ++i)
 	{
        if ( (i->view == pPC->currentView) &&
             (i->Rect.contains(pts) ) &&
             i->enabled)
            return i->Description;
	}
	return("");
}

void Ckeyb::keyscan(void)
{
    int i, j;
	unsigned char ch;

//    for (i = 0; i < NM; i++)	{ keym[i] = 0; }
    memset(&keym,0,sizeof(keym));
		
    for (i = 0; i < NM; i++)
	{
		for (j = 0; j < 8; j++)
		{
			ch = scandef[i * 8 + j];

            if ( keyPressedList.contains(TOUPPER(ch)) ||
                 keyPressedList.contains(ch) ||
                 keyPressedList.contains(TOLOWER(ch)) ||
                 (LastKey == ch) ||
                 (TOUPPER(LastKey) == TOUPPER(ch)) )
			{
				keym[i] = (1 << j);
//				LastKey = 0;
				break;
			}	    
		}
		if (keym[i] != 0) break;
    }
}

CKey &Ckeyb::getKey(int code)
{
    for (int i=0;i<Keys.size();i++) {
        if (Keys.at(i).ScanCode == code) return Keys[i];
    }
}


BYTE Ckeyb::Read(BYTE data)
{
	BYTE	ret	=0;
	BYTE	jj	=0;

    if ((data == 0) && ((KStrobe) != 0))
	{
        jj = bit(KStrobe);
        if (jj < 8) { ret = keym[jj]; }
    }
    else 
	{
		jj = bit(data);
        if (jj < 8) { ret = keym[8 + jj]; }
    }
	return(ret);  
}

BYTE Ckeyb::Get_KS(void)
{
	return(KStrobe);
}

void Ckeyb::Set_KS(BYTE data)
{
//    if (pPC->pCPU->fp_log) fprintf(pPC->pCPU->fp_log,"KStrobe = [%02X]\n",data);

	KStrobe = data;
}

bool Ckeyb::CheckKon()
{
    Kon = false;
    if ( (LastKey == K_BRK) )
	{
        Kon = true;
		AddLog(2,"Kon TRUE");
        qWarning()<<"Kon true";
		LastKey = 0;
    }
	return (Kon);
}	

int Ckeyb::CheckOff()
{
	return(0);
}	

bool Ckeyb::init(void)
{
    isShift = false;
    isCtrl = false;
    LastKey = 0;
	QFile file( fn_KeyMap );
	QXmlInputSource source(&file);

	QXmlSimpleReader reader;
	reader.setContentHandler( handler );

	bool result = reader.parse( source );

	if (result) return true;

    // Else load from ressource
    QFile fileRes(":/KEYMAP/keymap/"+fn_KeyMap);
    QXmlInputSource sourceRes(&fileRes);
    result = reader.parse(sourceRes);
//    if (result) qWarning("success read key ressource\n");
    if (result) return true;

	// else load the struct
	for (int i = 0;i < pPC->KeyMapLenght;i++)
	{
		Keys.append(CKey(pPC->KeyMap[i].ScanCode,
						pPC->KeyMap[i].KeyDescription,
						QRect(	pPC->KeyMap[i].x1,
								pPC->KeyMap[i].y1,
								RIGHT[ pPC->KeyMap[i].Type - 1 ],
								BOTTOM[pPC->KeyMap[i].Type - 1 ])
						)
					);
	}
	return true;
}

bool Ckeyb::exit(void)
{
#ifdef EMSCRIPTEN
    return true;
#endif

	if (! modified) return true;
		
     int ret = QMessageBox::warning(mainwindow, QObject::tr("PockEmul"),
                   QObject::tr("The keyboard layout has been modified.\n"
                      "Do you want to save your changes?"),
                   QMessageBox::Save | QMessageBox::Discard,
                   QMessageBox::Save);
                   
    if (ret == QMessageBox::Discard) return true;

	// save the XML definition file
    QFile file(fn_KeyMap);

    file.open( QIODevice::WriteOnly);
    XmlWriter xw( &file );
    xw.setAutoNewLine( true );
    xw.writeRaw( "<!DOCTYPE KEYBMAP><KEYBMAP version=\"1.0\">" );
    xw.newLine();
    xw.writeTaggedString( "model", QString(pPC->getcfgfname()) );
    xw.writeOpenTag( "Keyboard" );
	
	QList<CKey>::iterator j;

	for (j = Keys.begin(); j != Keys.end(); ++j)
	{
    	AttrMap attrs;
    	attrs.insert( "description", j->Description.trimmed () );
        attrs.insert( "scancode", QString("0x%1").arg(j->ScanCode,2,16,QChar('0')) );
        attrs.insert( "masterscancode", QString("0x%1").arg(j->MasterScanCode,2,16,QChar('0')) );
    	attrs.insert( "left", QString("%1").arg(j->Rect.left()) );
    	attrs.insert( "top", QString("%1").arg(j->Rect.top() ));
    	attrs.insert( "width", QString("%1").arg(j->Rect.width() ) );
    	attrs.insert( "height", QString("%1").arg(j->Rect.height() ) );
        attrs.insert( "modifier", j->Modifier );
        QString _view;
        switch (j->view) {
        case FRONTview: _view = "FRONT"; break;
        case TOPview: _view = "TOP"; break;
        case LEFTview: _view = "LEFT"; break;
        case RIGHTview: _view = "RIGHT"; break;
        case BACKview: _view = "BACK"; break;
        case BOTTOMview: _view = "BOTTOM"; break;
        }
        attrs.insert( "view",_view);
    	xw.writeAtomTag( "KEY", attrs );
    }

    xw.writeCloseTag( "Keyboard" );
    xw.writeRaw( "</KEYBMAP>" );

	return true;
}

bool KEYBMAPParser::startDocument()
{
	inKeyboard = false;
	return true;
}

bool KEYBMAPParser::endElement( const QString&, const QString&, const QString &name )
{
	if( name == "Keyboard" )
		inKeyboard = false;

   return true;
}

bool KEYBMAPParser::startElement( const QString&, const QString&, const QString &name, const QXmlAttributes &attrs )
{
	QString desc = "";
    QString modifier="";
    int scancode,masterscancode,x,y,w,h;
    View view=FRONTview;
	bool ok = false;
	
    scancode=masterscancode=x=y=w=h=0;
	
	if( inKeyboard && name == "KEY" )
	{
		
		for( int i=0; i<attrs.count(); i++ )
		{
			if( attrs.localName( i ) == "description" )
				desc = attrs.value( i );
			else if( attrs.localName( i ) == "scancode" )
				scancode = attrs.value( i ).toInt(&ok,16);
			else if( attrs.localName( i ) == "left" )
				x = attrs.value( i ).toInt(&ok,10);
			else if( attrs.localName( i ) == "top" )
				y = attrs.value( i ).toInt(&ok,10);
			else if( attrs.localName( i ) == "width" )
				w = attrs.value( i ).toInt(&ok,10);
			else if( attrs.localName( i ) == "height" )
				h = attrs.value( i ).toInt(&ok,10);
            else if( attrs.localName( i ) == "masterscancode" )
                masterscancode = attrs.value( i ).toInt(&ok,16);
            else if( attrs.localName( i ) == "modifier" )
                modifier = attrs.value( i );
            else if( attrs.localName( i ) == "view" ) {
                qWarning()<<"view="<<attrs.value( i );
                if (attrs.value( i ) == "FRONT") view = FRONTview;
                if (attrs.value( i ) == "TOP") view = TOPview;
                if (attrs.value( i ) == "LEFT") view = LEFTview;
                if (attrs.value( i ) == "RIGHT") view = RIGHTview;
                if (attrs.value( i ) == "BACK") view = BACKview;
                if (attrs.value( i ) == "BOTTOM") view = BOTTOMview;
            }
		}
        Parent->Keys.append(CKey(scancode,desc,QRect(x,y,w,h),masterscancode,modifier,view));
        AddLog(LOG_KEYBOARD,mainwindow->tr("XML Read key : %1, scan=0x%2 , Rect=(%3,%4,%5,%6), mscan=0x%7, mod=%8").
               arg(desc).
               arg(scancode,2,16,QChar('0')).
               arg(x).arg(y).arg(w).arg(h).
               arg(masterscancode,2,16,QChar('0')).
               arg(modifier));
	}
	else if( name == "Keyboard" )
		inKeyboard = true;

	return true;
}
