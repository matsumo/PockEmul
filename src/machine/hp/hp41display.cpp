// *********************************************************************
//    Copyright (c) 1989-2004  Warren Furlow
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// *********************************************************************

// *********************************************************************
// Chp41Display.cpp
//   Executes the Display instructions for Halfnut LCD
// *********************************************************************

#include <QDebug>
#if 1
#include "hp41.h"
#include "hp41Cpu.h"
#include "Lcdc.h"

/****************************/
/* called from subclass A - perph 0xfd */
/****************************/
void Chp41::DisplayWrite()
  {
  switch (hp41cpu->Modifier)
    {
    // 028          SRLDA    WRA12L   SRLDA
    case 0:
      {
      DIS_A_REG[0]=hp41cpu->r->C_REG[0];
      DIS_A_REG[1]=hp41cpu->r->C_REG[1];
      DIS_A_REG[2]=hp41cpu->r->C_REG[2];
      DIS_A_REG[3]=hp41cpu->r->C_REG[3];
      DIS_A_REG[4]=hp41cpu->r->C_REG[4];
      DIS_A_REG[5]=hp41cpu->r->C_REG[5];
      DIS_A_REG[6]=hp41cpu->r->C_REG[6];
      DIS_A_REG[7]=hp41cpu->r->C_REG[7];
      DIS_A_REG[8]=hp41cpu->r->C_REG[8];
      DIS_A_REG[9]=hp41cpu->r->C_REG[9];
      DIS_A_REG[10]=hp41cpu->r->C_REG[10];
      DIS_A_REG[11]=hp41cpu->r->C_REG[11];
      break;
      }
    // 068          SRLDB    WRB12L   SRLDB
    case 1:
      {
      DIS_B_REG[0]=hp41cpu->r->C_REG[0];
      DIS_B_REG[1]=hp41cpu->r->C_REG[1];
      DIS_B_REG[2]=hp41cpu->r->C_REG[2];
      DIS_B_REG[3]=hp41cpu->r->C_REG[3];
      DIS_B_REG[4]=hp41cpu->r->C_REG[4];
      DIS_B_REG[5]=hp41cpu->r->C_REG[5];
      DIS_B_REG[6]=hp41cpu->r->C_REG[6];
      DIS_B_REG[7]=hp41cpu->r->C_REG[7];
      DIS_B_REG[8]=hp41cpu->r->C_REG[8];
      DIS_B_REG[9]=hp41cpu->r->C_REG[9];
      DIS_B_REG[10]=hp41cpu->r->C_REG[10];
      DIS_B_REG[11]=hp41cpu->r->C_REG[11];
      break;
      }
    // 0A8          SRLDC    WRC12L   SRLDC
    case 2:
      {
      DIS_C_REG[0]=hp41cpu->r->C_REG[0];
      DIS_C_REG[1]=hp41cpu->r->C_REG[1];
      DIS_C_REG[2]=hp41cpu->r->C_REG[2];
      DIS_C_REG[3]=hp41cpu->r->C_REG[3];
      DIS_C_REG[4]=hp41cpu->r->C_REG[4];
      DIS_C_REG[5]=hp41cpu->r->C_REG[5];
      DIS_C_REG[6]=hp41cpu->r->C_REG[6];
      DIS_C_REG[7]=hp41cpu->r->C_REG[7];
      DIS_C_REG[8]=hp41cpu->r->C_REG[8];
      DIS_C_REG[9]=hp41cpu->r->C_REG[9];
      DIS_C_REG[10]=hp41cpu->r->C_REG[10];
      DIS_C_REG[11]=hp41cpu->r->C_REG[11];
      break;
      }
    // 0E8          SRLDAB   WRAB6L   SRLDAB
    case 3:
      {
      DisplayRotLeft(DIS_A_REG,6);
      DisplayRotLeft(DIS_B_REG,6);
      DIS_A_REG[6]=hp41cpu->r->C_REG[0];
      DIS_B_REG[6]=hp41cpu->r->C_REG[1];
      DIS_A_REG[7]=hp41cpu->r->C_REG[2];
      DIS_B_REG[7]=hp41cpu->r->C_REG[3];
      DIS_A_REG[8]=hp41cpu->r->C_REG[4];
      DIS_B_REG[8]=hp41cpu->r->C_REG[5];
      DIS_A_REG[9]=hp41cpu->r->C_REG[6];
      DIS_B_REG[9]=hp41cpu->r->C_REG[7];
      DIS_A_REG[10]=hp41cpu->r->C_REG[8];
      DIS_B_REG[10]=hp41cpu->r->C_REG[9];
      DIS_A_REG[11]=hp41cpu->r->C_REG[10];
      DIS_B_REG[11]=hp41cpu->r->C_REG[11];
      break;
      }
    // 128          SRLABC   WRABC4L  SRLABC                       ;also HP:SRLDABC
    case 4:
      {
      DisplayRotLeft(DIS_A_REG,4);
      DisplayRotLeft(DIS_B_REG,4);
      DisplayRotLeft(DIS_C_REG,4);
      DIS_A_REG[8]=hp41cpu->r->C_REG[0];
      DIS_B_REG[8]=hp41cpu->r->C_REG[1];
      DIS_C_REG[8]=hp41cpu->r->C_REG[2]&0x01;
      DIS_A_REG[9]=hp41cpu->r->C_REG[3];
      DIS_B_REG[9]=hp41cpu->r->C_REG[4];
      DIS_C_REG[9]=hp41cpu->r->C_REG[5]&0x01;
      DIS_A_REG[10]=hp41cpu->r->C_REG[6];
      DIS_B_REG[10]=hp41cpu->r->C_REG[7];
      DIS_C_REG[10]=hp41cpu->r->C_REG[8]&0x01;
      DIS_A_REG[11]=hp41cpu->r->C_REG[9];
      DIS_B_REG[11]=hp41cpu->r->C_REG[10];
      DIS_C_REG[11]=hp41cpu->r->C_REG[11]&0x01;
      break;
      }
    // 168          SLLDAB   WRAB6R   SLLDAB
    case 5:
      {
      DisplayRotRight(DIS_A_REG,6);
      DisplayRotRight(DIS_B_REG,6);
      DIS_A_REG[5]=hp41cpu->r->C_REG[0];
      DIS_B_REG[5]=hp41cpu->r->C_REG[1];
      DIS_A_REG[4]=hp41cpu->r->C_REG[2];
      DIS_B_REG[4]=hp41cpu->r->C_REG[3];
      DIS_A_REG[3]=hp41cpu->r->C_REG[4];
      DIS_B_REG[3]=hp41cpu->r->C_REG[5];
      DIS_A_REG[2]=hp41cpu->r->C_REG[6];
      DIS_B_REG[2]=hp41cpu->r->C_REG[7];
      DIS_A_REG[1]=hp41cpu->r->C_REG[8];
      DIS_B_REG[1]=hp41cpu->r->C_REG[9];
      DIS_A_REG[0]=hp41cpu->r->C_REG[10];
      DIS_B_REG[0]=hp41cpu->r->C_REG[11];
      break;
      }
    // 1A8          SLLABC   WRABC4R  SLLABC                       ;also HP:SLLDABC
    case 6:
      {
      DisplayRotRight(DIS_A_REG,4);
      DisplayRotRight(DIS_B_REG,4);
      DisplayRotRight(DIS_C_REG,4);
      DIS_A_REG[3]=hp41cpu->r->C_REG[0];
      DIS_B_REG[3]=hp41cpu->r->C_REG[1];
      DIS_C_REG[3]=hp41cpu->r->C_REG[2]&0x01;
      DIS_A_REG[2]=hp41cpu->r->C_REG[3];
      DIS_B_REG[2]=hp41cpu->r->C_REG[4];
      DIS_C_REG[2]=hp41cpu->r->C_REG[5]&0x01;
      DIS_A_REG[1]=hp41cpu->r->C_REG[6];
      DIS_B_REG[1]=hp41cpu->r->C_REG[7];
      DIS_C_REG[1]=hp41cpu->r->C_REG[8]&0x01;
      DIS_A_REG[0]=hp41cpu->r->C_REG[9];
      DIS_B_REG[0]=hp41cpu->r->C_REG[10];
      DIS_C_REG[0]=hp41cpu->r->C_REG[11]&0x01;
      break;
      }
    // 1E8          SRSDA    WRA1L    SRSDA
    case 7:
      {
      DisplayRotLeft(DIS_A_REG,1);
      DIS_A_REG[11]=hp41cpu->r->C_REG[0];
      break;
      }
    // 228          SRSDB    WRB1L    SRSDB
    case 8:
      {
      DisplayRotLeft(DIS_B_REG,1);
      DIS_B_REG[11]=hp41cpu->r->C_REG[0];
      break;
      }
    // 268          SRSDC    WRC1L    SRSDC
    case 9:
      {
      DisplayRotLeft(DIS_C_REG,1);
      DIS_C_REG[11]=hp41cpu->r->C_REG[0]&0x01;
      break;
      }
    // 2A8          SLSDA    WRA1R    SLSDA
    case 0xA:
      {
      DisplayRotRight(DIS_A_REG,1);
      DIS_A_REG[0]=hp41cpu->r->C_REG[0];
      break;
      }
    // 2E8          SLSDB    WRB1R    SLSDB
    case 0xB:
      {
      DisplayRotRight(DIS_B_REG,1);
      DIS_B_REG[0]=hp41cpu->r->C_REG[0];
      break;
      }
    // 328          SRSDAB   WRAB1L   SRSDAB                        ;Zenrom manual incorrectly says this is WRC1R
    case 0xC:
      {                 //
      DisplayRotLeft(DIS_A_REG,1);
      DisplayRotLeft(DIS_B_REG,1);
      DIS_A_REG[11]=hp41cpu->r->C_REG[0];
      DIS_B_REG[11]=hp41cpu->r->C_REG[1];
      break;
      }
    // 368          SLSDAB   WRAB1R   SLSDAB
    case 0xD:
      {
      DisplayRotRight(DIS_A_REG,1);
      DisplayRotRight(DIS_B_REG,1);
      DIS_A_REG[0]=hp41cpu->r->C_REG[0];
      DIS_B_REG[0]=hp41cpu->r->C_REG[1];
      break;
      }
    // 3A8          SRSABC   WRABC1L  SRSABC                        ;also HP:SRSDABC
    case 0xE:
      {
      DisplayRotLeft(DIS_A_REG,1);
      DisplayRotLeft(DIS_B_REG,1);
      DisplayRotLeft(DIS_C_REG,1);
      DIS_A_REG[11]=hp41cpu->r->C_REG[0];
      DIS_B_REG[11]=hp41cpu->r->C_REG[1];
      DIS_C_REG[11]=hp41cpu->r->C_REG[2]&0x01;
      break;
      }
    // 3E8          SLSABC   WRABC1R  SLSABC                        ;also HP:SLSDABC
    case 0xF:
      {
      DisplayRotRight(DIS_A_REG,1);
      DisplayRotRight(DIS_B_REG,1);
      DisplayRotRight(DIS_C_REG,1);
      DIS_A_REG[0]=hp41cpu->r->C_REG[0];
      DIS_B_REG[0]=hp41cpu->r->C_REG[1];
      DIS_C_REG[0]=hp41cpu->r->C_REG[2]&0x01;
      break;
      }
    }
  UpdateDisplay=true;
  pLCDC->updated = true;

  hp41cpu->Boost=DISPLAY_BOOST;
  }

