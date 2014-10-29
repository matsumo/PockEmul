#ifndef _PCXXXX_H
#define _PCXXXX_H

#include <stdlib.h>
#include <stdio.h>


#include <QFile>

#include "pobject.h"

class CbreakpointManager;
class Clcdc;
class CCPU;
class Cconnector;
class CExtension;
class CExtensionArray;
class Cprinter;
class Ckeyb;
#include "slot.h"



#define	SLOT1		0
#define	SLOT2		1
#define	SLOT3		2
#define	SLOT4		3
#define	SLOT5		4
#define	SLOT6		5
#define	SLOT7		6
#define	SLOT8		7
#define	SLOT9		8
#define	SLOT10		9
#define	SLOT11		10
#define	SLOT12		11
#define	SLOT13		12
#define	SLOT14		13
#define	SLOT15		14
#define	SLOT16		15

#define	SLOT3EXT	17


#define EXTENSION_CE125				ext_11pins->ExtArray[ID_CE125]
#define EXTENSION_CE126P			ext_11pins->ExtArray[ID_CE126P]
#define EXTENSION_CE123P			ext_11pins->ExtArray[ID_CE123P]
#define EXTENSION_CE129P			ext_11pins->ExtArray[ID_CE129P]
#define EXTENSION_CE140P			ext_Serial->ExtArray[ID_CE140P]

#define EXTENSION_CE125_CHECK		EXTENSION_CE125->IsChecked
#define EXTENSION_CE126P_CHECK		EXTENSION_CE126P->IsChecked
#define EXTENSION_CE123P_CHECK		EXTENSION_CE123P->IsChecked
#define EXTENSION_CE129P_CHECK		EXTENSION_CE129P->IsChecked
#define EXTENSION_CE140P_CHECK		EXTENSION_CE140P->IsChecked

#define EXTENSION_CE201M			ext_MemSlot1->ExtArray[ID_CE201M]
#define EXTENSION_CE202M			ext_MemSlot1->ExtArray[ID_CE202M]
#define EXTENSION_CE203M			ext_MemSlot1->ExtArray[ID_CE203M]
#define EXTENSION_CE210M			ext_MemSlot1->ExtArray[ID_CE210M]
#define EXTENSION_CE211M			ext_MemSlot1->ExtArray[ID_CE211M]
#define EXTENSION_CE212M			ext_MemSlot1->ExtArray[ID_CE212M]

#define EXTENSION_CE201M_CHECK		EXTENSION_CE201M->IsChecked
#define EXTENSION_CE202M_CHECK		EXTENSION_CE202M->IsChecked
#define EXTENSION_CE203M_CHECK		EXTENSION_CE203M->IsChecked
#define EXTENSION_CE210M_CHECK		EXTENSION_CE210M->IsChecked
#define EXTENSION_CE211M_CHECK		EXTENSION_CE211M->IsChecked
#define EXTENSION_CE212M_CHECK		EXTENSION_CE212M->IsChecked

#define S1_EXTENSION_CE210M_CHECK	ext_MemSlot1->ExtArray[ID_CE210M]->IsChecked
#define S1_EXTENSION_CE211M_CHECK	ext_MemSlot1->ExtArray[ID_CE211M]->IsChecked
#define S1_EXTENSION_CE212M_CHECK	ext_MemSlot1->ExtArray[ID_CE212M]->IsChecked
#define S1_EXTENSION_CE2H16M_CHECK	ext_MemSlot1->ExtArray[ID_CE2H16M]->IsChecked
#define S1_EXTENSION_CE2H32M_CHECK	ext_MemSlot1->ExtArray[ID_CE2H32M]->IsChecked
#define S1_EXTENSION_CE2H64M_CHECK	ext_MemSlot1->ExtArray[ID_CE2H64M]->IsChecked

#define S2_EXTENSION_CE210M_CHECK	ext_MemSlot2->ExtArray[ID_CE210M]->IsChecked
#define S2_EXTENSION_CE211M_CHECK	ext_MemSlot2->ExtArray[ID_CE211M]->IsChecked
#define S2_EXTENSION_CE212M_CHECK	ext_MemSlot2->ExtArray[ID_CE212M]->IsChecked
#define S2_EXTENSION_CE2H16M_CHECK	ext_MemSlot2->ExtArray[ID_CE2H16M]->IsChecked
#define S2_EXTENSION_CE2H32M_CHECK	ext_MemSlot2->ExtArray[ID_CE2H32M]->IsChecked
#define S2_EXTENSION_CE2H64M_CHECK	ext_MemSlot2->ExtArray[ID_CE2H64M]->IsChecked

