// FIXME: UDKRead broken

#include <math.h>

#include <QTime>
#include <QPainter>
#include <QKeyEvent>
#include <QDebug>

#include "ct6834.h"
#include "Lcdc.h"
#include "Log.h"
#include "cpu.h"
#include "cx07.h"
#include "cx07char.h"
#include "Keyb.h"


CT6834::CT6834(CPObject *parent)	: CPObject(parent)
{
    pPC = (Cx07*)parent;
    Locate_OnOff = 0;
    Loc_X=0;
    Loc_Y=0;
    R5 = 0;
    First = 1;
    curOnOff = false;

    kana=graph=shift=ctrl = false;

}

CT6834::~CT6834(){
}

const CMD_T6834 CT6834::Cmd_T6834[] =
{{   1,   0,"Unknown"},          // 0x00
 {   1,   8,"TimeCall"},         // 0x01
 {   1,   1,"Stick"},            // 0x02
 {   1,   1,"Strig"},            // 0x03
 {   1,   1,"Strig1"},           // 0x04
 {   3,   1,"RamRead"},          // 0x05
 {   4,   0,"RamWrite"},         // 0x06
 {   3,   0,"ScrollSet"},        // 0x07
 {   1,   0,"ScrollExet"},       // 0x08
 {   2,   0,"LineClear"},        // 0x09
 {   9,   0,"TimeSet"},          // 0x0A
 {   1,   0,"CalcDay"},          // 0x0B
 {   9,   0,"AlarmSet"},         // 0x0C
 {   1,   0,"BuzzerOff"},        // 0x0D
 {   1,   0,"BuzzerOn"},         // 0x0E
 {   2, 120,"TrfLine"},          // 0x0F
 {   3,   1,"TestPoint"},        // 0x10
 {   3,   0,"Pset"},             // 0x11
 {   3,   0,"Preset"},           // 0x12
 {   3,   0,"Peor"},             // 0x13
 {   5,   0,"Line"},             // 0x14
 {   4,   0,"Circle"},           // 0x15
 {0x82,   0,"UDKWrite"},         // 0x16
 {   2,0x80,"UDKRead"},          // 0x17
 {   1,   0,"UDKOn"},            // 0x18
 {   1,   0,"UDKOff"},           // 0x19
 {  10,   0,"UDCWrite"},         // 0x1A
 {   2,   8,"UDCRead"},          // 0x1B
 {   1,   0,"UDCInt"},           // 0x1C
 {0x81,   0,"StartPgmWrite"},    // 0x1D
 {0x81,   0,"SPWriteCont"},      // 0x1E
 {   1,   0,"SPOn"},             // 0x1F
 {   1,   0,"SPOff"},            // 0x20
 {   1,0x80,"StartPgmRead"},     // 0x21
 {   1,   1,"OnStat"},           // 0x22
 {   1,   0,"OFFReq"},           // 0x23
 {   4,   0,"Locate"},           // 0x24
// {   38,   0,"Locate"},           // 0x24
 {   1,   0,"CursOn"},           // 0x25
 {   1,   0,"CursOff"},          // 0x26
 {   3,   1,"TestKey"},          // 0x27
 {   2,   1,"TestChr"},          // 0x28
 {   1,   0,"InitSec"},          // 0x29
 {   2,   0,"InitDate"},         // 0x2A
 {   1,   0,"ScrOff"},           // 0x2B
 {   1,   0,"ScrOn"},            // 0x2C
 {   1,   0,"KeyBufferClear"},   // 0x2D
 {   1,   0,"ClsScr"},           // 0x2E
 {   1,   0,"Home"},             // 0x2F
 {   1,   0,"AffUdkOn"},         // 0x30
 {   1,   0,"AffUDKOff"},        // 0x31
 {   1,   0,"RepateKeyOn"},      // 0x32
 {   1,   0,"RepateKeyOff"},     // 0x33
 {   1,   0,"UDK=KANA"},         // 0x34
 {0x82,   0,"UdkContWrite"},     // 0x35
 {   1,   8,"AlarmRead"},        // 0x36
 {   1,   1,"BuzzZero"},         // 0x37
 {   1,   0,"ClickOff"},         // 0x38
 {   1,   0,"ClickOn"},          // 0x39
 {   1,   0,"LocateClose"},      // 0x3A
 {   1,   0,"KeybOn"},           // 0x3B
 {   1,   0,"KeybOff"},          // 0x3C
 {   1,   0,"ExecStartPgm"},     // 0x3D
 {   1,   0,"UnexecStartPgm"},   // 0x3E
 {   1,   0,"Sleep"},            // 0x3F
 {   1,   0,"UDKInit"},          // 0x40
 {   9,   0,"AffUDC"},           // 0x41
 {   1,   8,"ReadCar"},          // 0x42
 {   3,   2,"ScanR"},            // 0x43
 {   3,   2,"ScanL"},            // 0x44
 {   1,   1,"TimeChk"},          // 0x45
 {   1,   1,"AlmChk"}};          // 0x46


const int CT6834::udk_ofs[12] = {
    0, 42, 84, 126, 168, 210, 256, 298, 340, 382, 424, 466
};

