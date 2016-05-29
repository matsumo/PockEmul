#ifndef _KEYB_H_
#define _KEYB_H_

#include <QList>
#include <QXmlDefaultHandler>
#include "common.h"

#if 0
enum KEY_NAME {
K_BRK,
K_UA,
K_DA,
K_LA,
K_RA,
K_MOD,
K_INS,
K_DEL,
K_CLR,
K_RET,
K_SML,
K_DEF,
K_SHT,
K_OB,
K_OF,
K_RCL,
K_RSV,
K_SIGN,
K_CAL,
K_BASIC,
K_SQR,
K_ROOT,
K_POT,
K_EXP,
K_XM,
K_STAT,
K_1X,
K_LOG,
K_LN,
K_DEG,
K_HEX,
K_MPLUS,
K_CCE,
K_FSE,
K_TAN,
K_COS,
K_SIN,
K_HYP,
K_RM,
K_CTRL,K_BS,K_KBII,
K_F0,K_F1,K_F2,K_F3,K_F4,K_F5,K_F6,K_F7,
K_A,K_B,K_C,K_D,K_E,K_F,K_G,K_H,K_I,K_J,K_K,K_L,K_M,K_N,K_O,K_P,K_Q,K_R,K_S,K_T,K_U,K_V,K_W,K_X,K_Y,K_Z,
K_0,K_1,K_2,K_3,K_4,K_5,K_6,K_7,K_8,K_9
};
#endif

#define	MAX_KO		12					//KOn(0-11)
#define NUL			0xff
#define NM			15

#define K_BRK		0x03		/* ctrl-C */
#define K_DA		0x04 // ???
#define K_UA		0x05 // ???
#define K_LA		0x06 // ???
#define K_RA		0x07 // ???
#define K_MOD		0x08
#define K_INS		0x09
#define K_DEL		0x0a
#define K_DEF		0x0b
#define K_CLR		0x0c
#define K_RET		0x0d
#define K_PEN		0x0e
#define K_TAB       0x0f
#define K_CLOSE     0x10
#define K_SHT		0x11
#define K_SHT2		0x12
#define K_SML		0x14
#define K_OB		0x3f
#define K_DBLQUOTE  0x22

#if 1
#define K_0         0xa0
#define K_1         0xa1
#define K_2         0xa2
#define K_3         0xa3
#define K_4         0xa4
#define K_5         0xa5
#define K_6         0xa6
#define K_7         0xa7
#define K_8         0xa8
#define K_9         0xa9
#endif

#define K_MIN       0xaa
#define K_PT        0xab
#define K_SLH       0xac
#define K_COMMA     0xad
#define K_BLANK     0xae
#define K_QUOTE     0xb0
#define K_BRACKET   0xb1
#define K_OF		0xb2

#define K_PRINT		0xc0
#define K_PFEED		0xc1
#define K_PBFEED	0xc2
#define K_PRINT_ON	0xc3
#define K_PRINT_OFF 0xc4
#define K_RMT_ON    0xc5
#define K_RMT_OFF   0xc6
#define K_POW_ON    0xc7
#define K_POW_OFF   0xc8
#define K_PRT_COND  0xc9
#define K_PRT_TRACE 0xca
#define K_PRT_NORM  0xcb
#define K_PRT_MANUAL  0xcc
#define K_PRT_INT_MIN 0xcd
#define K_PRT_INT_NORM 0xce
#define K_PRT_INT_MAX 0xcf

#define K_EJECT		0xd0
#define K_PLAY		0xd1
#define K_STOP		0xd2
#define K_LOAD		0xd3
#define K_RECORD	0xd4
#define K_CTRL      0xd5
#define K_BS        0xd6
#define K_KBII      0xd7
#define K_CAL_PER   0xd8
#define K_CAL_RES   0xd9
#define K_MENU      0xda
#define K_EXTMENU   0xdb
#define K_KANA      0xdc
#define K_STO		0xdd
#define K_RCL		0xde
#define K_RSV		0xdf

#define K_SIGN		0xe0
#define K_CAL		0xe1
#define K_BASIC		0xe2
#define K_SQR		0xe3
#define K_ROOT		0xe4
#define K_POT		0xe5
#define K_EXP		0xe6
#define K_XM		0xe7
#define K_STAT		0xe8
#define K_1X		0xe9
#define K_LOG		0xea
#define K_LN		0xeb
#define K_DEG		0xec
#define K_HEX		0xed
#define K_MPLUS		0xee
#define K_CCE		0xef
#define K_FSE		0xf0
#define K_TAN		0xf1
#define K_COS		0xf2
#define K_SIN		0xf3
#define K_HYP		0xf4
#define K_RM		0xf5

#define K_F1		0xf6
#define K_F2		0xf7
#define K_F3		0xf8
#define K_F4		0xf9
#define K_F5		0xfa
#define K_F6		0xfb
#define K_F7		0xfc
#define K_F8		0xfd
#define K_F9		0xfe
#define K_F0		0xff

#if 0
#define K_A			0x41
#define K_B			0x42
#define K_C			0x43
#define K_D			0x44
#define K_E			0x45
#define K_F			0x46
#define K_G			0x47
#define K_H			0x48
#define K_I			0x49
#define K_J			0x50
#define K_K			0x51
#define K_L			0x52
#define K_M			0x53
#define K_N			0x54
#define K_O			0x55
#define K_P			0x56
#define K_Q			0x57
#define K_R			0x58
#define K_S			0x59
#define K_T			0x60
#define K_U			0x61
#define K_V			0x62
#define K_W			0x63
#define K_X			0x64
#define K_Y			0x65
#define K_Z			0x66
#endif