#define EXTENSION_CE150				ext_60pins->ExtArray[ID_CE150]
#define EXTENSION_CE151				ext_MemSlot1->ExtArray[ID_CE151]
#define EXTENSION_CE155				ext_MemSlot1->ExtArray[ID_CE155]
#define EXTENSION_CE159				ext_MemSlot1->ExtArray[ID_CE159]
#define EXTENSION_CE160				ext_MemSlot1->ExtArray[ID_CE160]
#define EXTENSION_CE161				ext_MemSlot1->ExtArray[ID_CE161]

#define EXTENSION_CE150_CHECK		EXTENSION_CE150->IsChecked
#define EXTENSION_CE151_CHECK		EXTENSION_CE151->IsChecked
#define EXTENSION_CE155_CHECK		EXTENSION_CE155->IsChecked
#define EXTENSION_CE159_CHECK		EXTENSION_CE159->IsChecked
#define EXTENSION_CE160_CHECK		EXTENSION_CE160->IsChecked
#define EXTENSION_CE161_CHECK		EXTENSION_CE161->IsChecked



enum PORTS{
	PORT_A,PORT_B,PORT_C,PORT_F,PORT_T};

#define GET_PORT_BIT(Port,Bit) ((Get_Port(Port) & (1<<((Bit)-1))) ? 1:0)

extern TransMap KeyMap1250[]; extern int KeyMap1250Lenght; 
extern TransMap KeyMap1280[]; extern int KeyMap1280Lenght;
extern TransMap KeyMap1350[]; extern int KeyMap1350Lenght;
extern TransMap KeyMap1360[]; extern int KeyMap1360Lenght;
extern TransMap KeyMap2500[]; extern int KeyMap2500Lenght;

class CpcXXXX:public CPObject{
Q_OBJECT
public:
    virtual const char*	GetClassName(){ return("CpcXXXX");}


	
	QString	Initial_Session_Fname;
	bool Initial_Session_Load();
	bool Initial_Session_Save();

	virtual void Regs_Info(UINT8);
	virtual void TurnON(void);
    virtual void TurnOFF(void);
	virtual void Reset();

    virtual bool UpdateFinalImage();

	virtual bool InitDisplay(void);
	virtual bool CompleteDisplay(void);

	CCPU		*pCPU;
	Cconnector	*pCONNECTOR;		qint64 pCONNECTOR_value;
	Cconnector	*pSIOCONNECTOR;		qint64 pSIOCONNECTOR_value;

	bool	SoundOn;



	BYTE	IO_A,IO_B,IO_C,IO_F,IO_T;			// Four Input Output port 
	bool	Get_Port_bit(PORTS Port, int bit);
	virtual BYTE	Get_Port(PORTS Port);
	virtual void	Set_Port(PORTS Port,BYTE data);
	virtual void	Set_Port_Bit(PORTS Port, int bit, BYTE data);

	virtual BYTE	Get_PortA(void);
	virtual BYTE	Get_PortB(void);
	virtual BYTE	Get_PortC(void);
	virtual BYTE	Get_PortF(void);
	virtual BYTE	Get_PortT(void);
	virtual void	Set_PortA(BYTE data);
	virtual void	Set_PortB(BYTE data);
	virtual void	Set_PortC(BYTE data);
	virtual void	Set_PortF(BYTE data);
	virtual void	Set_PortT(BYTE data);

	virtual bool	LoadConfig(QFile *);	// Load PC Configuration
    virtual bool	LoadConfig(QXmlStreamReader *);	// Load PC Configuration
    virtual bool	LoadExt(QXmlStreamReader *);	// Load PC Configuration
	virtual bool	SaveConfig(QFile *);	// Save PC Configuration
    virtual bool	SaveConfig(QXmlStreamWriter *);	// Save PC Configuration
    virtual bool	SaveExt(QXmlStreamWriter *);	// Save PC Configuration

