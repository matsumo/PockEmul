#ifndef PC1211_H
#define PC1211_H
#include <QMutex>

#include "pcxxxx.h"

class CTinyBasic;
class Clcdc_pc1211;

class Cpc1211 : public CpcXXXX
{
public:
    Cpc1211(CPObject *parent = 0);
    virtual ~Cpc1211();

    virtual	bool	init(void);				// initialize
    virtual	bool	run(void);					// emulator main
    virtual bool	exit(void);

    virtual bool	Set_Connector(void);

    virtual bool	Chk_Adr(UINT32 *d,UINT32 data) {Q_UNUSED(d) Q_UNUSED(data) return true;}
    virtual bool	Chk_Adr_R(UINT32 *d,UINT32 *data) {Q_UNUSED(d) Q_UNUSED(data)return true;}
    virtual UINT8 in(UINT8 adr) {Q_UNUSED(adr) return 0;}
    virtual UINT8 out(UINT8 adr,UINT8 val) {Q_UNUSED(adr) Q_UNUSED(val) return 0;}
    void afficheChar(quint8 c);

    Clcdc_pc1211* pLCDC1211;

    QByteArray inputBuffer;
    QByteArray printerBuffer;

    CTinyBasic *pBASIC;
    bool DisplayWaitForRTN;
    void Editor();
    int cursorPos;
    bool shiftFlag;
    bool printerConnected();

    void print(unsigned char c);

    QMutex printerBufferMutex;
    void sendToPrinter();

    qint64 lastBreakPress;
};

#endif // PC1211_H