const int CT6834::udk_size[12] = {
    42, 42, 42, 42, 42, 46, 42, 42, 42, 42, 42, 46
};
//extern FILE */*fp_tm*/p3;
int CT6834::InitReponseT6834 (UINT8 Ordre, UINT8 *Rsp, PorT_FX *Port)
{
    int    Lng_rsp;
    UINT32  Adresse;
    UINT8  i;

    Lng_rsp = Cmd_T6834[Ordre].lng_rsp;

    AddLog (LOG_TEMP,tr("*** ORDRE = %1").arg(Ordre,2,16,QChar('0')));
//    if (fp_tmp3)  fprintf(fp_tmp3,"ORDER %02X\n",Ordre);
 switch (Ordre & 0x7F)
  {
   case 0x00: //lng_rsp = Cmd_T6834[Ordre].lng_rsp;
              break;

   case 0x01: {
                  QDateTime my_t = QDateTime::currentDateTime();
                  Rsp[0] = ((my_t.date().year()-100)>>8) & 0xFF;
                  Rsp[1] = my_t.date().year() & 0xFF;
                  Rsp[2] = my_t.date().month();
                  Rsp[3] = my_t.date().day();
                  Rsp[4] = ~(((0x01 << (7 - my_t.date().dayOfWeek())) - 1) & 0xff);
                  Rsp[5] = my_t.time().hour();
                  Rsp[6] = my_t.time().minute();
                  Rsp[7] = my_t.time().second();
              }
              break;

   case 0x02: AddLog (LOG_TEMP,tr("Stick = %1").arg(General_Info.Stick,2,16,QChar('0')));
     switch (General_Info.Stick) {
         case 0x00: Rsp[0] = 0x30; break;
         case 0x01: Rsp[0] = 0x31; break;
         case 0x03: Rsp[0] = 0x32; break;
         case 0x02: Rsp[0] = 0x33; break;
         case 0x06: Rsp[0] = 0x34; break;
         case 0x04: Rsp[0] = 0x35; break;
         case 0x0C: Rsp[0] = 0x36; break;
         case 0x08: Rsp[0] = 0x37; break;
         case 0x09: Rsp[0] = 0x38; break;
     default: Rsp[0] = 0x30; break;
     }
              break;

   case 0x03: AddLog (LOG_TEMP,tr("Strig = %1").arg(General_Info.Strig,2,16,QChar('0')));
              Rsp[0] = General_Info.Strig;
              Clavier.clear();
              break;

   case 0x04: AddLog(LOG_TEMP,tr("Strig1 = %1").arg(General_Info.Strig1,2,16,QChar('0')));
              Rsp[0] = General_Info.Strig1;
              Clavier.clear();
              break;

   case 0x05: // RamRead
              Adresse = Send_Cmd_T6834[1] + ( Send_Cmd_T6834[2] << 8);
              Adresse |= 0xC000;
              if (pPC->pCPU->fp_log) fprintf(pPC->pCPU->fp_log,"Lecture adr %04X",Adresse);
              AddLog(LOG_CANON,tr("Lecture adr %1").arg(Adresse,4,16,QChar('0')));
              if(Adresse == 0xc00e) {
                  Rsp[0] = 0x0A;
              }
              else
              if(Adresse == 0xd000) {
                  Rsp[0] = 0x30;
              }
              else
              {

                  Rsp[0] = mem[Adresse - 0xC000];
              }
              break;

   case 0x06: // RamWrite
     Adresse = Send_Cmd_T6834[2] + ( Send_Cmd_T6834[3] << 8);
//     if (fp_tmp3) fprintf(fp_tmp3,"Ecriture adr %04X : %02X\n",Adresse,Send_Cmd_T6834[1]);
              if (pPC->pCPU->fp_log) fprintf(pPC->pCPU->fp_log,"Ecriture adr %04X : %02X",Adresse,Send_Cmd_T6834[1]);
              AddLog(LOG_CANON,tr("Ecriture adr [%1]=%2").arg(Adresse,4,16,QChar('0')).arg(Send_Cmd_T6834[1],2,16,QChar('0')));
              Adresse |= 0xC000;
              mem[Adresse - 0xC000] = Send_Cmd_T6834[1];
              if(Adresse == 0xc00e) {
                  AddLog(LOG_TEMP,tr("RAM WRITE C00E %1").arg(Rsp[0],2,16,QChar('0')));
              }
              break;

   case 0x07: // Scroll Set
                General_Info.Scroll_Min_Y = Send_Cmd_T6834[1];
                General_Info.Scroll_Max_Y = Send_Cmd_T6834[2]+1;
              break;

   case 0x08:
              pPC->pLCDC->updated = true;
              ScrollVideo ();
              break;

   case 0x09:
              pPC->pLCDC->updated = true;
              LineClear (Send_Cmd_T6834[1]);
              break;

   case 0x0A:
              fprintf (stderr,"Time set:%d/%d/%d (%d) %d:%d:%d\n",(Send_Cmd_T6834[1]<<8)+
                                                                   Send_Cmd_T6834[2],
                                                                   Send_Cmd_T6834[3],
                                                                   Send_Cmd_T6834[4],
                                                                   Send_Cmd_T6834[5],
                                                                   Send_Cmd_T6834[6],
                                                                   Send_Cmd_T6834[7],
                                                                   Send_Cmd_T6834[8]);
   case 0x0B: // Calc Day (not used)
              break;

   case 0x0C: // AlarmSet (not used)
              fprintf (stderr,"AlarmSet:[%d %d]%d/%d/%d (%d) %d:%d:%d\n",
                                                                   Send_Cmd_T6834[1],
                                                                   Send_Cmd_T6834[2],
                                                                  (Send_Cmd_T6834[1]<<8)+
                                                                   Send_Cmd_T6834[2],
                                                                   Send_Cmd_T6834[3],
                                                                   Send_Cmd_T6834[4],
                                                                   Send_Cmd_T6834[5],
                                                                   Send_Cmd_T6834[6],
                                                                   Send_Cmd_T6834[7],
                                                                   Send_Cmd_T6834[8]);
              break;
  case 0x0d:
  case 0x0e:
  case 0x0f:
  {
     // TrfLine
             int sy = Send_Cmd_T6834[1];
             for(i = 0; i < 120; i++) {
                 if(sy < 32) {
                     Rsp[i] = Ram_Video[i][sy];
                 }
                 else {
                     Rsp[i] = 0;
                 }
             }
  }
      break;

  case 0x10:    //POINT
     {
         int sx = Send_Cmd_T6834[1];
         int sy = Send_Cmd_T6834[2];
         if(sx < 120 && sy < 32) {
             Rsp[0] = Ram_Video[sx][sy] ? 0x00 : 0xff;

         }
         else {
             Rsp[0] = 0;
         }
     }
            break;

   case 0x11: pPC->pLCDC->updated = true;
              Pset (Send_Cmd_T6834[1],Send_Cmd_T6834[2]);
              break;

   case 0x12: pPC->pLCDC->updated = true;
              Preset (Send_Cmd_T6834[1],Send_Cmd_T6834[2]);
              break;
  case 0x13:	// Peor
              pPC->pLCDC->updated = true;
              if(Send_Cmd_T6834[1] < 120 && Send_Cmd_T6834[2] < 32) {
                  setRamVideo(Send_Cmd_T6834[1],Send_Cmd_T6834[2], ~(Ram_Video[Send_Cmd_T6834[1]][Send_Cmd_T6834[2]]));
              }
              break;

   case 0x14: // Line (x1,y1)-(x2,y2)
              pPC->pLCDC->updated = true;
              Line (Send_Cmd_T6834[1],Send_Cmd_T6834[2],
                    Send_Cmd_T6834[3],Send_Cmd_T6834[4]);
              break;
  case 0x15: pPC->pLCDC->updated = true;
             Circle(Send_Cmd_T6834[1],Send_Cmd_T6834[2],Send_Cmd_T6834[3]);
             break;

  case 0x16: // UDKWrite
      for(i = 0; i < udk_size[Send_Cmd_T6834[1]]; i++) {
          mem[0x800+udk_ofs[Send_Cmd_T6834[1]] + i] = Send_Cmd_T6834[2+i];
      }
      break;

  case 0x17:	// UDKRead
                  //val = Send_Cmd_T6834[1];
                  for(i = 0; i < 42; i++) {
                      UINT8 code = mem[0x800+udk_ofs[Send_Cmd_T6834[1]] + i];//General_Info.F_Key [Send_Cmd_T6834[1]-1][i];
                      qWarning()<<"udkread("<<i<<"="<<code;
                      Rsp[i] = code;
                      if(!code) {
                          return (i+1);
                      }
                  }
                  return (i+1);
                  break;
   case 0x18: // UDKOn
   case 0x19: // UDKOff
                 pPC->pLCDC->updated = true;
                 break;

   case 0x1A: // UDCWrite
              for (i=0;i<8;i++)
               X07_CarDef [Send_Cmd_T6834[1]][i] = Send_Cmd_T6834[2+i];
              break;

   case 0x1B: // UDCRead
                for (i=0;i<8;i++)
                    Rsp[i] = X07_CarDef [Send_Cmd_T6834[1]][i];
              break;

  case 0x1d:
  case 0x1e:
      for(i = 0; (i < 0x100 && Send_Cmd_T6834[i+1]); i++) {
          mem[0xE00+i] = Send_Cmd_T6834[12+i];
      }

      break;
   case 0x1F: // SPOn
   case 0x20: // SPOff
                 break;
   case 0x21: // SP Read  0xCE00 -> 0xCFFF
      for(int i = 0; i < 0x200; i++) {
          UINT8 code = mem[0xE00+i];
          Rsp[i] = code;
          if(!code) {
              return (i+1);
          }
      }
      return (i+1);
      break;

   case 0x22: Rsp[0]=mem[0x0006];//0x04; //  | (m_sleep<<6) | m_warm_start; // MEM 0xC006
                // teste le bit 6 de A. tenter de le mettre à 1

              break;
   case 0x23: // Turn OFF
      pPC->pLCDC->updated = true;
      TurnOFF();

              break;

   case 0x24:
              pPC->pLCDC->updated = true;
              AddLog (LOG_TEMP,tr("Locate %1,%2 ").arg(Send_Cmd_T6834[1],2,16,QChar('0')).arg(Send_Cmd_T6834[2],2,16,QChar('0')));

              if ((Loc_X == Send_Cmd_T6834[1]) && (Loc_Y == Send_Cmd_T6834[2]))
                {
                    AddLog (LOG_TEMP,"Locate_OnOff = 0\n");
                    Locate_OnOff = 0;
                }
               else
                {
                    AddLog (LOG_TEMP,"Locate_OnOff = 1\n");
                    Locate_OnOff = 1;
                }
              Loc_X = General_Info.Curs_X = Send_Cmd_T6834[1];
              Loc_Y = General_Info.Curs_Y = Send_Cmd_T6834[2];
              if (Send_Cmd_T6834[3])
              {
                  AffCar (Send_Cmd_T6834[1],Send_Cmd_T6834[2],Send_Cmd_T6834[3]);
              }
              else
              {
                  //pPC->AffCurseur();
              }

              break;

   case 0x25: // CursOn
                pPC->pLCDC->updated = true;
                AddLog (LOG_TEMP,"Curseur ON\n");

              General_Info.Curseur = true;
              break;

   case 0x26: // CursOff
            pPC->pLCDC->updated = true;
              AddLog (LOG_TEMP,"Curseur OFF\n");

              General_Info.Curseur = false;
              break;
  case 0x27 : {
     UINT16 matrix;
     UINT8 data = 0;
     matrix = Send_Cmd_T6834[1] | (Send_Cmd_T6834[2] << 8);
//     qWarning()<<"matrix="<<matrix;
     Rsp[0] = getKey(matrix);
 }
      /*
  {
      static const char *const lines[] = {"S1", "S2", "S3", "S4", "S5", "S6", "S7", "S8", "BZ", "A1"};
      UINT16 matrix;
      UINT8 data = 0;
      matrix = m_in.data[m_in.read++];
      matrix |= (m_in.data[m_in.read++] << 8);

      for (int i=0 ;i<10; i++)
          if (matrix & (1<<i))
              data |= input_port_read(machine, lines[i]);

      m_out.data[m_out.write++] = data;
  }
  */
      break;
  case 0x28:	//test chr
     if (Clavier.contains(TOUPPER(Send_Cmd_T6834[1]))) {
         Rsp[0] = 0;
         Clavier.clear();
     }
     else Rsp[0] = 0xFF;

      break;

  case 0x29:	//init sec
  case 0x2a:	//init date
      break;

   case 0x2b: // LCD OFF
                pPC->pLCDC->updated = true;
                General_Info.LcdOn = false;
                break;
   case 0x2c: // LCD ON
                pPC->pLCDC->updated = true;
                General_Info.LcdOn = true;
                break;
   case 0x2D: // KeyBufferClear
      //memset(m_t6834_ram + 0x400, 0, 0x100);
              Clavier.clear();
              break;

   case 0x30: // UDKOn
                pPC->pLCDC->updated = true;
              General_Info.Aff_Udk = 1;
              AffUdkON (0);
              break;
   case 0x31: // UDKOff
                pPC->pLCDC->updated = true;
              General_Info.Aff_Udk = 0;
              LineClear (3);
              break;
    case 0x37: Rsp[0]=0;
              break;
   case 0x3b: // KeybOn
                General_Info.EnableKeyEntry = true;
                break;
   case 0x3c: // KeybOff
                General_Info.EnableKeyEntry = false;
                 break;

   case 0x3F: // Sleep
                pPC->pLCDC->updated = true;
              AddLog (LOG_TEMP,"Sleep\n");
              mem[0x0006] |= 0x41;      // bit 0 and 6 to 1 for OFF and SLEEP
              General_Info.LcdOn = false;
                pPC->TurnOFF();
                 break;
  case 0x40:
        initUdk();
      break;
  case 0x41:	//char write
      {
//          for(int cy = 0; cy < 8; cy++)
//          {
//              UINT8 cl = m_in.data[m_in.read++];

//              for(int cx = 0; cx < 6; cx++)
//                  m_lcd_map[m_cursor.y * 8 + cy][m_cursor.x * 6 + cx] = (cl & (0x80>>cx)) ? 1 : 0;
//          }
      }
      break;

  case 0x42: //char read
      {
          for(int cy = 0; cy < 8; cy++)
          {
              Rsp[cy] = 0;
//              UINT8 cl = 0x00;

//              for(int cx = 0; cx < 6; cx++)
//                  cl |= (m_lcd_map[m_cursor.y * 8 + cy][m_cursor.x * 6 + cx] != 0) ? (1<<(7-cx)) : 0;

//              m_out.data[m_out.write++] = cl;
          }
      }
      break;
  case 0x43:	// ScanR
  case 0x44:	// ScanL
      Rsp[0]=0;
      Rsp[1]=0;
      break;
  case 0x45:	// TimeChk
  case 0x46:	// AlmChk
      Rsp[0]=0;
      break;

   default:    fprintf (stderr,"(%s) \n",Cmd_T6834[Send_Cmd_T6834[0]].Str_Cmd);
               break;

  }
 return (Lng_rsp);
}

