#include <QFileDialog>
#include <QTime>
#include <QPainter>
#include <QDebug>

#include "common.h"
#include "pcxxxx.h"
#include "Log.h"
#include "dialoganalog.h"
#include "sc61860.h"
#include "ce152.h"
#include "Keyb.h"
#include "keybce152.h"

#define sample_freq		44100
#define rec_speed		1200
#define BASE_FREQ1      4000
#define BASE_FREQ2      2500
#define TRANS_HIGH      0x80
#define TRANS_LOW       0x7F
#define ORDER_STD       0
#define ORDER_INV       1
#define SYNC_NB_TRY     50
#define IDENT_UNKNOWN   0x00
#define IDENT_OLD_BAS   0x20
#define IDENT_OLD_PAS   0x21
#define IDENT_OLD_BIN   0x26
#define IDENT_NEW_BAS   0x70
#define IDENT_NEW_PAS   0x71
#define IDENT_EXT_BAS   0x72
#define IDENT_EXT_PAS   0x73
#define IDENT_NEW_BIN   0x76
#define IDENT_PC1211    0x80
#define IDENT_PC1500    0x0A
#define IDENT_PC15_BIN  0xA0
#define IDENT_PC15_BAS  0xA1

static char* bitwav[] = {
    "\255\255\0\0\255\255\0\0\255\255\0\0\255\255\0\0",
    "\255\0\255\0\255\0\255\0\255\0\255\0\255\0\255\0"} ;

char *ftyp[] = {"BASIC (prg)","BASIC (dat)","binaire","assembleur"};
FILE	*fp_tape=NULL;

//extern u_long testReadQuarterTape(void);


/*
	RIFF Chunk (12 bytes in length total) Byte Number
		0 - 3	"RIFF" (ASCII Characters)
		4 - 7	Total Length Of Package To Follow (Binary, little endian)
		8 - 12	"WAVE" (ASCII Characters)

	FORMAT Chunk (24 bytes in length total) Byte Number
		0 - 3	"fmt_" (ASCII Characters)
		4 - 7	Length Of FORMAT Chunk (Binary, always 0x10)
		8 - 9	Always 0x01
		10 - 11	Channel Numbers (Always 0x01=Mono, 0x02=Stereo)
		12 - 15	Sample Rate (Binary, in Hz)
		16 - 19	Bytes Per Second
		20 - 21	Bytes Per Sample: 1=8 bit Mono, 2=8 bit Stereo or 16 bit Mono, 4=16 bit Stereo
		22 - 23	Bits Per Sample

  DATA Chunk Byte Number
		0 - 3	"data" (ASCII Characters)
		4 - 7	Length Of Data To Follow
		8 - end	Data (Samples)
*/


Cce152::Cce152(CPObject *parent)	: CPObject(parent)
{
   // if (parent == 0) pPC = (CPObject *)this;
    BackGroundFname	= P_RES(":/ext/ce-152.png");
    pignon = QImage(P_RES(":/ext/ce-152-wheel.png"));

    setcfgfname("ce152");
    Tapein		= 0;				//Tape loaded (0:none, other:access)
    TapeCounter	= 0;
    mode		= EJECT;
    SoundOn		= false;
    info.ptrFd	= 0;
    pTIMER		= new Ctimer(this);
    setDX(393);
    setDY(600);
    setDXmm(186);
    setDYmm(115);
    setDZmm(52);
    KeyMap		= KeyMapce152;
    KeyMapLenght= KeyMapce152Lenght;
    pKEYB		= new Ckeyb(this,"ce152.map");
    first_state = 0;
    counter		= 0;
    counterDiv100 = 0;
    GetWav_Val	= 0;
    previous_state_setwav = 0;
}
Cce127r::Cce127r(CPObject *parent) : Cce152(parent)
{
    BackGroundFname	= P_RES(":/ext/ce-127r.png");
    setcfgfname("ce127r");
    setDXmm(132);//Pc_DX_mm = 135;
    setDYmm(74);//Pc_DY_mm = 70;
    setDZmm(25);//Pc_DZ_mm = 10;
    setDX(472);//Pc_DX		= 483;//409;
    setDY(266);//Pc_DY		= 252;//213;
    pKEYB->fn_KeyMap = "ce127r.map";

    //touche play 460,293 : 60x70

}

