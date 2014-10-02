#ifndef CT6834_H
#define CT6834_H

#include <Qt>
#include "pobject.h"
#include "cx07.h"

typedef struct
 {
  int size_point_x;
  int size_point_y;
  UINT8 Scroll_Min_Y;
  UINT8 Scroll_Max_Y;
  UINT8 Curs_X;
  UINT8 Curs_Y;
  bool Curseur;
  UINT8 Aff_Udk;
  UINT8 Stick;
  UINT8 Strig;
  UINT8 Strig1;
  FILE *K7_File;
  UINT8 Baud_Out_Mode;
  UINT8 Rem_Canal;
  UINT8 Break;
  bool EnableKeyEntry;
  bool LcdOn;

 } GENERAL_INFO;


typedef struct
 {
  UINT8 lng_send;
  UINT8 lng_rsp;
  char *Str_Cmd;
 } CMD_T6834;

class CT6834 : public CPObject
{
    Q_OBJECT
public:
    CT6834(CPObject *parent = 0);
    virtual ~CT6834();

    bool init(void);
    void Reset(void);
    static const int udk_ofs[12];
    static const int udk_size[12];
    static const CMD_T6834 Cmd_T6834[0x48];

    int InitReponseT6834(UINT8 Ordre, UINT8 *Rsp, PorT_FX *Port);
    void AffUdkON(bool State);
    void ClrScr();
    void RefreshVideo();
    void AffCurseur();
    void AffCar(UINT8 x, UINT8 y, UINT8 Car);
    void ScrollVideo();
    void LineClear(UINT8 P_y);
    void Pset(int x, int y);
    void Preset(int x, int y);
    void Line(UINT8 x1, UINT8 y1, UINT8 x2, UINT8 y2);
    void Circle(int x, int y, int r);

    void initUdk();
    void save_internal(QXmlStreamWriter *xmlOut);
    void Load_Internal(QXmlStreamReader *xmlIn);

    void AddFKey(UINT8 F_Key);
    void AddKey(UINT8 Key);

    void keyPress(QKeyEvent *event);
    void keyRelease(QKeyEvent *event);
    void TurnOFF();

    GENERAL_INFO General_Info;
    UINT8 Send_Cmd_T6834 [0x200];
    UINT8 Locate_OnOff;
    UINT8 Loc_X;
    UINT8 Loc_Y;

    UINT8 Loc_x,Loc_y;

    UINT8 R5;
    Cx07 *pPC;
    UINT8 Ram_Video[120][32];
    void setRamVideo(int x, int y, UINT8 val);
    QTime cursorTimer;
    UINT8 First;

    static const UINT8 key_tbl[256];
    static const UINT8 key_tbl_c[256];
    static const UINT8 key_tbl_s[256];
    static const UINT8 key_tbl_k[256];
    static const UINT8 key_tbl_ks[256];
    static const UINT8 key_tbl_g[256];

    UINT8 mem[0x2200];

    QByteArray Clavier;
    quint8 LastKey;

    bool kana,graph,shift,ctrl;
    bool curOnOff;

    quint8 getKey(quint16 strobe);
signals:
    void TurnOFFSig();

};

#endif // CT6834_H