void CT6834::initUdk() {
    strcpy((char*)&mem[0x800+udk_ofs[0]],"tim?TIME$\r");
    strcpy((char*)&mem[0x800+udk_ofs[1]],"cldCLOAD");
    strcpy((char*)&mem[0x800+udk_ofs[2]],"locLOCATE");
    strcpy((char*)&mem[0x800+udk_ofs[3]],"lstLIST");
    strcpy((char*)&mem[0x800+udk_ofs[4]],"runRUN\r");
    strcpy((char*)&mem[0x800+udk_ofs[5]],"nul");
    strcpy((char*)&mem[0x800+udk_ofs[6]],"dat?DATE$\r");
    strcpy((char*)&mem[0x800+udk_ofs[7]],"csaCSAVE");
    strcpy((char*)&mem[0x800+udk_ofs[8]],"prtPRINT");
    strcpy((char*)&mem[0x800+udk_ofs[9]],"slpSLEEP");
    strcpy((char*)&mem[0x800+udk_ofs[10]],"cntCONT\r");
    strcpy((char*)&mem[0x800+udk_ofs[11]],"nul");
}

void CT6834::AffUdkON (bool shift)
{
    qint8 Offset,i,j;
    qint8 x=0;

    Offset = (shift)?1:0;
    for (i=0;i<5;i++)
    {
        AffCar (x++,3,131);
        for (j=0;j<3;j++)
            AffCar (x++,3,mem[0x800+udk_ofs[i+6*Offset]+j]);//General_Info.F_Key[i+(6*Offset)][j]);
    }
}

