#ifndef POBJECTINTERFACE_H
#define POBJECTINTERFACE_H


#include <QWidget>
#include <QPoint>
#include <QImage>
#include <QMenu>
#include <QFile>
#include <QMutex>

#ifndef NO_SOUND
#include <QAudioOutput>
#endif

#include <QXmlStreamWriter>
#include <QPixmap>

#include "common.h"

class CpcXXXX;
class Ckeyb;
class Ctimer;
class Clcdc;
class Cconnector;
class CExtensionArray;
class DialogKeyList;
class DialogDump;
class DialogDasm;
class DialogVKeyboard;
class QSwipeGesture;
class Cbus;

#include "slot.h"
#include "viewobject.h"


#define PS_OFF	0
#define PS_RUN	1
#define PS_RSV	2
#define PS_PRO	3



class CPObjectInterface
{

public:
    enum KEYEVENT{ KEY_PRESSED,KEY_RELEASED};

    virtual ~CPObjectInterface() {}

    virtual void    Reset() =0;
    virtual	bool	init() = 0;			// initialize
    virtual	bool	run(void)=0;					// emulator main step
    virtual bool	exit()=0;					// exit
    virtual bool	InitDisplay(void);
    virtual bool	UpdateFinalImage(void) =0;
    virtual bool	Set_Connector(Cbus *_bus = 0) =0;
    virtual bool	Get_Connector(Cbus *_bus = 0) = 0;
    virtual void    TurnON() =0;
    virtual void    TurnOFF() =0;
    virtual bool    SaveSession_File(QXmlStreamWriter *xmlOut) =0;
    virtual bool	LoadSession_File(QXmlStreamReader *xmlIn) =0;
    virtual void    ComputeKey(KEYEVENT ke = KEY_PRESSED,int scancode=0) = 0;
    virtual void    BuildContextMenu(QMenu * menu) =0;
    virtual void    TurnCLOSE(void) =0;
    virtual int     mapKey(QKeyEvent *event) =0;


};

#define CPObjectInterface_iid "org.qt-project.Qt.Examples.CPObjectInterface"

Q_DECLARE_INTERFACE(CPObjectInterface, CPObjectInterface_iid)

#endif // POBJECTINTERFACE_H
