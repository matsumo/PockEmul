#ifndef CE140F_H
#define CE140F_H

/********************************************************************************************************
 * PROGRAM      : test
 * DATE - TIME  : samedi 28 octobre 2006 - 12h40
 * AUTHOR       :  (  )
 * FILENAME     : Ce126.h
 * LICENSE      : GPL
 * COMMENTARY   : printer(CE-126P) emulation class
 ********************************************************************************************************/

#include <QTime>
#ifndef NO_SOUND
#include <QSound>
#endif
#include <QDir>


#include "pobject.h"
//#include "Keyb.h"
//#include "Connect.h"
//#include "Inter.h"


class Cconnector;

class Cce140f:public CPObject{
    Q_OBJECT
public:

    virtual bool init(void);				//initialize
    virtual bool exit(void);				//end
    virtual	bool run(void);					//check access

    virtual bool	Set_Connector(void);
    virtual bool	Get_Connector(void);
    virtual bool UpdateFinalImage(void);
    virtual void ComputeKey(KEYEVENT ke = KEY_PRESSED,int scancode=0);


    Cconnector	*pCONNECTOR;		qint64 pCONNECTOR_value;
    Cconnector	*pCONNECTOR_Ext;	qint64 pCONNECTOR_Ext_value;

    int ce140f_Mode;

    void Push8(BYTE b);
    void Push4(BYTE b);
    BYTE Pop_out8(void);
    BYTE Pop_out4(void);

    void processCommand(void);

    void process_DSKF(void);
    void process_FILES(void);
    void process_INIT(int cmd);
    void process_FILES_LIST(int cmd);
    void process_SAVE(int cmd);
    void process_LOAD(int cmd);
    void process_CLOSE(int cmd);
    void process_KILL(int cmd);

    void sendString(QString s);
    BYTE CheckSum(BYTE b);
    QString cleanFileName(QString s);

    Cce140f(CPObject *parent = 0);

    virtual ~Cce140f();

public:

protected slots:
    void contextMenuEvent ( QContextMenuEvent * );
    void definePath(void);

private:
    QDir    directory;

    void	pulldownsignal(void);

    QByteArray	TextBuffer;
    bool	Previous_PIN_BUSY;
    bool	Previous_PIN_MT_OUT1;
    bool	Previous_PIN_D_OUT;
   // QTime	time;
    bool	ctrl_char;
    BYTE	t,c;
    qint64	run_oldstate;
    int     code_transfer_step;
    int     device_code;
    QList<BYTE>   data;
    QList<BYTE> data_out;
    bool    halfdata;
    bool    halfdata_out;
    int     checksum;
    int     wait_data_function;
    qint64     file_size;

    QFile   file_save;
    QFile   file_load;
    QByteArray ba_load;
    quint64  lastState;
    int     fileCount;
    QStringList fileList;
    quint64  lastRunState;
    bool    busyLed;

    bool MT_OUT2;
    bool BUSY;
    bool D_OUT;
    bool MT_IN;
    bool MT_OUT1;
    bool D_IN;
    bool ACK;
    bool SEL2;
    bool SEL1;
};


#endif // CE140F_H
