#include "common.h"
#include "Log.h"
#include "cpu.h"
#include "Debug.h"
#include "pcxxxx.h"

CCPU::CCPU(CPObject *parent)
{				//[constructor]
    pPC = (CpcXXXX*) parent;

    halt=0;				//halt?(0:none, 1:halting)
    off = 0;
    end=0;				//program end?(0:none, 1:end)
    savesw=1;			//end with memory save?(0:no, 1:yes)
    cpulog=0;				//execute log?(0:off, 1:on)
    logsw=false;			//log mode?(0:off, 1:on)
    resetFlag = false;
    fp_log=0;
    fn_log="cpu.log";
    CallSubLevel=prevCallSubLevel=0;

    imemsize = 0x200;
    for (int i=0;i<imemsize;i++) imem[i]=0;

    regwidget = 0;
    pDEBUG=0;
    connect(this,SIGNAL(msgError(QString)),mainwindow,SLOT(slotMsgError(QString)));
    connect(this,SIGNAL(showDasm()),pPC,SLOT(Dasm()));
}

CCPU::~CCPU() {
//    delete regwidget;
    delete pDEBUG;
}

 
void CCPU::Check_Log(void)
{
    if( logsw && (!fp_log) ) fp_log=fopen(fn_log,"wt");	// Open log file
	if( (!logsw) && fp_log ) fclose(fp_log);					// Close log file
}

bool CCPU::exit(void)
{
	if(logsw) fclose(fp_log);							//close log file

	pDEBUG->exit();

	return true;
}

QByteArray CCPU::getimem() {
    return (QByteArray((const char*)&imem,imemsize));
}

void	CCPU::setImemBit(WORD adr, int bit, BYTE data)
{
    int t;

    if (data)
    {
        t=1<<(bit-1);
        imem[adr] |= t;
    }
    else
    {
        t=0xFF - (1<<(bit-1));
        imem[adr] &= t;
    }
}

/*!
 \brief Get data from memory

 \fn Csc::get_mem
 \param adr     address
 \param size    SIZE_08 or SIZE_16 or SIZE_20 or SIZE_24
 \return DWORD  value
*/
UINT32 CCPU::get_mem(UINT32 adr,int size)
{
    switch(size)
    {
    case 8:
    case SIZE_8 :return(pPC->Get_PC(adr));
    case 16:
    case SIZE_16:return(pPC->Get_PC(adr)+(pPC->Get_PC(adr+1)<<8));
    case 20:
    case SIZE_20:return((pPC->Get_PC(adr)+(pPC->Get_PC(adr+1)<<8)+(pPC->Get_PC(adr+2)<<16))&MASK_20);
    case 24:
    case SIZE_24:return((pPC->Get_PC(adr)+(pPC->Get_PC(adr+1)<<8)+(pPC->Get_PC(adr+2)<<16))&MASK_24);
    }
    return(0);
}
/*****************************************************************************/
/* Set data to memory														 */
/*  ENTRY :DOWRD adr=address, int size=SIZE_xx, DWORD data=value			 */
/*  RETURN:none																 */
/*****************************************************************************/
void CCPU::set_mem(UINT32 adr,int size,UINT32 data)
{
    switch(size)
    {
    case SIZE_8 :
        pPC->Set_8(adr , (BYTE) data);
        break;
    case SIZE_16:
        pPC->Set_8(adr , (BYTE) data);
        pPC->Set_8(adr+1 , (BYTE) (data>>8));
        break;
    case SIZE_20:
        pPC->Set_8(adr , (BYTE) data);
        pPC->Set_8(adr+1 , (BYTE) (data>>8));
        pPC->Set_8(adr+2 , (BYTE) ((data>>16)&MASK_4));
        break;
    case SIZE_24:
        pPC->Set_8(adr , (BYTE) data);
        pPC->Set_8(adr+1 , (BYTE) (data>>8));
        pPC->Set_8(adr+2 , (BYTE) (data>>16));
        break;
    }
}