/****************************/
/* called from subclass E - perph 0xfd */
/****************************/
void Chp41::DisplayRead()
  {
  switch (hp41cpu->Modifier)
    {
    // 038          FLLDA    RDA12L   FLLDA
    case 0:
      {
      hp41cpu->r->C_REG[0]=DIS_A_REG[11];
      hp41cpu->r->C_REG[1]=DIS_A_REG[10];
      hp41cpu->r->C_REG[2]=DIS_A_REG[9];
      hp41cpu->r->C_REG[3]=DIS_A_REG[8];
      hp41cpu->r->C_REG[4]=DIS_A_REG[7];
      hp41cpu->r->C_REG[5]=DIS_A_REG[6];
      hp41cpu->r->C_REG[6]=DIS_A_REG[5];
      hp41cpu->r->C_REG[7]=DIS_A_REG[4];
      hp41cpu->r->C_REG[8]=DIS_A_REG[3];
      hp41cpu->r->C_REG[9]=DIS_A_REG[2];
      hp41cpu->r->C_REG[10]=DIS_A_REG[1];
      hp41cpu->r->C_REG[11]=DIS_A_REG[0];
      break;
      }
    // 078          FLLDB    RDB12L   FLLDB
    case 1:
      {
      hp41cpu->r->C_REG[0]=DIS_B_REG[11];
      hp41cpu->r->C_REG[1]=DIS_B_REG[10];
      hp41cpu->r->C_REG[2]=DIS_B_REG[9];
      hp41cpu->r->C_REG[3]=DIS_B_REG[8];
      hp41cpu->r->C_REG[4]=DIS_B_REG[7];
      hp41cpu->r->C_REG[5]=DIS_B_REG[6];
      hp41cpu->r->C_REG[6]=DIS_B_REG[5];
      hp41cpu->r->C_REG[7]=DIS_B_REG[4];
      hp41cpu->r->C_REG[8]=DIS_B_REG[3];
      hp41cpu->r->C_REG[9]=DIS_B_REG[2];
      hp41cpu->r->C_REG[10]=DIS_B_REG[1];
      hp41cpu->r->C_REG[11]=DIS_B_REG[0];
      break;
      }
    // 0B8          FLLDC    RDC12L   FLLDC
    case 2:
      {
      hp41cpu->r->C_REG[0]=DIS_C_REG[11];
      hp41cpu->r->C_REG[1]=DIS_C_REG[10];
      hp41cpu->r->C_REG[2]=DIS_C_REG[9];
      hp41cpu->r->C_REG[3]=DIS_C_REG[8];
      hp41cpu->r->C_REG[4]=DIS_C_REG[7];
      hp41cpu->r->C_REG[5]=DIS_C_REG[6];
      hp41cpu->r->C_REG[6]=DIS_C_REG[5];
      hp41cpu->r->C_REG[7]=DIS_C_REG[4];
      hp41cpu->r->C_REG[8]=DIS_C_REG[3];
      hp41cpu->r->C_REG[9]=DIS_C_REG[2];
      hp41cpu->r->C_REG[10]=DIS_C_REG[1];
      hp41cpu->r->C_REG[11]=DIS_C_REG[0];
      break;
      }
    // 0F8          FLLDAB   RDAB6L   FLLDAB
    case 3:
      {
      hp41cpu->r->C_REG[0]=DIS_A_REG[11];
      hp41cpu->r->C_REG[1]=DIS_B_REG[11];
      hp41cpu->r->C_REG[2]=DIS_A_REG[10];
      hp41cpu->r->C_REG[3]=DIS_B_REG[10];
      hp41cpu->r->C_REG[4]=DIS_A_REG[9];
      hp41cpu->r->C_REG[5]=DIS_B_REG[9];
      hp41cpu->r->C_REG[6]=DIS_A_REG[8];
      hp41cpu->r->C_REG[7]=DIS_B_REG[8];
      hp41cpu->r->C_REG[8]=DIS_A_REG[7];
      hp41cpu->r->C_REG[9]=DIS_B_REG[7];
      hp41cpu->r->C_REG[10]=DIS_A_REG[6];
      hp41cpu->r->C_REG[11]=DIS_B_REG[6];
      DisplayRotRight(DIS_A_REG,6);
      DisplayRotRight(DIS_B_REG,6);
      UpdateDisplay=true;
      pLCDC->updated = true;
      hp41cpu->Boost=DISPLAY_BOOST;
      break;
      }
    // 138          FLLABC   RDABC4L  FLLABC                        ;also HP:FLLDABC
    case 4:
      {
      hp41cpu->r->C_REG[0]=DIS_A_REG[11];
      hp41cpu->r->C_REG[1]=DIS_B_REG[11];
      hp41cpu->r->C_REG[2]=DIS_C_REG[11];
      hp41cpu->r->C_REG[3]=DIS_A_REG[10];
      hp41cpu->r->C_REG[4]=DIS_B_REG[10];
      hp41cpu->r->C_REG[5]=DIS_C_REG[10];
      hp41cpu->r->C_REG[6]=DIS_A_REG[9];
      hp41cpu->r->C_REG[7]=DIS_B_REG[9];
      hp41cpu->r->C_REG[8]=DIS_C_REG[9];
      hp41cpu->r->C_REG[9]=DIS_A_REG[8];
      hp41cpu->r->C_REG[10]=DIS_B_REG[8];
      hp41cpu->r->C_REG[11]=DIS_C_REG[8];
      DisplayRotRight(DIS_A_REG,4);
      DisplayRotRight(DIS_B_REG,4);
      DisplayRotRight(DIS_C_REG,4);
      UpdateDisplay=true;
      pLCDC->updated = true;

      hp41cpu->Boost=DISPLAY_BOOST;
      break;
      }
    // 178          READEN   READAN   READEN                        ;copy annunciators into C[2:0]
    case 5:
      {
      hp41cpu->r->C_REG[0]=(My_byte)(DIS_ANNUN_REG&0x00f);
      hp41cpu->r->C_REG[1]=(My_byte)((DIS_ANNUN_REG&0x0f0)>>4);
      hp41cpu->r->C_REG[2]=(My_byte)((DIS_ANNUN_REG&0xf00)>>8);
      break;
      }
    // 1B8          FLSDC    RDC1L    FLSDC
    case 6:
      {
      hp41cpu->r->C_REG[0]=DIS_C_REG[11];
      DisplayRotRight(DIS_C_REG,1);
      UpdateDisplay=true;
      pLCDC->updated = true;

      hp41cpu->Boost=DISPLAY_BOOST;
      break;
      }
    // 1F8          FRSDA    RDA1R    FRSDA
    case 7:
      {
      hp41cpu->r->C_REG[0]=DIS_A_REG[0];
      DisplayRotLeft(DIS_A_REG,1);
      UpdateDisplay=true;
      pLCDC->updated = true;

      hp41cpu->Boost=DISPLAY_BOOST;
      break;
      }
    // 238          FRSDB    RDB1R    FRSDB
    case 8:
      {
      hp41cpu->r->C_REG[0]=DIS_B_REG[0];
      DisplayRotLeft(DIS_B_REG,1);
      UpdateDisplay=true;
      pLCDC->updated = true;

      hp41cpu->Boost=DISPLAY_BOOST;
      break;
      }
    // 278          FRSDC    RDC1R    FRSDC
    case 9:
      {
      hp41cpu->r->C_REG[0]=DIS_C_REG[0];
      DisplayRotLeft(DIS_C_REG,1);
      UpdateDisplay=true;
      pLCDC->updated = true;

      hp41cpu->Boost=DISPLAY_BOOST;
      break;
      }
    // 2B8          FLSDA    RDA1L    FLSDA
    case 0xA:
      {
      hp41cpu->r->C_REG[0]=DIS_A_REG[11];
      DisplayRotRight(DIS_A_REG,1);
      UpdateDisplay=true;
      pLCDC->updated = true;

      hp41cpu->Boost=DISPLAY_BOOST;
      break;
      }
    // 2F8          FLSDB    RDB1L    FLSDB
    case 0xB:
      {
      hp41cpu->r->C_REG[0]=DIS_B_REG[11];
      DisplayRotRight(DIS_B_REG,1);
      UpdateDisplay=true;
      pLCDC->updated = true;

      hp41cpu->Boost=DISPLAY_BOOST;
      break;
      }
    // 338          FRSDAB   RDAB1R   FRSDAB
    case 0xC:
      {
      hp41cpu->r->C_REG[0]=DIS_A_REG[0];
      hp41cpu->r->C_REG[1]=DIS_B_REG[0];
      DisplayRotLeft(DIS_A_REG,1);
      DisplayRotLeft(DIS_B_REG,1);
      UpdateDisplay=true;
      pLCDC->updated = true;

      hp41cpu->Boost=DISPLAY_BOOST;
      break;
      }
    // 378          FLSDAB   RDAB1L   FLSDAB
    case 0xD:
      {
      hp41cpu->r->C_REG[0]=DIS_A_REG[11];
      hp41cpu->r->C_REG[1]=DIS_B_REG[11];
      DisplayRotRight(DIS_A_REG,1);
      DisplayRotRight(DIS_B_REG,1);
      UpdateDisplay=true;
      pLCDC->updated = true;

      hp41cpu->Boost=DISPLAY_BOOST;
      break;
      }
    // 3B8          RABCR    RDABC1R  RABCR				;also HP:FRSDABC, FRSABC
    case 0xE:
      {
      hp41cpu->r->C_REG[0]=DIS_A_REG[0];
      hp41cpu->r->C_REG[1]=DIS_B_REG[0];
      hp41cpu->r->C_REG[2]=DIS_C_REG[0];
      DisplayRotLeft(DIS_A_REG,1);
      DisplayRotLeft(DIS_B_REG,1);
      DisplayRotLeft(DIS_C_REG,1);
      UpdateDisplay=true;
      pLCDC->updated = true;

      hp41cpu->Boost=DISPLAY_BOOST;
      break;
      }
    // 3F8          RABCL    RDABC1L  RABCL				;also HP:FLSDABC, FLSABC
    case 0xF:
      {
      hp41cpu->r->C_REG[0]=DIS_A_REG[11];
      hp41cpu->r->C_REG[1]=DIS_B_REG[11];
      hp41cpu->r->C_REG[2]=DIS_C_REG[11];
      DisplayRotRight(DIS_A_REG,1);
      DisplayRotRight(DIS_B_REG,1);
      DisplayRotRight(DIS_C_REG,1);
      UpdateDisplay=true;
      pLCDC->updated = true;

      hp41cpu->Boost=DISPLAY_BOOST;
      break;
      }
    }
  }