void CT6834::ClrScr (void)
{
    memset(&Ram_Video,0,sizeof(Ram_Video));
//    RefreshVideo ();
    pPC->Refresh_Display = true;
}

#define PIXEL_SIZE 4
#define PIXEL_GAP 1

void CT6834::RefreshVideo (void)
{
    int x;
    int y;

if (!General_Info.LcdOn) return;
    //AffCurseur ();
    if (cursorTimer.elapsed()>500) {
//        qWarning()<<"Cursor";
        curOnOff = !curOnOff;
        pPC->pLCDC->updated = true;
        cursorTimer.restart();
    }
    if (!pPC->pLCDC->updated) return;
    pPC->pLCDC->updated = false;
    if (!pPC->pLCDC->LcdImage) return;
    QPainter painter;
    painter.begin(pPC->pLCDC->LcdImage);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    for (x=0;x<120;x++)
        for (y=0;y<32;y++)
        {
            if (General_Info.Curseur &&
                curOnOff &&
                General_Info.Curs_X == (x/6) &&
                General_Info.Curs_Y == (y/8)) {
                QColor col = (General_Info.LcdOn &&
                              (y == General_Info.Curs_Y * 8 + 6) &&
                              (x!=General_Info.Curs_X*6+5)) ?
                                 pPC->pLCDC->Color_On :
                                 pPC->pLCDC->Color_Off;
                painter.setPen( col );
                painter.setBrush( col );
//                painter.drawPoint(x,y);
                painter.drawRect((x)*(PIXEL_SIZE+PIXEL_GAP),
                                 (y)*(PIXEL_SIZE+PIXEL_GAP),
                                 PIXEL_SIZE-1,
                                 PIXEL_SIZE-1);
            }
            else {
                QColor col = (General_Info.LcdOn && Ram_Video[x][y])?
                                    pPC->pLCDC->Color_On : pPC->pLCDC->Color_Off;

                painter.setPen(  col  );
                painter.setBrush(  col  );
//                painter.drawPoint(x,y);
                painter.drawRect((x)*(PIXEL_SIZE+PIXEL_GAP),
                                 (y)*(PIXEL_SIZE+PIXEL_GAP),
                                 PIXEL_SIZE-1,
                                 PIXEL_SIZE-1);
            }
        }
//    if (General_Info.Curseur && cursorTimer.elapsed()>1000) cursorTimer.restart();
    painter.end();


    pPC->pLCDC->Refresh = true;
    Refresh_Display = true;
    pPC->update();
}