Cpc2081::Cpc2081(CPObject *parent) : Cce152(parent)
{
    BackGroundFname	= P_RES(":/ext/pc-2081.png");
    setcfgfname("pc2081");
    setDXmm(184);
    setDYmm(114);
    setDZmm(31);
    setDX(658);
    setDY(410);
    pKEYB->fn_KeyMap = "pc2081.map";


}

/*********************************/
/* Initialize Tape				 */
/*********************************/

bool Cpc2081::init(void) {
    Cce152::init();
    pTAPECONNECTOR->setSnap(QPoint(0,300));
    pTAPECONNECTOR->setDir(Cconnector::WEST);
    return true;
}

bool Cce127r::init(void) {
    Cce152::init();

    pTAPECONNECTOR->setSnap(QPoint(472,130));
    pTAPECONNECTOR->setDir(Cconnector::EAST);
    return true;
}

bool Cce152::init(void)
{
    AddLog(LOG_TAPE,"Tape initializing...")

    info.ptrFd	= 0;
    mode	= EJECT;

    CPObject::init();

    setfrequency( 0);

    pTAPECONNECTOR	= new Cconnector(this,3,0,Cconnector::Jack,"Line in / Rec / Rmt",true,
                                     QPoint(0,150),Cconnector::WEST);	publish(pTAPECONNECTOR);

    WatchPoint.add(&pTAPECONNECTOR_value,64,2,this,"Line In / Rec");

    if(pKEYB)	pKEYB->init();
    if(pTIMER)	pTIMER->init();

//if (fp_tape==NULL) fp_tape=fopen("LOG TAPE.txt","wb");
    return true;
}

/*****************************************************/
/* Exit TAPE										 */
/*****************************************************/
bool Cce152::exit(void)
{
    StopPlay();
	EjectTape();
    CPObject::exit();
	return true;
}

void Cce152::ComputeKey(void)
{
    BYTE k = pKEYB->LastKey;
    switch (k) {
    case K_PLAY : if (mode == LOAD) { Play();
        }
                    break;
    case K_EJECT:
        switch (mode) {
            case EJECT:
            case STOP : LoadTape(); break;
            case LOAD : LoadTape(); break;
            case PLAY : StopPlay(); break;
            case RECORD : StopPlay(); break;
		}
        break;
    case K_RECORD: RecTape(); break;
    }
}
bool Cce152::UpdateFinalImage(void) {
    CPObject::UpdateFinalImage();

    QPainter painter;


    if (mode == RECORD) {
        painter.begin(FinalImage);
        QRect recordRect = pKEYB->getKey(0xD4).Rect;
        painter.drawImage(recordRect,QImage(P_RES(":/ext/ce-152-key.png")));
        painter.drawImage(67,304,QImage(P_RES(":/ext/ce-152-tape.png")));
        painter.end();
    }
    if ( (mode == PLAY) || (mode == RECORD) ){
        painter.begin(FinalImage);
        QRect recordRect = pKEYB->getKey(0xD1).Rect;
        painter.drawImage(recordRect,QImage(P_RES(":/ext/ce-152-key.png")));
        painter.drawImage(67,304,QImage(P_RES(":/ext/ce-152-tape.png")));
        painter.end();

    }
    if (mode == LOAD) {
        painter.begin(FinalImage);
        painter.drawImage(67,304,QImage(P_RES(":/ext/ce-152-tape.png")));
        painter.end();

    }

    if (TapeCounter !=0) {


        painter.begin(FinalImage);
        painter.translate(67+189+19,304+29+19);
        painter.rotate(-TapeCounter/100);
        painter.drawImage(-19,-19,pignon);
        painter.end();

        painter.begin(FinalImage);
        painter.translate(67+50+19,304+29+19);
        painter.rotate(-TapeCounter/300);
        painter.drawImage(-19,-19,pignon);
        painter.end();

//        qWarning()<<counter/100;
    }
    // PRINTER SWITCH

//    painter.drawImage(282,235,BackgroundImageBackup->copy(282,235,30,20).mirrored(rmtSwitch,false));


    Refresh_Display = true;

    return true;
}