/****************************/
void Chp41::DisplayRotRight(
  My_byte REG[],
  int Times)
  {
  My_byte TEMP[12];
  memcpy(TEMP,REG,12);
  memcpy(REG+Times,TEMP,12-Times);
  memcpy(REG,TEMP+12-Times,Times);
  }

/****************************/
void Chp41::DisplayRotLeft(
  My_byte REG[],
  int Times)
  {
  My_byte TEMP[12];
  memcpy(TEMP,REG,12);
  memcpy(REG,TEMP+Times,12-Times);
  memcpy(REG+12-Times,TEMP,Times);
  }

/****************************/
/* called from subclass C - perph 0xfd */
/****************************/
void Chp41::AnnunWrite()
  {
  // 2F0          WRTEN    WRITAN   WRTEN                         ;copy bits from C[2:0] into annunciators

    DIS_ANNUN_REG=(hp41cpu->r->C_REG[2]<<8) | (hp41cpu->r->C_REG[1]<<4) | hp41cpu->r->C_REG[0];
  UpdateAnnun=1;
  pLCDC->updated= true;
  qWarning()<<"AnnunWrite:"<<DIS_ANNUN_REG;
  }

/****************************/
/* called from subclass A - perph 0x10 */
/****************************/
void Chp41::HalfnutWrite()
{
    // REG=C 5
    if (hp41cpu->Modifier==5)
    {
        SetContrast(hp41cpu->r->C_REG[0]);
        UpdateDisplay=true;
        pLCDC->updated = true;

        UpdateAnnun=1;
    }
}

/****************************/
/* called from subclass E - perph 0x10 */
/****************************/
void Chp41::HalfnutRead()
  {
  // C=REG 5
  if (hp41cpu->Modifier==5)
    hp41cpu->r->C_REG[0]=(My_byte)Contrast;
  }

#endif