void CT6834::AffCurseur (void)
{
    if (!First)
    {
        if (General_Info.Curseur)
        {
            UINT8 y = ((Loc_Y+1) * General_Info.size_point_y * NB_POINT_CAR_Y) - General_Info.size_point_y;
            UINT8 x =   Loc_X    * General_Info.size_point_x * NB_POINT_CAR_X;

            for (int i=0;i<=NB_POINT_CAR_X*General_Info.size_point_x;i++)
                setRamVideo(x+i,y-1,0xff);

        }
    }
    else First = 0;

    Loc_X = General_Info.Curs_X;
    Loc_Y = General_Info.Curs_Y;
    if (General_Info.Curseur)
    {
        UINT8 y = ((Loc_Y+1) * General_Info.size_point_y * NB_POINT_CAR_Y) - General_Info.size_point_y;
        UINT8 x =   Loc_X    * General_Info.size_point_x * NB_POINT_CAR_X;
        for (int i=0;i<=NB_POINT_CAR_X*General_Info.size_point_x;i++)
            setRamVideo(x+i,y-1, 0xff);
    }
}

void CT6834::AffCar(UINT8 x, UINT8 y, UINT8 Car)
{
    int P_x,P_y;
    UINT8 Mask;
    int offsetX = x*NB_POINT_CAR_X;
    int offsetY = y*NB_POINT_CAR_Y;
    /* Dessin du caractere point par point */
    /*-------------------------------------*/

    AddLog (LOG_TEMP,tr("Draw char (%1) at %2,%3").arg(Car,2,16,QChar('0')).arg(x).arg(y));
    for (P_y=0;P_y<8;P_y++)
    {
        Mask=0x80;


        for (P_x=0;P_x<6;P_x++)
        {
            int color = ( (X07_CarDef[Car][P_y] & Mask) ? 0xff : 0);

            /* Positionnement de la mémoire video */
            /*------------------------------------*/
            setRamVideo(offsetX+P_x,offsetY+P_y, color);

            Mask >>=1;
        }
    }


}

void CT6834::ScrollVideo (void)
{
    UINT8 x,y;

    for (x=0 ; x<MAX_X ; x++)
        for (y = (General_Info.Scroll_Min_Y * NB_POINT_CAR_Y);
             y < (General_Info.Scroll_Max_Y * NB_POINT_CAR_Y);
             y++)
            if (y<((General_Info.Scroll_Max_Y - 1)*NB_POINT_CAR_Y))
                setRamVideo(x,y, Ram_Video[x][y+8]);
            else
                setRamVideo(x,y,0);
//    RefreshVideo ();
    pPC->Refresh_Display=true;
}

void CT6834::LineClear (UINT8 P_y)
{
 UINT8 x,y;
// FIXME: Wat is the meaning of P_y=0xff ???? clear full screen ?
 //qWarning()<<"P_Y="<<P_y;
  if (P_y==0xff) P_y=0;

 /* Effacement de la mémoire video */
 /*--------------------------------*/
 for (x=0;x<MAX_X;x++)
  for (y=P_y*NB_POINT_CAR_Y;y<(P_y+1)*NB_POINT_CAR_Y;y++)
   setRamVideo(x,y,0);
}

/*---------------------------------------------------------------------------*/
void CT6834::Pset (int x, int y)
{
#if AFF_CMD_T6834
    fprintf (stderr,"Pset %d,%d ",x,y);
#endif
    if ( x>=0 && x <120 && y>=0 && y<32)
        setRamVideo(x,y,0xff);

}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                                                                           */
/*---------------------------------------------------------------------------*/
void CT6834::Preset (int x, int y)
{
#if AFF_CMD_T6834
    fprintf (stderr,"Preset %d,%d ",x,y);
#endif
    setRamVideo(x,y,0);
}