bool Cce152::run(void)
{
    bool rmt = pTAPECONNECTOR->Get_pin(3);

    if (rmt) {
        // Compute input pin
        SetWav(pTAPECONNECTOR->Get_pin(2));

        // Compute output pin
        pTAPECONNECTOR->Set_pin(1,GetWav());
    }

	pTAPECONNECTOR_value = pTAPECONNECTOR->Get_values();
	return true;
}

bool Cce152::GetWav(void)
{
	if (mode != PLAY) return false;			// Return 0 If not PLAY mode

    if (first_state == 0) {
        counter = 1;
        counterDiv100 = 0;
        first_state = pTIMER->state;
    }

	// Calculate nb of byte to skip corresponding to the CPU frequency
    quint64 wait =  (pTIMER->pPC->getfrequency() / info.freq);
    //qint64 delta = (pTIMER->state - first_state);
    while ((pTIMER->state - first_state) >= wait) {
		GetWav_Val = myfgetc(&info);
        if (fp_tape) fprintf(fp_tape,
                "delta=%lld val=%s c=%i c=%lld\n",
                (pTIMER->state - first_state),
                (GetWav_Val>0x10)?"1":"0",GetWav_Val,counter);
		counter++;
        if (counterDiv100 < (counter/100)) {
            counterDiv100 = counter/100;
            update();
        }
        first_state +=wait;
	}

	return ((GetWav_Val>0x10)?true:false);
}

int Cce152::myfgetc(WavFileInfo* ptrFile)
{
//	fprintf(fp_tape,"Read byte. - ");
	if (info.ptrFd)
	{
		int c=fgetc(ptrFile->ptrFd);
		if (c!=EOF)
		{
			TapeCounter++;
			return c;
		}
		else
			return 0;
	}
	else
		return 0;
}

bool Cce152::SetWav(bool bit)
{
//    if (fp_tape==NULL) fp_tape=fopen("LOG TAPE.txt","wb");
//fprintf(fp_tape,"setwav - mode=%d",mode);
	if (mode != RECORD) return false;			// Return 0 If not PLAY mode
//fprintf(fp_tape,"RECORD - ");

	if (first_state == 0) 
		{
            counter = 1;
            counterDiv100 = 0;
			first_state = pTIMER->state;
		}

	// Calculate nb of byte to skip corresponding to the CPU frequency
    quint64 wait = ( pTIMER->pPC->getfrequency()) / info.freq;
    quint64 delta = (pTIMER->state - first_state);

//    if (pTIMER->pPC->pCPU) fprintf(fp_tape," Xout=%d - ",pTIMER->pPC->pCPU->Get_Xout());

    while ((pTIMER->state - first_state) >= wait)
	{
        if (fp_tape) fprintf(fp_tape,"state=%lld diff=%lld delta=%lld val=%s c=%lld\n",pTIMER->state,pTIMER->state-previous_state_setwav,delta,bit?"1":"0",counter);
        previous_state_setwav = pTIMER->state;
        int error = fputc ( (bit?0xFF:0x00), info.ptrFd) ;
        TapeCounter++;
        counter++;
        if (counterDiv100 < (counter/100)) {
            counterDiv100 = counter/100;
            Refresh_Display=true;
            update();
        }
        first_state +=wait;
	}

	return (true);
}

int	Cce152::Play(void)
{
	mode = PLAY;
	return 1;
}

