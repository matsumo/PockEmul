#ifndef CE153_H
#define CE153_H


#include "pobject.h"

class Cconnector;
class CbusPc1500;
class CLH5810;

class Cce153:public CPObject{
public:

    virtual bool		run(void);
    virtual bool		init(void);
    virtual bool		exit(void);

    Cconnector	*pCONNECTOR;	qint64 pCONNECTOR_value;
    CbusPc1500    *bus;
    CLH5810		*pLH5810;

    bool lh5810_write(void);
    bool lh5810_read(void);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    virtual bool    SaveSession_File(QXmlStreamWriter *xmlOut);
    virtual bool	LoadSession_File(QXmlStreamReader *xmlIn);

    quint8 keyPressed;

    Cce153(CPObject *parent = 0);
    virtual ~Cce153();

private:
    bool dragging;
};



#endif // CE153_H