void CT6834::Line (UINT8 x1, UINT8 y1, UINT8 x2, UINT8 y2)
{
    int next_x = x1, next_y = y1;
    int delta_x = abs(x2 - x1) * 2;
    int delta_y = abs(y2 - y1) * 2;
    int step_x = (x2 < x1) ? -1 : 1;
    int step_y = (y2 < y1) ? -1 : 1;

    if(delta_x > delta_y) {
        int frac = delta_y - delta_x / 2;
        while(next_x != x2) {
            if(frac >= 0) {
                next_y += step_y;
                frac -= delta_x;
            }
            next_x += step_x;
            frac += delta_y;
            Pset(next_x, next_y);
        }
    }
    else {
        int frac = delta_x - delta_y / 2;
        while(next_y != y2) {
            if(frac >= 0) {
                next_x += step_x;
                frac -= delta_y;
            }
            next_y += step_y;
            frac += delta_x;
            Pset(next_x, next_y);
        }
    }
    Pset(x1, y1);
    Pset(x2, y2);
}

void CT6834::Circle(int x, int y, int r)
{

// high accuracy
    double xlim = sqrt((double)(r * r) / 2);

    for(int cx = 0, cy = r; cx <= xlim+1 ; cx++) {
        double d1 = (cx * cx + cy * cy) - r * r;
        double d2 = (cx * cx + (cy - 1) * (cy - 1)) - r * r;
        if(abs(d1) > abs(d2)) {
            cy--;
        }
        Pset(cx + x, cy + y);
        Pset(cx + x, -cy + y);
        Pset(-cx + x, cy + y);
        Pset(-cx + x, -cy + y);
        Pset(cy + x, cx + y);
        Pset(cy + x, -cx + y);
        Pset(-cy + x, cx + y);
        Pset(-cy + x, -cx + y);
    }

}


void CT6834::Load_Internal(QXmlStreamReader *xmlIn)
{
    if (xmlIn->readNextStartElement()) {
        if ( (xmlIn->name()=="cpu") &&
             (xmlIn->attributes().value("model").toString() == "t6834")) {
//            QByteArray ba_reg = QByteArray::fromBase64(xmlIn->attributes().value("registers").toString().toAscii());
//            memcpy((char *) &r,ba_reg.data(),sizeof(r));
            QByteArray ba_mem = QByteArray::fromBase64(xmlIn->attributes().value("Mem").toString().toLatin1());
            memcpy((char *)&mem,ba_mem.data(),sizeof(mem));
            QByteArray ba_lcd = QByteArray::fromBase64(xmlIn->attributes().value("Lcd").toString().toLatin1());
            memcpy((char *)&Ram_Video,ba_lcd.data(),MIN(ba_lcd.size(),sizeof(Ram_Video)));
        }
        xmlIn->skipCurrentElement();
    }
}

void CT6834::save_internal(QXmlStreamWriter *xmlOut)
{
    xmlOut->writeStartElement("cpu");
        xmlOut->writeAttribute("model","t6834");
//        QByteArray ba_reg((char*)&r,sizeof(r));
//        xmlOut->writeAttribute("registers",ba_reg.toBase64());
        QByteArray ba_mem((char*)&mem,sizeof(mem));
        xmlOut->writeAttribute("Mem",ba_mem.toBase64());
        QByteArray ba_lcd((char*)&Ram_Video,sizeof(Ram_Video));
        xmlOut->writeAttribute("Lcd",ba_lcd.toBase64());
    xmlOut->writeEndElement();
}

bool CT6834::init()
{


    Reset();

    connect(this,SIGNAL(TurnOFFSig()),pPC,SLOT(TurnOFFSlot()));

    cursorTimer.start();

    return true;
}

void CT6834::Reset()
{
    // Ram_Video should be integrated into mem.
    memset((void*)&Ram_Video,0,sizeof (Ram_Video));
    memset(mem,0,0x2200);
    initUdk();

    General_Info.Scroll_Min_Y = 0;
    General_Info.Scroll_Max_Y = 4;
    General_Info.size_point_x = 1;
    General_Info.size_point_y = 1;
    General_Info.Curs_X       = 0;
    General_Info.Curs_Y       = 0;
    General_Info.Curseur      = false;
    General_Info.Aff_Udk      = 0;
    General_Info.Rem_Canal    = 0;
    General_Info.Stick        = 0x00;
    General_Info.Strig        = 0xFF;
    General_Info.Strig1       = 0xFF;
    General_Info.Break        = 0;
    General_Info.EnableKeyEntry = true;
    General_Info.LcdOn        = false;
}

void CT6834::AddKey (UINT8 Key)
{
    Clavier.append(Key);
    LastKey = Key;
}

void CT6834::AddFKey (UINT8 F_Key)
{

    if (F_Key < 12)
        for (int i=3;(i<udk_size[i]) && mem[0x800+udk_ofs[F_Key]+i];i++)
            AddKey(mem[0x800+udk_ofs[F_Key]+i]);
}

void CT6834::keyRelease(QKeyEvent *event)
{

    kana=graph=shift=ctrl = false;

//    switch (event->modifiers()) {
//        case Qt::ShiftModifier : shift = true; break;
//        case Qt::AltModifier:   graph = true; break;
//        case Qt::ControlModifier: ctrl = true; break;
//    }

//    if(General_Info.Aff_Udk) {
//        AffUdkON(shift);
//    }

    switch(event->key()) {
    case Qt::Key_Backspace:	// bs->left
        General_Info.Stick = 0x00;
        break;

    case Qt::Key_Space:
        General_Info.Strig1 = 0xff;
        break;
    case Qt::Key_Up    : General_Info.Stick &= ~0x01; break;
    case Qt::Key_Right : General_Info.Stick &= ~0x02; break;
    case Qt::Key_Down  : General_Info.Stick &= ~0x04; break;
    case Qt::Key_Left  : General_Info.Stick &= ~0x08; break;
    case Qt::Key_F6:	// F6
        General_Info.Strig = 0xff;
        break;
    }
//    while (Clavier.indexOf(event->key())>=0) {
//        Clavier.remove(Clavier.indexOf(event->key()),1);
//    }
    event->accept();
}