int	Cce152::StopPlay(void)
{
	if (mode == RECORD)
	{
		UpdateHeadToWav (0,&info);
        fflush(info.ptrFd);
	}
    // SEEK to the begining of the file
     myfseek (&info, 0, SEEK_SET);
    mode = LOAD;
	return 1;
}

int	Cce152::EjectTape(void)
{
	if (info.ptrFd)
	{
        StopPlay();
		AddLog(LOG_TAPE,"Stop Play");
		fclose(info.ptrFd);
		info.ptrFd=0;
		TapeCounter = 0;
	}
	mode = EJECT;
	return 1;
}

int Cce152::LoadTape(void)
{
    int		error;
    long	nbByte ;


    // Display the Open dialog box.

    QString ofn = QFileDialog::getOpenFileName(
                    mainwindow,
                    "Choose a file",
                    ".",
                    "Wav Files (*.wav)");


    if (ofn.isEmpty())
    {
        mode = EJECT;
        return 0;
    }
    QFile file(ofn);

    char * str = qstrdup(ofn.toLocal8Bit());

    if ((info.ptrFd = fopen(str,"rb"))==NULL)
//	if (!file.open(QIODevice::ReadOnly))
    {
        MSG_ERROR(tr("Failed to open file"));
        return 0;
    }

    TapeFileName = ofn;

//	info.ptrFd = (FILE *) file.handle();

    /* Seek to the end of the source file */
    error = fseek (info.ptrFd, 0, SEEK_END) ;
    if (error != ERR_OK) {
        MSG_ERROR(tr("ERROR: Can't seek the file")) ;
        return 0;
    }

    /* Get the length of the source file */
    nbByte = ftell (info.ptrFd) ;
    if (nbByte == ERR_NOK) {
        MSG_ERROR(tr("ERROR: Can't ftell the file")) ;
        error = ERR_NOK ;
        return 0;
    }

    /* Seek to the begining of the source file */
    error = myfseek (&info, 0, SEEK_SET) ;
    if (error != ERR_OK) {
        MSG_ERROR(tr("ERROR: Can't seek the file")) ;
        return 0;
    }

    if (nbByte <= 58) {
        MSG_ERROR(tr("Source file is not a valid WAV File"));
        error = ERR_NOK ;
        return 0;
    }

    /* Read the header of the source WAV file */
    error = ReadWavHead (&info) ;
    if (error != ERR_OK) return 0;

    /* Check the lower frequency limit */
    if (info.freq < 5000) {
        AddLog(LOG_TAPE,tr("ERROR: Sampling frequency is lower than 5000 Hz : %1 Hz").arg(info.freq));
        MSG_ERROR(tr("Sampling frequency is lower than 5000 Hz"));
        error = ERR_NOK ;
        return 0;
    }

    /* Check the higher frequency limit */
    if (info.freq > 22050) {
        AddLog(LOG_TAPE,tr("ERROR: Sampling frequency is greater than 22050 Hz : %1 Hz").arg(info.freq));
        MSG_ERROR(tr("Sampling frequency is greater than 22050 Hz : %1 Hz").arg(info.freq));
        error = ERR_NOK ;
        return 0;
    }
//	MSG_ERROR(tr("Sampling frequency is  %1 Hz").arg(info.freq));

//	info.freq = (u_long) pPC->Tape_Base_Freq ;
    info.bitLen = (info.freq  * 8) / pTIMER->pPC->Tape_Base_Freq ;

    med_pt	= (info.freq/rec_speed)*0.45;
    low_pt	= (info.freq/rec_speed)*0.25;


    first_state = 0;
    mode = LOAD;
    return (1);
}

