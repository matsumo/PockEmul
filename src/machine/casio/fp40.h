#ifndef FP40_H
#define FP40_H

#include "printerctronics.h"

class DialogConsole;

class Cfp40:public CprinterCtronics{
Q_OBJECT
public:

    virtual bool run(void);
    virtual bool init(void);				//initialize
    virtual bool exit(void);				//end
    //virtual void ExportByte(qint8);
    virtual void ComputeKey(void);
    virtual bool UpdateFinalImage(void);


    Cfp40(CPObject *parent = 0);
    virtual ~Cfp40();


    virtual void Printer(quint8 data);

protected:

protected slots:


private:
    enum escSEQ {NONE,WAITCMD,CMD_A,CMD_K,CMD_L,READ_DATA};
    escSEQ escapeSeq;
    escSEQ currentCMD;
    int n1,n2;
    int readCounter;
    QByteArray readData;
    int paperfeedsize;

};
#endif // FP40_H