void CT6834::keyPress(QKeyEvent *event)
{
    UINT8 code,val;

General_Info.Strig = 0;

    switch (event->modifiers()) {
    case Qt::ShiftModifier : switch (event->key()) {
        case Qt::Key_F1    : AddFKey (6);break;
        case Qt::Key_F2    : AddFKey (7);break;
        case Qt::Key_F3    : AddFKey (8);break;
        case Qt::Key_F4    : AddFKey (9);break;
        case Qt::Key_F5    : AddFKey (10);break;
        case Qt::Key_F6    : AddFKey (11);break;
        } break;
    default:  switch (event->key()) {
        case Qt::Key_F1    : AddFKey (0);break;
        case Qt::Key_F2    : AddFKey (1);break;
        case Qt::Key_F3    : AddFKey (2);break;
        case Qt::Key_F4    : AddFKey (3);break;
        case Qt::Key_F5    : AddFKey (4);break;
        case Qt::Key_F6    : General_Info.Strig = 0x00; AddFKey (5);break;
        }
    }

    switch (event->key()) {
    case Qt::Key_F1    :
    case Qt::Key_F2    :
    case Qt::Key_F3    :
    case Qt::Key_F4    :
    case Qt::Key_F5    :
    case Qt::Key_F6    : break;
    case Qt::Key_F12   : TurnOFF(); break;

    case Qt::Key_Up    : General_Info.Stick |= 0x01; AddKey(0x1e);break;
    case Qt::Key_Right : General_Info.Stick |= 0x02; AddKey(0x1c);break;
    case Qt::Key_Down  : General_Info.Stick |= 0x04; AddKey(0x1f);break;
    case Qt::Key_Left  : General_Info.Stick |= 0x08; AddKey(0x1d);break;


    case Qt::Key_Return : AddKey(0x0d);break;

    case Qt::Key_Shift : shift = true; break;
    case Qt::Key_Control: ctrl = true; break;
    case Qt::Key_Alt:   graph = true; break;

    case Qt::Key_Space:
        General_Info.Strig1 = 0x00;
    default:

        kana=graph=shift=ctrl = false;
        switch (event->modifiers()) {
            case Qt::ShiftModifier : shift = true; break;
            case Qt::AltModifier:   graph = true; break;
            case Qt::ControlModifier: ctrl = true; break;
        }
        code = event->key();
        if ((code == 0x03) || (ctrl && code==Qt::Key_C)) {
            General_Info.Break = 1;
            val = 0;
        }
        else
        if(ctrl) val = key_tbl_c[code];
        else if(kana) {
            if(shift) val = key_tbl_ks[code];
            else val = key_tbl_k[code];
        }
        else if(graph) val = key_tbl_g[code];
        //else if(shift) val = key_tbl_s[code];
        else {
            val = code;//key_tbl[code];
            // Manage lowercase
            if (shift ) {//&& !((val >=0x30) && (val <=0x39))) {
                if ((val >=0x41) && (val <= 0x5a)) val += 0x20;
                else if ((code == pPC->pKEYB->LastKey)/*&&(val >=0x30) && (val <= 0x39)*/) val =key_tbl_s[code];
            }
        }

        if(val) AddKey(val);

    }
    event->accept();
}


void CT6834::TurnOFF() {
    //      m_warm_start = 1;
    //      m_sleep = 0;
    mem[0x0006] &= 0xBF;      // bit 6 to 0 for SLEEP
    mem[0x0006] |= 0x01;      // bit 0 to 1 for OFF
    General_Info.LcdOn = false;
    emit TurnOFFSig();
    //    pPC->TurnOFF();
}

void CT6834::setRamVideo(int x, int y,UINT8 val)
{
    if ((x>=0) && (x<120) && (y>=0) && ( y<32)) {
        Ram_Video[x][y] = val;
    }
}

#define KEY(a) (Clavier.contains(a) || Clavier.contains(TOUPPER(a)))