#define K_TS_00     0x100
#define K_TS_01     0x101
#define K_TS_02     0x102
#define K_TS_03     0x103
#define K_TS_10     0x110
#define K_TS_11     0x111
#define K_TS_12     0x112
#define K_TS_13     0x113
#define K_TS_20     0x120
#define K_TS_21     0x121
#define K_TS_22     0x122
#define K_TS_23     0x123
#define K_TS_30     0x130
#define K_TS_31     0x131
#define K_TS_32     0x132
#define K_TS_33     0x133

#define K_NEWALL    0x200
#define K_LCKEY     0x201
#define K_CALC      0x202
#define K_MEMO      0x203
#define K_OUT       0x204
#define K_IN        0x205
#define K_MEMO_IN   0x206
#define K_ANS       0x207
#define K_PI        0x208
#define K_CON       0x209
#define K_TXT       0x20a
#define K_MDF       0x20b
#define K_NPR       0x20c
#define K_RESET     0x20d
#define K_CETL      0x20e
#define K_SEARCH    0x20f
#define K_ENG       0x210
#define K_OFF       0x211

// FIX ME
#define K_SHIFT_DOWN_MOD 0x210
#define K_SHIFT_UP_MOD 0x211
#define K_CTRL_DOWN_MOD  0x212
#define K_CTRL_UP_MOD  0x213

#define K_HELP      0x220
#define K_IO        0x221
#define K_STP       0x222
#define K_ROTATE    0x223
#define K_C1        0x224
#define K_C2        0x225
#define K_C3        0x226
#define K_C4        0x227
#define K_LOCK      0x228

// TI57
#define K_GTO       0x230
#define K_SBR       0x231
#define K_RST       0x232
#define K_RS        0x233
#define K_LRN       0x234
#define K_XT        0x235
#define K_SST       0x236
#define K_STL       0x238
#define K_SUM       0x239
#define K_BST       0x23a
#define K_EE        0x23b
#define K_CE        0x23c
#define K_RUN       0x23D
#define K_FN        0x23E
#define K_GSB       0x23F
#define K_SHARP11PINS    0x240
#define K_XY        0x241



// FX-8000G
#define K_ALPHA       0x250
#define K_INIT       0x251
#define K_DISP       0x252
#define K_GRAPH       0x253
#define K_RANGE       0x254
#define K_GT       0x255
#define K_PROG       0x256
#define K_XROOT       0x257
#define K_FIX       0x258
#define K_LIST      0x259
#define K_FUNC      0x25a
#define K_OLD      0x25b
#define K_CM      0x25c

#define K_PAPER_POS 0x260
#define K_HOME 0x261
#define K_PAUSE 0x262
#define K_NUM 0x263
#define K_SCREEN 0x264
#define K_OVERLAY 0x265

class CPObject;
class CpcXXXX;
class Ckeyb;
#include "viewobject.h"

class CKey{
public:
    CKey();
    CKey(int ScanCode, QString Description, QRect Rect = QRect(), int masterscancode=0, QString modifier = QString(), View view = FRONTview);
    ~CKey(){}
	
	int		Icode;
    QString	Description;
    int		ScanCode,MasterScanCode;
    QString Modifier;
	QRect	Rect;
    View    view;
    bool    enabled;

protected:
};


class KEYBMAPParser : public QXmlDefaultHandler
{
public:
  bool startDocument();
  bool endElement( const QString&, const QString&, const QString &name );
  bool startElement( const QString&, const QString&, const QString &name, const QXmlAttributes &attrs );

    KEYBMAPParser(Ckeyb *parent = 0)
    {
        this->Parent = parent;
    }
private:
  bool inKeyboard;
  Ckeyb *Parent;
};

class Ckeyb
{
public:
    CpcXXXX *pPC;
    CPObject *Parent;

	virtual bool init(void);				//initialize
	virtual bool exit(void);				//end
    virtual void read_config(void){}		//read key config
	void	Set_KS(BYTE data);
	BYTE	Get_KS(void);

	BYTE	Read(BYTE);						// Scan keyboard
	bool	CheckKon();
	int		CheckOff();

	QString KeyString(QPoint pts);
    int	KeyClick(QPoint pts);
	void	keyscan(void);
	BYTE	*scandef;

	BYTE	KStrobe;							// Last K strobe signal sent
	BYTE	keym[200];
	bool	access;							//KO access flag(0:none,1:access)
	QString fn_KeyMap;
    bool	Kon;
    int	    LastKey,lastMousePressedKey;

    QList<int> keyPressedList;

	QList<CKey>	Keys;
    CKey &getKey(int code);
	bool	modified;
	int		baseWidth;
	int		baseHeight;

    bool    isShift;
    bool    isCtrl;


    bool enabled;

	KEYBMAPParser *handler;
		
    Ckeyb(CPObject *parent = 0,QString map = "",BYTE *scan=0);
    virtual ~Ckeyb();
					
protected:
	BYTE pc1350KeyStatus[MAX_KO];			//Ko0-11
	BYTE IA_PORT;

};




#endif
