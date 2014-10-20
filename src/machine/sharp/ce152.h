#ifndef _CE152_H_
#define _CE152_H_

#include "pobject.h"
#include "Connect.h"
#include "Inter.h"
#include "Keyb.h"


#define BIT_0           8
#define BIT_1           16
#define BIT_MID         ((BIT_0 + BIT_1) / 2)


#define ERR_OK	0
#define ERR_NOK	-1


typedef unsigned char    u_char;    //  8 bits
typedef unsigned short   u_short;   // 16 bits
typedef unsigned long    u_long;    // 32 bits

enum TAPEmode {PLAY,STOP,LOAD,EJECT,RECORD};

class Cce152:public CPObject{

Q_OBJECT

public:
	typedef struct {
    FILE*   ptrFd ;
    u_long  ident ;
    u_long  freq ;			// frequency
    u_long  bitLen ;
    u_long  count ;
    u_long  sum ;
    u_long  type ;
    u_long  graph ;
    u_long  debug ;
} WavFileInfo ;

typedef struct {
    FILE*   ptrFd ;
    u_long  ident ;
    u_long  count ;
    u_long  sum ;
} FileInfo ;

	BYTE	Tapein;					//Tape loaded ? (0:no,other:yes)

    virtual bool	init(void);				//initialize
    virtual bool	exit(void);				//end
    virtual bool	run(void);
    virtual bool UpdateFinalImage(void);
	bool	GetWav(void);
	bool	SetWav(bool);



    TAPEmode mode;
	bool	paused;
	
	int		getTapeCounter(void);

	bool	SoundOn;

    WavFileInfo	info ;

	QString		TapeFileName;
	Cconnector	*pTAPECONNECTOR;
	qint64		pTAPECONNECTOR_value;
    virtual void ComputeKey(KEYEVENT ke = KEY_PRESSED,int scancode=0);
	
    Cce152(CPObject *parent = 0);
    ~Cce152() {
        StopPlay();
    }

protected slots:
	int		Play(void);
	int		StopPlay(void);
	int		LoadTape(void);
	int		EjectTape(void);
	int		RecTape(void);

protected:
    QImage pignon;
	int		myfgetc(WavFileInfo* ptrFile);
	long	TapeCounter;
	int		carac;
	char	*fname_wav;
	int		med_pt;
	int		low_pt;
    quint64	first_state;
    quint64	counter;
    quint64 counterDiv100;
	int		GetWav_Val;

	int		UpdateHeadToWav (u_long    nbSamp,WavFileInfo* ptrFile);

	int		WriteHeadToWav (u_long    nbSamp,u_long    freq,WavFileInfo* ptrFile);
	int		WriteShortToFile (u_long value,FILE** ptrFd);
	int		WriteLongToFile (u_long value,FILE** ptrFd);
	int		WriteStringToFile (char*  ptrStr,FILE** ptrFd);
	int		myfseek(WavFileInfo* ptrFile , long offset , int origin );

	int		ReadWavHead (WavFileInfo* ptrFile);
	int		ReadStringFromFile (char*  ptrStr,long   nb,FILE** ptrFd);
	int		ReadLongFromFile (u_long* ptrVal,FILE**  ptrFd);
	int		ReadShortFromFile (u_short* ptrVal,FILE**   ptrFd);

	void	CvStringIToShort (char*    ptrStr,u_short* ptrVal);
	void	CvStringIToLong (char*   ptrStr,u_long* ptrVal);
	void	CvShortToStringI (u_short value,char*   ptrStr);
    void	CvLongToStringI (u_long value,unsigned char*  ptrStr);

    qint64  previous_state_setwav;
};

class Cce127r:public Cce152{
Q_OBJECT

public:
    Cce127r(CPObject *parent = 0);
    virtual bool	init(void);
};

class Cpc2081:public Cce152{
Q_OBJECT

public:
    Cpc2081(CPObject *parent = 0);
    virtual bool	init(void);
};


#endif
