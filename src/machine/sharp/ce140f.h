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
class Cce1600f;

/*
Boot sector
At power-on time, the contents cf the boot secter are r-ead out ta the buffer and checked whether
they are e boot program.

FAT
The file allocation table (FAT) (s a map indicating which files are oecupying where on the disk.
Files on a disk are physicalIy managed in the units called cluster-s, and each cluster is managed by
one byte cf cluster information written in FAT.
The first byte cf FAT contains the disk far-mat ID code (F2H) and the second and the following
bytes contain the mapping informatien cf the data area. The mapping information consists et 122
bytes cf data, e~chcf which represents information cf a particular cluster- and they are written in
the order of from cluster 1 te cluster 122 (the clusters 1 te 122 respectively correspond ta logical
secters 6 te 127.) Each cluster information has the fellawing meanings:
00H: This means that the cluster is not used.
01H ta 7AH: This means that the cluster (s used and the value designates the cluster number cf
the cluster that should corne after the cur-rently concer-ned cluster.
FOH: This means that the cluster (s the last cluster cf the file.

Directory
The dir-ectory contains information about eech file on the disk and uses 32 bytes et ar-ea per- file.
The directory accupies 3 secters cf area on the disk, and since one sector eonsists cf 512 bytes, 48
files (512 x 3/32) can exist on each side cf the disk.
Each file infermation (32 bytes) consists cf the feilewing items:
* The location et each item such as 08H or OCH is the offset from 00H.
(1) File name (00H te 07H)
Stores the file narne. If the file name is Iess than 8 char-acter-s, the space (s fiiled with space
char-acter-s (code 20H).
(2) Extension (08H ta OAH)
Stores the extension. if no extension (s given er if the extension is less than 3 character-s, the
space is filled with space characters (code 20H).
(3) Attr-ibute (OBH)
Stores the attributes cf the file. The bits of this byte have the foliowing rneen(ngs:
Bit O: O = Read/write 1 = Read oniy
Bits 1 te 7: Reserved
(4) Reserved area (OCH to 15H)
Always filied with 00H.
(5) Update time (16H and 17H)
Star-es the time when the file is created or updated.

Update date (18H and 19H)
Stores the date when the file is cr-eated or updated.

First cluster number- (1AH and 18H)
Stores the cluster number of the tirst cluster used for the file. 1AH stores this cluster number
and 18H always stores 00H.
(8) File size (1CH ta 1FH)
Stores the size cf the file in bytes. The size date are written from 1CH in the order 0f low byte
and high byte.
*/
class Cce1650f {
public:
    int filePtr[8];
    quint8 boot[512];
    quint8 fat[512];
    quint8 fatBackup[512];
    quint8 directory[3][512];
    quint8 data[122][512];
};


class Cce140f:public CPObject{
    Q_OBJECT
public:

    virtual bool init(void);				//initialize
    virtual bool exit(void);				//end
    virtual	bool run(void);					//check access

    virtual bool	Set_Connector(Cbus *_bus = 0);
    virtual bool	Get_Connector(Cbus *_bus = 0);
    virtual bool UpdateFinalImage(void);
    virtual void ComputeKey(KEYEVENT ke = KEY_PRESSED,int scancode=0,QMouseEvent *event=0);


    Cconnector	*pCONNECTOR;		qint64 pCONNECTOR_value;
    Cconnector	*pCONNECTOR_Ext;	qint64 pCONNECTOR_Ext_value;

    Cce1600f *pce1600f;

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