int Cce152::RecTape(void)
{

    // Display the Save dialog box.
    TapeFileName = QFileDialog::getSaveFileName(
                    mainwindow,
                    "Choose a filename to save under",
                    ".",
                    "Wav Files (*.wav)");
    QFile file(TapeFileName);

    char * str = qstrdup(TapeFileName.toLocal8Bit());

    //if (!file.open(QIODevice::WriteOnly))
    if ((info.ptrFd = fopen(str,"wb"))==NULL)
    {
        AddLog(LOG_TAPE,tr("ERROR: Can't create the wav file."));
        MSG_ERROR(tr("Failed to create file"));
        return 0;
    }

    // create th header
    WriteHeadToWav (10,
                    16000,//pTIMER->pPC->Tape_Base_Freq*2,
                    &info);
    info.bitLen = 0x20;
    info.freq = 16000;//pTIMER->pPC->Tape_Base_Freq*2;
//       ptrFile->bitLen = (freq * 8) / ptrFile->freq ;


    mode = RECORD;

    return (1);
}


int Cce152::getTapeCounter(void)
{
    return TapeCounter;
}

int Cce152::myfseek( WavFileInfo* ptrFile , long offset , int origin )
{

    switch (origin)
    {
    case SEEK_SET: TapeCounter=offset; break;
    case SEEK_CUR: TapeCounter+=offset;break;
    }

    return fseek (ptrFile->ptrFd, offset, origin);
}

void Cce152::CvStringIToShort (char* ptrStr,u_short* ptrVal)
{
    /* Convert the String to a short value with msb first (INTEL) */
    *ptrVal =  (u_short) ptrStr[0] & 0xFF ;
    *ptrVal += ((u_short) ptrStr[1] & 0xFF) << 8 ;
}

void Cce152::CvStringIToLong (char*   ptrStr,u_long* ptrVal)
{

    /* Convert the String to a long value with msb first (INTEL) */
    *ptrVal =  (u_short) ptrStr[0] & 0xFF ;
    *ptrVal += ((u_short) ptrStr[1] & 0xFF) << 8 ;
    *ptrVal += ((u_short) ptrStr[2] & 0xFF) << 16 ;
    *ptrVal += ((u_short) ptrStr[3] & 0xFF) << 24 ;
}

int Cce152::ReadStringFromFile (char*  ptrStr,long   nb,FILE** ptrFd)
{
    int         ii ;
    int         inVal ;
    int         error ;

    error = ERR_OK ;

    for ( ii = 0 ; ii < nb ; ii++  ) {
        inVal = fgetc (*ptrFd) ;
        if (inVal == EOF) {
            printf ("ERROR : Can't read over End Of File\n") ;
            error = ERR_NOK ;
            break ;
        }
        *ptrStr++ = (char) inVal ;
    }
    *ptrStr = 0 ;
    return (error);
}

int Cce152::ReadLongFromFile (u_long* ptrVal,FILE**  ptrFd)
{
    char        str[10] ;
    int         error ;

    error = ReadStringFromFile (str, 4, ptrFd) ;
    CvStringIToLong (str, ptrVal) ;

    return (error);
}

int Cce152::ReadShortFromFile (u_short* ptrVal,FILE**   ptrFd)
{
    char        str[10] ;
    int         error ;

    error = ReadStringFromFile (str, 2, ptrFd) ;
    CvStringIToShort (str, ptrVal) ;

    return (error);
}



