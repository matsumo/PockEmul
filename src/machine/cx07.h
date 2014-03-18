#ifndef CX07_H
#define CX07_H

#include <QTime>

class CPObject;
#include "pcxxxx.h"
class CT6834;
class Cuart;


typedef struct
 {
  UINT8 F0;
  UINT8 F1;
  UINT8 F2;
  UINT8 F3;
  UINT8 F4;
  UINT8 F5;
  UINT8 F6;
  UINT8 F7;
 } PORT_FX;

typedef struct
 {
  PORT_FX R;
  PORT_FX W;
 } PorT_FX;

enum MODE {
    RESET,
    SERIE,
    TAPE,
    BUZZER
};

typedef struct
 {
  UINT8 Buff_Key[20];
  UINT8 Nb_Key;
  UINT8 Pt_Lec;
  UINT8 Pt_Ecr;
 } CLAVIER;

#define NB_POINT_CAR_X 6
#define NB_POINT_CAR_Y 8
#define MAX_X          120
#define MAX_Y          32

#define IT_RST_A       0x3C  // Interuptions T6834
#define IT_RST_B       0x34  // Interuptions Liaisons séries et Timer
#define IT_RST_C       0x2C  // Interuptions non utilisées
#define IT_NMI         0x66  // Non Maskable Interrupt



class Cx07 : public CpcXXXX
{
    Q_OBJECT
public:
    Cx07(CPObject *parent = 0);
    virtual ~Cx07();

    virtual bool	Chk_Adr(UINT32 *d,UINT32 data);
    virtual bool	Chk_Adr_R(UINT32 *d, UINT32 *data);
    UINT8 in(UINT8 address);
    UINT8 out(UINT8 address,UINT8 value);
    bool init();

    PorT_FX Port_FX;
    void ReceiveFromT6834(UINT8 Cmd, PorT_FX *Port);
    void SendToT6834(PorT_FX *Port);

    bool Mode_K7;
    bool Mode_BUZ;
    bool Mode_SERIE;

    int IT_T6834;
    UINT8 Int_nsc800_BB;

    Cconnector *pPARConnector;    qint64      pPARConnector_value;
    Cconnector *pSERConnector;    qint64      pSERConnector_value;

    CT6834  *pT6834;
    Cuart    *pUART;
    bool run();
    void AddKey(UINT8 Key);

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    void Reset();

    int soundTP;

    int Cpt;
    int Lng_Cmd;
    int Lng_Rsp;

    UINT8 Rsp[0x80];
    int  pt;
    int  lng_rsp;
    UINT8 Ordre;

    int Nb;
    int Lec_K7;
    void AddFKey(UINT8 F_Key);

    void TurnON();
    void TurnOFF();
    bool SaveConfig(QXmlStreamWriter *xmlOut);
    bool LoadConfig(QXmlStreamReader *xmlIn);
    void Print(UINT8 Cmd, PorT_FX *Port);
    void SendToSerial(PorT_FX *Port);

    void ReceiveFromSerial(PorT_FX *Port);
    void manageSound();
    void Send_to_K7(PorT_FX *Port);
    void Receive_from_K7(PorT_FX *Port);
    bool Presence_k7;
    QFile Fichier_k7; /* Fichier de k7 */

public slots:
    void LoadK7(void);
    void LoadNewK7(void);
    void TurnOFFSlot(void);

protected slots:
    void contextMenuEvent ( QContextMenuEvent * );

};

#endif // CX07_H
