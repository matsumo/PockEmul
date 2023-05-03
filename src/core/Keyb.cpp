#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QSound>

#include "xmlwriter.h"
 
#include "common.h"
#include "init.h"
#include "pcxxxx.h"
#include "cpu.h"
#include "Log.h"
#include "Inter.h"
#include "Keyb.h"

extern QString workDir;
extern bool kbSoundEnabled;

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

CKey::CKey(int scancode, QString description,QRect rect,int masterscancode,QString modifier,View view, int delay)
{
    this->Description = description;
    this->ScanCode	= scancode;
    this->MasterScanCode = masterscancode;
    this->Modifier = modifier;
    this->Rect = rect;
    this->view = view;
    this->enabled = true;
    this->delay = delay;
}

Ckeyb::Ckeyb(CViewObject *parent, QString map, BYTE *scan) //: CPObject(parent)								//[constructor]
{
    pPC = (CpcXXXX *)parent;
    Parent	= parent;
    enabled = true;
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

void Ckeyb::setMap(QString _map,BYTE *scan) {
    fn_KeyMap = _map;
    scandef     = scan;
}
QString Ckeyb::getMap() {
    return fn_KeyMap;
}

QString Ckeyb::msgDelay(Ctimer *timer) {
    QString _msg="";

    keyPressing.lock();

    QMapIterator<int, quint64> i(keyPressedList);
    while (i.hasNext()) {
        i.next();
        // Check if this key is delayed
        CKey _key = getKey(TOUPPER(i.key()));
        int _delay = _key.delay;
        if ( _delay > 0) {
            // Check timing
            quint64 _stick = i.value();
            quint64 _elapsed = timer->msElapsed(_stick);
            //                qWarning()<<"delay"<<_delay<<"stick"<<_stick<<"elapsed"<<_elapsed;

            if (_elapsed <= ((quint64)_delay*1000)) {
                // Draw text
                if (!_msg.isEmpty()) _msg+= "\n";
                _msg += QString(_key.Description+" in %1s").arg(_delay - _elapsed/1000);
            }
        }
    }

    keyPressing.unlock();

    return _msg;
}



bool Ckeyb::isKeyPressed() {
    bool _res = false;
    keyPressing.lock();
    QMapIterator<int, quint64> i(keyPressedList);
    keyPressing.unlock();

    while (i.hasNext()) {
        i.next();
        if (isKey(i.key())) _res = true;
    }

    return _res;
}

int Ckeyb::keyPressedCount() {
    int _res = 0;
    keyPressing.lock();
    QMapIterator<int, quint64> i(keyPressedList);
    keyPressing.unlock();

    while (i.hasNext()) {
        i.next();
        if (isKey(i.key())) _res++;
    }
    return _res;
}

bool Ckeyb::isKeyPressed(int _key) {
    keyPressing.lock();
    bool _ret = keyPressedList.contains(TOUPPER(_key));

    keyPressing.unlock();

    return _ret;
}

bool Ckeyb::isKey(int _key,int _addedDelay) {

    keyPressing.lock();
    if ( keyPressedList.contains(TOUPPER(_key)) ) {
         // Check if this key is delayed
        int _delay = getKey(TOUPPER(_key)).delay *1000 + _addedDelay;
        if ( _delay > 0) {
            // Check timing
            quint64 _stick = keyPressedList[TOUPPER(_key)];
            if (pPC->pTIMER->msElapsed(_stick) >= _delay) {
                pPC->Refresh_Display = true;
                keyPressing.unlock();
                return true;
            }
            else {
                pPC->Refresh_Display = true;
                keyPressing.unlock();
                return false;
            }
        }
        else {
            keyPressing.unlock();
            return true;
        }
    }
    keyPressing.unlock();
    return false;
}

void Ckeyb::insertKey(int _key)
{
    keyPressing.lock();

    keyPressedList.insert(_key,pPC->pTIMER ? pPC->pTIMER->state:0);

    keyPressing.unlock();

    if (kbSoundEnabled) {
        QSound::play(":/core/key.wav");
    }
//    qWarning()<<"insert key into buffer"<<_key<<"   buffer size"<<keyPressedList.size();
    emit keyPressed(_key);
}

void Ckeyb::removeKey(int _key) {
    keyPressing.lock();

    keyPressedList.remove(_key);

    keyPressing.unlock();

//    qWarning()<<"remove key into buffer"<<_key<<"   buffer size"<<keyPressedList.size();

}

int Ckeyb::KeyClick(QPoint pts)
{
    if (!enabled) return 0;

//    qWarning()<<"keyclick:"<<pts;
    // calculate all distance betwwen pts and keys centers
    // Keep the nearest

    int nearestIndex = -1;
    int smallerDistance = 99999;
    for (int i=0;i<Keys.size();i++)
    {
        View _curView = pPC->currentView;
        if (_curView == BACKviewREV) _curView = BACKview;

        if (Keys.at(i).view != _curView) continue;
        if (!(Keys.at(i).enabled)) continue;
        QRect r = Keys.at(i).Rect;
            r.setCoords(r.x()*mainwindow->zoom,
                        r.y()*mainwindow->zoom,
                        (r.x()+r.width())*mainwindow->zoom,
                        (r.y()+r.height())*mainwindow->zoom);
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
//    qWarning()<<"smallerDist:"<<smallerDistance<<nearestIndex<<(30*mainwindow->zoom);
    if ((smallerDistance < (30*mainwindow->zoom)) && (nearestIndex>=0)) {
        if (!pPC->closed) {
//            qWarning()<<"OPEN-return key:"<<Keys.at(nearestIndex).ScanCode;
            return TOUPPER(Keys.at(nearestIndex).ScanCode);
        }
        else
        {
//            qWarning()<<"CLOSE";
            if (Keys.at(nearestIndex).ScanCode == K_CLOSE)
                return TOUPPER(Keys.at(nearestIndex).ScanCode);
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
// CHANGE THIS
            if ( isKey(ch) ||
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

void Ckeyb::drawPressed(QImage *_img)
{

    if (keyPressedList.isEmpty()) { return; }

    keyPressing.lock();
    QMapIterator<int, quint64> i(keyPressedList);
    keyPressing.unlock();

    while (i.hasNext()) {
        i.next();
        QRect _rect = getKey(i.key()).Rect;
        _rect.setCoords(_rect.x()*pPC->internalImageRatio,
                        _rect.y()*pPC->internalImageRatio,
                        (_rect.x()+_rect.width())*pPC->internalImageRatio,
                        (_rect.y()+_rect.height())*pPC->internalImageRatio);

        int _m = qMin(_rect.width(), _rect.height())*.25;
        _rect+= QMargins(_m,_m,_m,_m);
        int dim = qMin(_rect.width(), _rect.height());
        int magnifierSize = dim * 2;
        int radius = magnifierSize / 2;
        int ring = radius - 15;
        QSize box = QSize(magnifierSize, magnifierSize);

        QPixmap maskPixmap;
        maskPixmap = QPixmap(box);
        maskPixmap.fill(Qt::transparent);

        QRadialGradient g;
        g.setCenter(radius, radius);
        g.setFocalPoint(radius, radius);
        g.setRadius(radius);
        g.setColorAt(1.0, QColor(64, 64, 64, 0));
        g.setColorAt(0.5, QColor(0, 0, 0, 255));

        QPainter mask(&maskPixmap);
        mask.setRenderHint(QPainter::Antialiasing);
        mask.setCompositionMode(QPainter::CompositionMode_Source);
        mask.setBrush(g);
        mask.setPen(Qt::NoPen);
        mask.drawRect(maskPixmap.rect());
        mask.setBrush(QColor(Qt::transparent));
        mask.drawEllipse(g.center(), ring, ring);
        mask.end();

        QPoint center = _rect.center() - QPoint(0, radius);
        center = center + QPoint(0, radius / 2);
        QPoint corner = center - QPoint(radius, radius);

        QPoint xy = center * 2 - QPoint(radius, radius);

        // only set the dimension to the magnified portion
        QPixmap zoomPixmap = QPixmap(box);
        zoomPixmap.fill(Qt::lightGray);
        QPainter pz(&zoomPixmap);
//        pz.translate(-xy);
        QRect target=QRect(QPoint(0,0),box);
        pz.drawImage(target, *_img,_rect);
        pz.end();

        QPainterPath clipPath;
        clipPath.addEllipse(center, ring, ring);

        QPainter p(_img);
        p.setRenderHint(QPainter::Antialiasing);
        p.setClipPath(clipPath);
        p.drawPixmap(corner, zoomPixmap);
        p.setClipping(false);
        p.drawPixmap(corner, maskPixmap);
        p.setPen(Qt::gray);
        p.drawPath(clipPath);

        p.end();
    }
}

CKey &Ckeyb::getKey(int code)
{
    for (int i=0;i<Keys.size();i++) {
        if (TOUPPER(Keys.at(i).ScanCode) == TOUPPER(code)) return Keys[i];
    }

    CKey *_k = new CKey();
    return *_k;
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
    qWarning()<<"Ckeyb::init";
    isShift = false;
    isCtrl = false;
    LastKey = 0;

    if (fn_KeyMap.isEmpty()) return true;


    QFile file;
    file.setFileName(workDir+"res/"+pPC->getresName()+"/"+fn_KeyMap );

    if (!file.exists()) {
        file.setFileName(":/KEYMAP/keymap/"+fn_KeyMap);
    }

    if (file.exists()) {

        QXmlInputSource source(&file);
        QXmlSimpleReader reader;
        reader.setContentHandler( handler );

        bool result = reader.parse( source );
        return result;
    }


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
    QFile file(workDir+"sessions/"+fn_KeyMap);

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
        case BACKview:
        case BACKviewREV: _view = "BACK"; break;
        case BOTTOMview: _view = "BOTTOM"; break;
        }
        attrs.insert( "view",_view);
        if (j->delay > 0) {
            attrs.insert( "delay", QString("%1").arg(j->delay) );
        }
        xw.writeAtomTag( "KEY", attrs );
    }

    xw.writeCloseTag( "Keyboard" );
    xw.writeRaw( "</KEYBMAP>" );

    file.close();

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
    int scancode,masterscancode,delay,x,y,w,h;
    View view=FRONTview;
	bool ok = false;
	
    scancode=masterscancode=delay=x=y=w=h=0;
	
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
            else if( attrs.localName( i ) == "delay" ) {
                delay = attrs.value( i ).toInt(&ok,10);
                qWarning()<<"delay="<<delay;
            }
            else if( attrs.localName( i ) == "modifier" )
                modifier = attrs.value( i );
            else if( attrs.localName( i ) == "view" ) {
                if (attrs.value( i ) == "FRONT") view = FRONTview;
                if (attrs.value( i ) == "TOP") view = TOPview;
                if (attrs.value( i ) == "LEFT") view = LEFTview;
                if (attrs.value( i ) == "RIGHT") view = RIGHTview;
                if (attrs.value( i ) == "BACK") view = BACKview;
                if (attrs.value( i ) == "BOTTOM") view = BOTTOMview;
            }
		}
        Parent->Keys.append(CKey(scancode,desc,QRect(x,y,w,h),masterscancode,modifier,view,delay));
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