int Cce152::ReadWavHead (WavFileInfo* ptrFile)
{
    char        str[20] ;
    u_long      tmpL ;
    u_short     tmpS ;
    int         error ;

    do {
        error = ReadStringFromFile (str, 4, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;
        if (strcmp (str, "RIFF") != 0) {
            AddLog(LOG_TAPE,"ERROR: Header isn't 'RIFF'") ;
            error = ERR_NOK ;
            break ;
        }

        error = ReadLongFromFile (&tmpL, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;

        error = ReadStringFromFile (str, 8, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;
        if (strcmp (str, "WAVEfmt ") != 0) {
            AddLog(LOG_TAPE,"ERROR: Header format isn't 'WAVEfmt '") ;
            error = ERR_NOK ;
            break ;
        }

        error = ReadLongFromFile (&tmpL, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;

        error = ReadShortFromFile (&tmpS, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;
        if (tmpS != 1) {
            AddLog(LOG_TAPE,"ERROR: Format isn't PCM") ;
            error = ERR_NOK ;
            break ;
        }

        error = ReadShortFromFile (&tmpS, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;
        if (tmpS != 1) {
            AddLog(LOG_TAPE,"ERROR: Format isn't Mono") ;
            error = ERR_NOK ;
            break ;
        }

        error = ReadLongFromFile (&ptrFile->freq, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;
        error = ReadLongFromFile (&tmpL, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;

        error = ReadShortFromFile (&tmpS, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;
        if (tmpS != 1) {
            AddLog(LOG_TAPE,"ERROR: Format isn't 1 byte / sample") ;
            error = ERR_NOK ;
            break ;
        }

        error = ReadShortFromFile (&tmpS, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;
        if (tmpS != 8) {
            AddLog(LOG_TAPE,"ERROR: Format isn't 8 bits / sample") ;
            error = ERR_NOK ;
            break ;
        }

        error = ReadStringFromFile (str, 4, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;
        if (strcmp (str, "data") != 0) {
            error = myfseek (ptrFile, -2, SEEK_CUR) ;
            if (error != ERR_OK) {
                AddLog(LOG_TAPE,"ERROR: Can't seek the file") ;
                break ;
            }

            error = ReadStringFromFile (str, 4, &ptrFile->ptrFd) ;
            if (error != ERR_OK) break ;
            if (strcmp (str, "data") != 0) {
                error = ReadStringFromFile (str, 8, &ptrFile->ptrFd) ;
                if (error != ERR_OK) break ;

                error = ReadStringFromFile (str, 4, &ptrFile->ptrFd) ;
                if (error != ERR_OK) break ;

                if (strcmp (str, "data") != 0) {
                    AddLog(LOG_TAPE,"ERROR: Data Header isn't 'data'") ;
                    error = ERR_NOK ;
                    break ;
                }
            }
        }

        error = ReadLongFromFile (&tmpL, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;

    } while (0) ;
    return (error);
}

int Cce152::WriteHeadToWav (u_long    nbSamp,
                    u_long    freq,
                    WavFileInfo* ptrFile)
{
    int     error ;

    do {
        error = WriteStringToFile ("RIFF", &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;

        error = WriteLongToFile ((nbSamp + 36), &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;

        error = WriteStringToFile ("WAVEfmt ", &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;

        error = WriteLongToFile (0x10, &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;

        error = WriteShortToFile (1, &ptrFile->ptrFd) ;      /* PCM */
        if (error != ERR_OK) break ;

        error = WriteShortToFile (1, &ptrFile->ptrFd) ;      /* Mono */
        if (error != ERR_OK) break ;

        error = WriteLongToFile (freq, &ptrFile->ptrFd) ;    /* Samp Freq */
        if (error != ERR_OK) break ;

        error = WriteLongToFile (freq, &ptrFile->ptrFd) ;    /* Byte / sec */
        if (error != ERR_OK) break ;

        error = WriteShortToFile (1, &ptrFile->ptrFd) ;      /* Byte / Samp x Chan */
        if (error != ERR_OK) break ;

        error = WriteShortToFile (8, &ptrFile->ptrFd) ;      /* Bit / Samp */
        if (error != ERR_OK) break ;

        error = WriteStringToFile ("data", &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;

        error = WriteLongToFile (nbSamp, &ptrFile->ptrFd) ;  /* Nb Samples */
        if (error != ERR_OK) break ;

    } while (0) ;
    return (error);
}

int ReadFileLength (u_long* ptrLen,
                    FILE**  ptrFd)
{
    long    nbByte ;
    int     error ;

    do {
        *ptrLen = 0 ;

        /* Seek to the end of the source file */
        error = fseek (*ptrFd, 0, SEEK_END) ;
        if (error != ERR_OK) {
            printf ("ERROR : Can't seek the file\n") ;
            break ;
        }

        /* Get the length of the source file */
        nbByte = ftell (*ptrFd) ;
        if (nbByte == ERR_NOK) {
            printf ("ERROR : Can't ftell the file\n") ;
            error = ERR_NOK ;
            break ;
        }

        /* Seek to the begining of the source file */
        error = fseek (*ptrFd, 0, SEEK_SET) ;
        if (error != ERR_OK) {
            printf ("ERROR : Can't seek the file\n") ;
            break ;
        }

        if (nbByte == 0) {
            printf ("ERROR : Source file is empty\n") ;
            error = ERR_NOK ;
            break ;
        }

        *ptrLen = nbByte ;

    } while (0) ;
    return (error);
}

int Cce152::UpdateHeadToWav (u_long    nbSamp,WavFileInfo* ptrFile)
{
    int     error ;


    do {
		u_long	Length;
		error=ReadFileLength (&Length,&ptrFile->ptrFd);
AddLog(LOG_TAPE,tr("Length=%1").arg(Length));
		error = fseek (ptrFile->ptrFd, 4, SEEK_SET) ;

        error = WriteLongToFile ((Length - 8), &ptrFile->ptrFd) ;
        if (error != ERR_OK) break ;

		error = fseek (ptrFile->ptrFd, 40, SEEK_SET) ;

        AddLog(LOG_TAPE,"ERROR");
        error = WriteLongToFile (Length - 44, &ptrFile->ptrFd) ;  /* Nb Samples */
        if (error != ERR_OK) break ;
AddLog(LOG_TAPE,"ERROR");
    } while (0) ;
    return (error);
}


int Cce152::WriteStringToFile (char*  ptrStr,
                       FILE** ptrFd)
{
    int     error ;

    error = fputs (ptrStr, *ptrFd) ;
    if (error == EOF) {
        printf ("ERROR : Can't write in the file\n") ;
        error = ERR_NOK ;
    }
    else {
        error = ERR_OK ;
    }
    return (error);
}

int Cce152::WriteLongToFile (u_long value,
                     FILE** ptrFd)
{
    unsigned char    str[10] ;
    int     ii ;
    int     error =0;

    CvLongToStringI (value, str) ;

    for ( ii = 0 ; ii < 4 ; ii++  ) {
        error = fputc (str[ii], *ptrFd) ;
        if (error == EOF) {
            printf ("ERROR : Can't write in the file\n") ;
            error = ERR_NOK ;
            break ;
        }
        else
            error = ERR_OK ;
    }
    return (error);
}

int Cce152::WriteShortToFile (u_long value,
                      FILE** ptrFd)
{
    char    str[10] ;
    int     ii ;
    int     error=0 ;

    CvShortToStringI (value, str) ;

    for ( ii = 0 ; ii < 2 ; ii++ ) {
        error = fputc (str[ii], *ptrFd) ;
        if (error == EOF) {
            printf ("ERROR : Can't write in the file\n") ;
            error = ERR_NOK ;
            break ;
        }
        else
            error = ERR_OK ;
    }
    return (error);
}

void Cce152::CvShortToStringI (u_short value,
                       char*   ptrStr)
{
    u_short tmp ;

    /* Convert the short value into a String with msb first (INTEL) */
    tmp = value & 0xFF ;
    *ptrStr ++ = (char) tmp ;
    tmp = value >> 8 ;
    *ptrStr ++ = (char) tmp ;
    *ptrStr = 0 ;
}

void Cce152::CvLongToStringI (u_long value,
                      unsigned char*  ptrStr)
{
    u_long  tmp ;

    /* Convert the long value into a String with msb first (INTEL) */
    tmp = value & 0xFF ;
    *ptrStr ++ = (unsigned char) tmp ;
    tmp = (value >> 8) & 0xFF ;
    *ptrStr ++ = (unsigned char) tmp ;
    tmp = (value >> 16) & 0xFF ;
    *ptrStr ++ = (unsigned char) tmp ;
    tmp = value >> 24 ;
    *ptrStr ++ = (unsigned char) tmp ;
    *ptrStr = 0 ;
}