	virtual bool	LoadExtra(QFile *);
	virtual bool	SaveExtra(QFile *);


	virtual	bool	init(void);				// initialize
	virtual	bool	run(void);					// emulator main
	virtual bool	exit(void);					// exit
    virtual bool	Set_Connector(Cbus *_bus = 0) { return (1); }
    virtual bool	Get_Connector(Cbus *_bus = 0) { return (1); }


    virtual void    ExtChanged(void);


	char	Regs_String[1024];

    virtual BYTE    Get_PC(UINT32 adr);
	BYTE	Get_8(UINT32 adr);
	WORD	Get_16(UINT32 adr);
	WORD	Get_16r(UINT32 adr);
    virtual WORD	Get_16rPC(UINT32 adr);
    UINT32 Get_20(UINT32 adr);
    UINT32 Get_24(UINT32 adr);
    UINT32 get_mem(UINT32 adr, int size);

	void	Set_8(UINT32 adr,BYTE d);
	void	Set_16(UINT32 adr,WORD d);
    void	Set_16r(UINT32 adr,WORD d);
    void Set_20(UINT32 adr,UINT32 d);
    void Set_24(UINT32 adr,UINT32 d);
    void set_mem(UINT32 adr, int size, UINT32 data);
	virtual bool	Chk_Adr(UINT32 *d,UINT32 data) = 0;
    virtual bool	Chk_Adr_R(UINT32 *d, UINT32 *data) = 0;
    virtual UINT8 in(UINT8 address)=0;
    virtual UINT8 out(UINT8 address,UINT8 value)=0;
    virtual UINT8 in8(UINT16 address){Q_UNUSED(address) return 0;}
    virtual UINT8 out8(UINT16 address,UINT8 value){Q_UNUSED(address) Q_UNUSED(value) return 0;}
    virtual UINT16 in16(UINT16 address){Q_UNUSED(address) return 0;}
    virtual UINT16 out16(UINT16 address,UINT16 value){Q_UNUSED(address) Q_UNUSED(value) return 0;}

    QByteArray getmem();

	void	SaveSession(void);
	bool	SaveSession_File(QFile *);
    virtual bool    SaveSession_File(QXmlStreamWriter *xmlOut);

    void	LoadSession(void);
	bool	LoadSession_File(QFile *);
    virtual bool	LoadSession_File(QXmlStreamReader *);

    char	Log_String[1024];

    quint64	old_state;



    quint8		RomBank,RamBank;					// Bank Number id
    int     ProtectMemory;

	int		Tape_Base_Freq;

	CExtensionArray	*ext_11pins;
	CExtensionArray	*ext_MemSlot1;
	CExtensionArray	*ext_MemSlot2;
    CExtensionArray	*ext_MemSlot3;
	CExtensionArray	*ext_Serial;
	CExtensionArray	*ext_60pins;
	

	void		addExtMenu(CExtensionArray *ext);

	void		updateExtension(CExtensionArray *,QAction *);
	void		updateExtension(QAction *action);
	CExtension * findExtension(CExtensionArray *array,QAction *action);
	virtual bool	CheckUpdateExtension(CExtension *ext);
	void		updateMenuFromExtension(void);
	void		emptyExtensionArray(QAction *action);
    void		loadExtensionArray(QAction *action);
    void		saveExtensionArray(QAction *action);


	
	bool	Japan;
    int     timerRate;


	CpcXXXX(CPObject *parent = 0);
	
    virtual ~CpcXXXX();


private slots:
	void manageEmptyExtensions();
    void manageLoadExtensions();
    void manageSaveExtensions();
	void manageExtensions(QAction *);
	
signals:
    void askRefreshDasm();
    void showDasm();

public:
    bool DasmStep;
    bool DasmFlag;
    UINT32 DasmLastAdr;
    QMap<QPair<UINT32,UINT32>,Qt::CheckState> TraceRange;
    int BreakSubLevel;

    CbreakpointManager *pBreakpointManager;

    bool checkTraceRange(UINT32 adr);
    void checkBreakRead(UINT32 adr, UINT32 d);
    void checkBreakWrite(UINT32 adr, UINT32 d);
};


#endif