quint8 CT6834::getKey(quint16 strobe) {
    quint8 data = 0;

    if (strobe & 0x01) {
        if (KEY('a'))   data |= 0x01;
        if (KEY('a'))   data |= 0x02;
        if (KEY('a'))   data |= 0x04;
        if (KEY('a'))   data |= 0x08;
        if (KEY('a'))   data |= 0x10;
        if (KEY('a'))   data |= 0x20;
        if (KEY(' '))   data |= 0x40;
    }

    if (strobe & 0x02) {
        if (KEY('z'))   data |= 0x01;
        if (KEY('x'))   data |= 0x02;
        if (KEY('c'))   data |= 0x04;
        if (KEY('v'))   data |= 0x08;
        if (KEY('b'))   data |= 0x10;
        if (KEY('n'))   data |= 0x20;
        if (KEY('m'))   data |= 0x40;
        if (KEY('a'))   data |= 0x80;
    }

    if (strobe & 0x04) {
        if (KEY('a'))   data |= 0x01;
        if (KEY('s'))   data |= 0x02;
        if (KEY('d'))   data |= 0x04;
        if (KEY('f'))   data |= 0x08;
        if (KEY('g'))   data |= 0x10;
        if (KEY('h'))   data |= 0x20;
        if (KEY('j'))   data |= 0x40;
        if (KEY('k'))   data |= 0x80;
    }

    if (strobe & 0x08) {
        if (KEY('q'))   data |= 0x01;
        if (KEY('w'))   data |= 0x02;
        if (KEY('e'))   data |= 0x04;
        if (KEY('r'))   data |= 0x08;
        if (KEY('t'))   data |= 0x10;
        if (KEY('y'))   data |= 0x20;
        if (KEY('u'))   data |= 0x40;
        if (KEY('i'))   data |= 0x80;
    }

    if (strobe & 0x10) {
        if (KEY('!'))   data |= 0x01;
        if (KEY('"'))   data |= 0x02;
        if (KEY('#'))   data |= 0x04;
        if (KEY('$'))   data |= 0x08;
        if (KEY('%'))   data |= 0x10;
        if (KEY('&'))   data |= 0x20;
//        if (KEY('\'))   data |= 0x40;
        if (KEY('('))   data |= 0x80;
    }

    if (strobe & 0x20) {
        if (KEY(Qt::Key_F1))   data |= 0x01;
        if (KEY(Qt::Key_F2))   data |= 0x02;
        if (KEY(Qt::Key_F3))   data |= 0x04;
        if (KEY(Qt::Key_F4))   data |= 0x08;
        if (KEY(Qt::Key_F5))   data |= 0x10;
        if (KEY(Qt::Key_F6))   data |= 0x20;
    }

    if (strobe & 0x40) {
        if (KEY('.'))   data |= 0x01;
        if (KEY('/'))   data |= 0x02;
        if (KEY('?'))   data |= 0x04;
        if (KEY(K_RET))   data |= 0x08;
        if (KEY('a'))   data |= 0x10;
        if (KEY('a'))   data |= 0x20;
        if (KEY('a'))   data |= 0x40;
        if (KEY('a'))   data |= 0x80;
    }

                return data;
}

#if 0
1246      PORT_START("S1")
1247          PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("INS")     PORT_CODE(KEYCODE_INSERT)           PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x12)
1248          PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DEL")     PORT_CODE(KEYCODE_DEL)              PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x16)
1249          PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RIGHT")   PORT_CODE(KEYCODE_RIGHT)            PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x1c)
1250          PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LEFT")    PORT_CODE(KEYCODE_LEFT)             PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x1d)
1251          PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("UP")      PORT_CODE(KEYCODE_UP)               PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x1e)
1252          PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DOWN")    PORT_CODE(KEYCODE_DOWN)             PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x1f)
1253          PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPC")     PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x20)
1254      PORT_START("S2")
1255          PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x5a)
1256          PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x58)
1257          PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x43)
1258          PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x56)
1259          PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x42)
1260          PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x4e)
1261          PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x4d)
1262          PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')   PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x2c)
1263      PORT_START("S3")
1264          PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x41)
1265          PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x53)
1266          PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x44)
1267          PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x46)
1268          PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x47)
1269          PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x48)
1270          PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x4a)
1271          PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x4b)
1272      PORT_START("S4")
1273          PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x51)
1274          PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x57)
1275          PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x45)
1276          PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x52)
1277          PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x54)
1278          PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x59)
1279          PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x55)
1280          PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x49)
1281      PORT_START("S5")
1282          PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x31)
1283          PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x32)
1284          PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x33)
1285          PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x34)
1286          PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x35)
1287          PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x36)
1288          PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')      PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x37)
1289          PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x38)
1290      PORT_START("S6")
1291          PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)                    PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_func_keys, 1)
1292          PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)                    PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_func_keys, 2)
1293          PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)                    PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_func_keys, 3)
1294          PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)                    PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_func_keys, 4)
1295          PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)                    PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_func_keys, 5)
1296          PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F6") PORT_CODE(KEYCODE_F6)                    PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_func_keys, 6)
1297      PORT_START("S7")
1298          PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')    PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x2e)
1299          PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')   PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x2f)
1300          PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGUP) PORT_CHAR('?')                   PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x3f)
1301          PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER)  PORT_CHAR(13)  PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x0d)
1302          PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x4f)
1303          PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x50)
1304          PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR('@') PORT_CHAR('\'')  PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x40)
1305          PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')   PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x5b)
1306      PORT_START("S8")
1307          PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x4c)
1308          PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(';') PORT_CHAR('+')   PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x3b)
1309          PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')   PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x3a)
1310          PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')  PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x5d)
1311          PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x39)
1312          PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('|')       PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x30)
1313          PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')   PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x2d)
1314          PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('`')  PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x3d)
1315      PORT_START("BZ")
1316          PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("HOME")    PORT_CODE(KEYCODE_HOME)             PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_keys, 0x0b)
1317          PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KANA")    PORT_CODE(KEYCODE_RALT)
1318          PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("GRPH")    PORT_CODE(KEYCODE_RCONTROL)
1319          PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("NUM")     PORT_CODE(KEYCODE_LALT)
1320          PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("OFF")     PORT_CODE(KEYCODE_RSHIFT)
1321          PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ON/BREAK") PORT_CODE(KEYCODE_F10)             PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_break, 0)
1322      PORT_START("A1")
1323          PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)             PORT_CHAR(UCHAR_SHIFT_1)    PORT_CHANGED_MEMBER(DEVICE_SELF,x07_state,kb_update_udk, 0)
1324          PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)
1325  INPUT_PORTS_END

#endif
/*
  E1FE
            VIDEO RAM
  E000

  CFFF
            Programme de lancement
  CE00

  CCFF
            Buffer clavier
  CC00
            Char user defined
  CA00
            F Key
  C800

  C000
  */
