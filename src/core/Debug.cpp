
#include <QDebug>

#include "Debug.h"
#include "cpu.h"

UINT32 Cdebug::DisAsm_File(UINT32 adr,FILE *fp)
{
	UINT32 Result;

	Result = DisAsm_1(adr);
	fprintf(fp,"%s",Buffer);
	return(Result);
}

void Cdebug::header(UINT32 adr,int l) {
    sprintf(Buffer,"%05X:",(uint) adr);
    for(int i=0;i<l;i++)
        sprintf(Buffer,"%s%02X",Buffer,(uint)pCPU->get_mem(adr+i,SIZE_8));
}

Cdebug::Cdebug(CCPU *parent)	: QObject(parent)
{							//[constructor]
    breakf		= 0;
    breakadr	= 0;
    debugged	= 0;	//break point(0:disable, 1:enable)
    isdebug		= 0;			//debug?(0:none, 1:debugging)

    pCPU = parent;

    loadSymbolMap();
}

char *Cdebug::toSymbol(quint32 adr,int size)
{
    if (symbolMap.contains(adr)) {
        return symbolMap[adr]->toLbl().toLatin1().data();
    }
    else {
        sprintf(tmpSymbolLabel,"%.*x",size,adr & ( (1<<(size*4))-1));
        return (char*)tmpSymbolLabel;
    }
}

/*****************************************************************************/
/* Initializeing Debugger													 */
/*  ENTRY :none																 */
/*  RETURN:none																 */
/*****************************************************************************/
bool Cdebug::init(void)
{
	return true;
}

/*****************************************************************************/
/* Exitting Debugger														 */
/*  ENTRY :none																 */
/*  RETURN:none																 */
/*****************************************************************************/
bool Cdebug::exit(void)
{
	return true;
}



QString Csymbol::toLbl()
{
    if (name.contains("$$")){
        return name.mid(name.indexOf("$$")+2);
    }
    return name;
}
