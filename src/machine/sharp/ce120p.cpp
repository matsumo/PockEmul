#include "ce120p.h"
#include "Connect.h"
#include "Keyb.h"
#include "watchpoint.h"



Cce120p::Cce120p(CPObject *parent):Cce126(this)
{								//[constructor]
    BackGroundFname	= P_RES(":/ext/ce-120p.png");
    setcfgfname("ce120p");
    setDX(883);
    setDY(568);

    setDXmm(253);
    setDYmm(164);
    setDZmm(38);

    setPaperPos(QRect(89,185-149,207,149));

    delete pKEYB; pKEYB=new Ckeyb(this,"ce120.map");

    //internal_device_code = 0x21;
}

Cce120p::~Cce120p() {
    delete pCONNECTOR_Ext;
}

bool Cce120p::init(void) {

    bool res = Cce126::init();
    pCONNECTOR->setSnap(QPoint(400,131));

    pCONNECTOR_Ext = new Cconnector(this,11,1,Cconnector::Sharp_11,"Connector 11 pins Ext.",false,QPoint(0,370));	publish(pCONNECTOR_Ext);

    WatchPoint.add(&pCONNECTOR_Ext_value,64,11,this,"Ext. 11pins connector");

    ACK = false;
    return res;
}

bool Cce120p::exit(void) {

    return Cce126::exit();
}


bool Cce120p::Get_Connector(Cbus *_bus) {
    MT_OUT2	= GET_PIN(PIN_MT_OUT2);
    BUSY    = GET_PIN(PIN_BUSY);
    D_OUT	= GET_PIN(PIN_D_OUT);
    MT_IN	= GET_PIN(PIN_MT_IN);
    MT_OUT1	= GET_PIN(PIN_MT_OUT1);
    D_IN	= GET_PIN(PIN_D_IN);
    //ACK		= GET_PIN(PIN_ACK);
    SEL2	= GET_PIN(PIN_SEL2);
    SEL1	= GET_PIN(PIN_SEL1);

    return true;
}

bool Cce120p::Set_Connector(Cbus *_bus) {
    //MT_OUT2	= GET_PIN(PIN_MT_OUT2);
    //BUSY    = GET_PIN(PIN_BUSY);
    bool extD_OUT	= pCONNECTOR_Ext->Get_pin(PIN_D_OUT);
    bool extMT_IN	= pCONNECTOR_Ext->Get_pin(PIN_MT_IN);
    //MT_OUT1	= GET_PIN(PIN_MT_OUT1);
    bool extD_IN	= pCONNECTOR_Ext->Get_pin(PIN_D_IN);
    bool extACK		= pCONNECTOR_Ext->Get_pin(PIN_ACK);
    bool extSEL2	= pCONNECTOR_Ext->Get_pin(PIN_SEL2);
    bool extSEL1	= pCONNECTOR_Ext->Get_pin(PIN_SEL1);

#if 1
    pCONNECTOR_Ext->Set_pin(PIN_MT_OUT2,MT_OUT2);
    pCONNECTOR_Ext->Set_pin(PIN_BUSY,BUSY);
    pCONNECTOR_Ext->Set_pin(PIN_D_OUT,D_OUT);
    pCONNECTOR_Ext->Set_pin(PIN_MT_IN,MT_IN);
    pCONNECTOR_Ext->Set_pin(PIN_MT_OUT1,MT_OUT1);
    pCONNECTOR_Ext->Set_pin(PIN_D_IN,D_IN);
    //pCONNECTOR_Ext->Set_pin(PIN_ACK,ACK || extACK);
    pCONNECTOR_Ext->Set_pin(PIN_SEL2,SEL2);
    pCONNECTOR_Ext->Set_pin(PIN_SEL1,SEL1);
#endif

    pCONNECTOR->Set_pin(PIN_MT_OUT2,MT_OUT2);
    pCONNECTOR->Set_pin(PIN_BUSY,BUSY);
    pCONNECTOR->Set_pin(PIN_D_OUT,D_OUT || extD_OUT);
    pCONNECTOR->Set_pin(PIN_MT_IN,MT_IN || extMT_IN);
    pCONNECTOR->Set_pin(PIN_MT_OUT1,MT_OUT1);
    pCONNECTOR->Set_pin(PIN_D_IN,D_IN || extD_IN);
    pCONNECTOR->Set_pin(PIN_ACK,ACK || extACK);
    pCONNECTOR->Set_pin(PIN_SEL2,SEL2 || extSEL2);
    pCONNECTOR->Set_pin(PIN_SEL1,SEL1 || extSEL1);


    return true;
}

bool Cce120p::run(void) {
    pCONNECTOR_Ext_value = pCONNECTOR_Ext->Get_values();

//    Get_Connector();


    Cce126::run();

    //pCONNECTOR_Ext->Set_values(pCONNECTOR->Get_values());

//    Set_Connector();

    return true;
}
