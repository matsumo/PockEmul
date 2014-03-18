// *********************************************************************
//    Copyright (c) 1989-2002  Warren Furlow
//    DecodeUCByte Copyright (c) 2000 Leo Duran
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
// HP41File.cpp : implementation file
// *********************************************************************

#if 1
#include <QDebug>

#include "hp41.h"
#include "hp41Cpu.h"
#include "hp41mod.h"

/****************************/
// Returns 0 for success, 1 for open fail, 2 for read fail, 3 for invalid file, 4 for load conflict or no space
/****************************/
int Chp41::LoadMOD(
  ModuleHeader *&pModuleOut,     // output
  QString FullPath)
  {

  pModuleOut=NULL;
  QByteArray pBuff;

  // open file and read its contents into a buffer
  QFile file(FullPath);
  if (! file.open(QIODevice::ReadOnly)) return(1);

  qint64 FileSize=file.size();
  if ((FileSize-sizeof(ModuleFileHeader))%sizeof(ModuleFilePage))
    return(3);


  pBuff = file.readAll();
  file.close();

  if (FileSize!=pBuff.size()) {
      return(2);
  }

  return LoadMOD(pModuleOut,FullPath,pBuff);
}

  int Chp41::LoadMOD(ModuleHeader *&pModuleOut,     // output
    QString FullPath,
          QByteArray pBuff)
    {

      uint page,hep_page=0,ww_page=0;

  // validate module header
  ModuleFileHeader *pMFH=(ModuleFileHeader*)pBuff.data();
  if (pBuff.size() != sizeof(ModuleFileHeader)+pMFH->NumPages*sizeof(ModuleFilePage) ||
    0!=strcmp(pMFH->FileFormat,MOD_FORMAT) || pMFH->MemModules>4 || pMFH->XMemModules>3 ||
    pMFH->Original>1 || pMFH->AppAutoUpdate>1 || pMFH->Category>CATEGORY_MAX || pMFH->Hardware>HARDWARE_MAX)    /* out of range */
    {
    return(3);
    }
  if ((MemModules+pMFH->MemModules>4) || (XMemModules+pMFH->XMemModules>3))
    {
    return(4);
    }
  ModuleHeader *pModuleNew=new ModuleHeader;
  pModuleOut=pModuleNew;
  // info strings
  strncpy(pModuleNew->szFullFileName,(char*)FullPath.data(),sizeof(pModuleNew->szFullFileName));
  strcpy(pModuleNew->szFileFormat,pMFH->FileFormat);
  strcpy(pModuleNew->szTitle,pMFH->Title);
  strcpy(pModuleNew->szVersion,pMFH->Version);
  strcpy(pModuleNew->szPartNumber,pMFH->PartNumber);
  strcpy(pModuleNew->szAuthor,pMFH->Author);
  strcpy(pModuleNew->szCopyright,pMFH->Copyright);
  strcpy(pModuleNew->szLicense,pMFH->License);
  strcpy(pModuleNew->szComments,pMFH->Comments);
  // special module characteristics
  pModuleNew->MemModules=pMFH->MemModules;
  MemModules+=pMFH->MemModules;
  pModuleNew->XMemModules=pMFH->XMemModules;
  XMemModules+=pMFH->XMemModules;
  pModuleNew->Category=pMFH->Category;
  pModuleNew->Hardware=pMFH->Hardware;
  switch (pMFH->Hardware)
    {
    case HARDWARE_PRINTER:
      fPrinter=currentSlot;
      break;
    case HARDWARE_CARDREADER:
      fCardReader=currentSlot;
      break;
    case HARDWARE_TIMER:
      InitTimer();        // sets fTimer
      break;
    case HARDWARE_WAND:
      fWand=currentSlot;
      break;
    case HARDWARE_HPIL:
      fHPIL=currentSlot;
      break;
    case HARDWARE_INFRARED:
      fInfrared=currentSlot;
      break;
    case HARDWARE_HEPAX:
    case HARDWARE_NONE:
    default:
      break;
    }
  pModuleNew->Original=pMFH->Original;
  pModuleNew->AppAutoUpdate=pMFH->AppAutoUpdate;
  pModuleNew->NumPages=pMFH->NumPages;
  ModuleList.append(pModuleNew);

  // these are arrays indexed on the page group number (1-8) (unique only within each mod file)
  // dual use: values are either a count stored as a negative number or a (positive) page number 1-f
  int LowerGroup[8]={0,0,0,0,0,0,0,0};                          // <0, or =page # if lower page(s) go in group
  int UpperGroup[8]={0,0,0,0,0,0,0,0};                          // <0, or =page # if upper page(s) go in group
  int OddGroup[8]={0,0,0,0,0,0,0,0};                            // <0, or =page # if odd page(s) go in group
  int EvenGroup[8]={0,0,0,0,0,0,0,0};                           // <0, or =page # if even page(s) go in group
  int OrderedGroup[8]={0,0,0,0,0,0,0,0};                        // <0, or =page # if ordered page(s) go in group
  // load ROM pages with three pass process
  for (int pass=1;pass<=3;pass++)
    {
    for (int pageIndex=0;pageIndex<pModuleNew->NumPages;pageIndex++)
      {
      ModuleFilePage *pMFP = (ModuleFilePage*)(pBuff.data()+sizeof(ModuleFileHeader)+pageIndex*sizeof(ModuleFilePage));
      flag fLoad=false;
      switch(pass)
        {
        case 1:                                    // pass 1: validate page variables, flag grouped pages
        if ((pMFP->Page>0x0f && pMFP->Page<POSITION_MIN) || pMFP->Page>POSITION_MAX || pMFP->PageGroup>8 ||
          pMFP->Bank==0 || pMFP->Bank>4 || pMFP->BankGroup>8 || pMFP->Ram>1 || pMFP->WriteProtect>1 || pMFP->FAT>1 ||  /* out of range values */
          (pMFP->PageGroup && pMFP->Page<=POSITION_ANY) ||    /* group pages cannot use non-grouped position codes */
          (!pMFP->PageGroup && pMFP->Page>POSITION_ANY))      /* non-grouped pages cannot use grouped position codes */
          {
          UnloadMOD(pModuleNew);
          return(3);
          }
        if (pMFP->PageGroup==0)                   // if not grouped do nothing in this pass
          break;
        if (pMFP->Page==POSITION_LOWER)
          LowerGroup[pMFP->PageGroup-1]-=1;       // save the count of pages with each attribute as a negative number
        else if (pMFP->Page==POSITION_UPPER)
          UpperGroup[pMFP->PageGroup-1]-=1;
        else if (pMFP->Page==POSITION_ODD)
          OddGroup[pMFP->PageGroup-1]-=1;
        else if (pMFP->Page==POSITION_EVEN)
          EvenGroup[pMFP->PageGroup-1]-=1;
        else if (pMFP->Page==POSITION_ORDERED)
          OrderedGroup[pMFP->PageGroup-1]-=1;
        break;

        case 2:                                   // pass 2: find free location for grouped pages
        if (pMFP->PageGroup==0)                   // if not grouped do nothing in this pass
          break;
        // a matching page has already been loaded
        if (pMFP->Page==POSITION_LOWER && UpperGroup[pMFP->PageGroup-1]>0)       // this is the lower page and the upper page has already been loaded
          page=UpperGroup[pMFP->PageGroup-1]-1;
        else if (pMFP->Page==POSITION_LOWER && LowerGroup[pMFP->PageGroup-1]>0)  // this is another lower page
          page=LowerGroup[pMFP->PageGroup-1];
        else if (pMFP->Page==POSITION_UPPER && LowerGroup[pMFP->PageGroup-1]>0)  // this is the upper page and the lower page has already been loaded
          page=LowerGroup[pMFP->PageGroup-1]+1;
        else if (pMFP->Page==POSITION_UPPER && UpperGroup[pMFP->PageGroup-1]>0)  // this is another upper page
          page=UpperGroup[pMFP->PageGroup-1];
        else if (pMFP->Page==POSITION_ODD && EvenGroup[pMFP->PageGroup-1]>0)
          page=EvenGroup[pMFP->PageGroup-1]+1;
        else if (pMFP->Page==POSITION_ODD && OddGroup[pMFP->PageGroup-1]>0)
          page=OddGroup[pMFP->PageGroup-1];
        else if (pMFP->Page==POSITION_EVEN && OddGroup[pMFP->PageGroup-1]>0)
          page=OddGroup[pMFP->PageGroup-1]-1;
        else if (pMFP->Page==POSITION_EVEN && EvenGroup[pMFP->PageGroup-1]>0)
          page=EvenGroup[pMFP->PageGroup-1];
        else if (pMFP->Page==POSITION_ORDERED && OrderedGroup[pMFP->PageGroup-1]>0)
          page=++OrderedGroup[pMFP->PageGroup-1];
        // find first page in group
        else     // find free space depending on which combination of positions are specified
          {
          if (LowerGroup[pMFP->PageGroup-1]!=0 && UpperGroup[pMFP->PageGroup-1]!=0)      // lower and upper
            {
            page=8;
            while (page<=0xe && (PageMatrix[page][pMFP->Bank-1]!=NULL || PageMatrix[page+1][pMFP->Bank-1]!=NULL))
              page++;
            }
          else if (LowerGroup[pMFP->PageGroup-1]!=0)                                        // lower but no upper
            {
            page=8;
            while (page<=0xf && PageMatrix[page][pMFP->Bank-1]!=NULL)
              page++;
            }
          else if (UpperGroup[pMFP->PageGroup-1]!=0)                                        // upper but no lower
            {
            page=8;
            while (page<=0xf && PageMatrix[page][pMFP->Bank-1]!=NULL)
              page++;
            }
          else if (EvenGroup[pMFP->PageGroup-1]!=0 && OddGroup[pMFP->PageGroup-1]!=0)          // even and odd
            {
            page=8;
            while (page<=0xe && (PageMatrix[page][pMFP->Bank-1]!=NULL || PageMatrix[page+1][pMFP->Bank-1]!=NULL))
              page+=2;
            }
          else if (EvenGroup[pMFP->PageGroup-1]!=0)                                         // even only
            {
            page=8;
            while (page<=0xe && PageMatrix[page][pMFP->Bank-1]!=NULL)
              page+=2;
            }
          else if (OddGroup[pMFP->PageGroup-1]!=0)                                          // odd only
            {
            page=9;
            while (page<=0xe && PageMatrix[page][pMFP->Bank-1]!=NULL)
              page+=2;
            }
          else if (OrderedGroup[pMFP->PageGroup-1]!=0)                                      // a block
            {
            uint count=-OrderedGroup[pMFP->PageGroup-1];
            for (page=8;page<=0x10-count;page++)
              {
              uint nFree=0;
              int page2;
              for (page2=page;page2<=0x0f;page2++)  // count up free spaces
                {
                if (PageMatrix[page][pMFP->Bank-1]==NULL)
                  nFree++;
                else
                  break;
                }
              if (count<=nFree)            // found a space
                break;
              }
            }
          else
            {
            page=8;
            while (page<=0xf && PageMatrix[page][pMFP->Bank-1]!=NULL)
              page++;
            }
          // save the position that was found in the appropriate array
          if (pMFP->Page==POSITION_LOWER)
            LowerGroup[pMFP->PageGroup-1]=page;
          else if (pMFP->Page==POSITION_UPPER)
            {
            page++;                                  // found two positions - take the upper one
            UpperGroup[pMFP->PageGroup-1]=page;
            }
          else if (pMFP->Page==POSITION_EVEN)
            EvenGroup[pMFP->PageGroup-1]=page;
          else if (pMFP->Page==POSITION_ODD)
            OddGroup[pMFP->PageGroup-1]=page;
          else if (pMFP->Page==POSITION_ORDERED)
            OrderedGroup[pMFP->PageGroup-1]=page;
          }
        fLoad=true;
        break;

        case 3:      // pass 3 - find location for non-grouped pages
        if (pMFP->PageGroup)
          break;
        if (pMFP->Page==POSITION_ANY)           // a single page that can be loaded anywhere 8-F
          {
          page=8;
          while (page<=0xf && PageMatrix[page][pMFP->Bank-1]!=NULL)
            page++;
          }
        else                                    // page number is hardcoded
          page=pMFP->Page;
        fLoad=true;
        break;
        }

      if (fLoad)    // load the image
        {
        ModulePage *pNewPage=new ModulePage;
        pNewPage->pModule=pModuleNew;
        pNewPage->pAltPage=NULL;
        strcpy(pNewPage->szName,pMFP->Name);
        strcpy(pNewPage->szID,pMFP->ID);
        pNewPage->Page=pMFP->Page;
        qWarning()<<"Loaded page:"<<pNewPage->Page;
        pNewPage->ActualPage=page;
        pNewPage->Bank=pMFP->Bank;
        pNewPage->PageGroup=pMFP->PageGroup;
        pNewPage->BankGroup=pMFP->BankGroup;
        if (pMFP->BankGroup)
          pNewPage->ActualBankGroup=pMFP->BankGroup+NextActualBankGroup*8;  // ensures each bank group has a number that is unique to the entire simulator
        else
          pNewPage->ActualBankGroup=0;
        pNewPage->fRAM=pMFP->Ram;
        pNewPage->fWriteProtect=pMFP->WriteProtect;
        pNewPage->fFAT=pMFP->FAT;
        pNewPage->fHEPAX=(pMFH->Hardware==HARDWARE_HEPAX);
        pNewPage->fWWRAMBOX=(pMFH->Hardware==HARDWARE_WWRAMBOX);
        Chp41Mod::unpack_image(&(pNewPage->Image[0]),&(pMFP->Image[0]));
        // patch the NULL timeout value to be longer - not known if this works for all revisions
        if (page==0 && (0==strcmp(pNewPage->szName,"NUT0-D") || 0==strcmp(pNewPage->szName,"NUT0-G") || 0==strcmp(pNewPage->szName,"NUT0-N")))
          {
          pNewPage->Image[0x0ec7]=0x3ff;                               // original value =0x240
          pNewPage->Image[0x0fff]=Chp41Mod::compute_checksum(&(pNewPage->Image[0]));   // fix the checksum so service rom wont report error
          }
        // HEPAX special case
        if (hep_page && pNewPage->fHEPAX && pNewPage->fRAM)            // hepax was just loaded previously and this is the first RAM page after it
          {
          pNewPage->ActualPage=hep_page;
          PageMatrix[hep_page][pMFP->Bank-1]->pAltPage=pNewPage;       // load this RAM into alternate page
          }
        // W&W RAMBOX II special case
        else if (ww_page && pNewPage->fWWRAMBOX && pNewPage->fRAM)     // W&W code was just loaded previously and this is the first RAM page after it
          {
          pNewPage->ActualPage=ww_page;
          PageMatrix[ww_page][pMFP->Bank-1]->pAltPage=pNewPage;        // load this RAM into alternate page
          }
        else if (page>0xf || PageMatrix[page][pMFP->Bank-1]!=NULL)     // there is no free space or some load conflict exists
          {
          delete pNewPage;
          UnloadMOD(pModuleNew);
          return(4);
          }
        else                                                           // otherwise load into primary page
          PageMatrix[page][pMFP->Bank-1]=pNewPage;
        hep_page=0;
        ww_page=0;
        if (pNewPage->fHEPAX && !pNewPage->fRAM)                       // detect HEPAX ROM
          hep_page=page;
        if (pNewPage->fWWRAMBOX && !pNewPage->fRAM)                   // detect W&W RAMBOXII ROM
          ww_page=page;
        }
      }
    }

  NextActualBankGroup++;

  return(0);
  }


/****************************/
// returns 0 for failure, 1 for success
/****************************/
int Chp41::SaveMOD(
  ModuleHeader *pModule,
  char *pszFullPath)
  {
#if 0
  if (pModule==NULL)
    return(0);

  // validate the number of pages
  uint page,bank,count=0;
  for (page=0;page<=0xf;page++)
    for (bank=1;bank<=4;bank++)
    {
    if (PageMatrix[page][bank-1]==NULL || PageMatrix[page][bank-1]->pModule!=pModule)
      continue;
    count++;
    }
  if (count!=pModule->NumPages)
    return(0);

  // allocate a buffer
  byte *pBuff;
  DWORD FileSize;
  FileSize=sizeof(ModuleFileHeader)+sizeof(ModuleFilePage)*pModule->NumPages;
  pBuff=(byte*)malloc(FileSize);
  if (pBuff==NULL)
    return(0);
  memset(pBuff,0,sizeof(FileSize));

  // copy header into buffer
  ModuleFileHeader *pMFH=(ModuleFileHeader*)pBuff;
  memset(pMFH,0,sizeof(ModuleFileHeader));
  strncpy(pMFH->FileFormat,MOD_FORMAT,sizeof(pMFH->FileFormat));
  strncpy(pMFH->Title,pModule->szTitle,sizeof(pMFH->Title));
  strncpy(pMFH->Version,pModule->szVersion,sizeof(pMFH->Version));
  strncpy(pMFH->PartNumber,pModule->szPartNumber,sizeof(pMFH->PartNumber));
  strncpy(pMFH->Author,pModule->szAuthor,sizeof(pMFH->Author));
  strncpy(pMFH->Copyright,pModule->szCopyright,sizeof(pMFH->Copyright));
  strncpy(pMFH->License,pModule->szLicense,sizeof(pMFH->License));
  strncpy(pMFH->Comments,pModule->szComments,sizeof(pMFH->Comments));
  pMFH->Category=pModule->Category;
  pMFH->Hardware=pModule->Hardware;
  pMFH->MemModules=pModule->MemModules;
  pMFH->XMemModules=pModule->XMemModules;
  pMFH->Original=0;
  pMFH->AppAutoUpdate=pModule->AppAutoUpdate;
  pMFH->NumPages=pModule->NumPages;

  // copy each page into buffer that points to this module header
  count=0;
  for (page=0;page<=0xf;page++)
    for (bank=1;bank<=4;bank++)
    {
    if (PageMatrix[page][bank-1]==NULL || PageMatrix[page][bank-1]->pModule!=pModule)
      continue;
    ModuleFilePage *pMFP=(ModuleFilePage*)(pBuff+sizeof(ModuleFileHeader)+count*sizeof(ModuleFilePage));
    memset(pMFP,0,sizeof(ModuleFilePage));
    strncpy(pMFP->Name,PageMatrix[page][bank-1]->szName,sizeof(pMFP->Name));
    strncpy(pMFP->ID,PageMatrix[page][bank-1]->szID,sizeof(pMFP->ID));
    pMFP->Page=PageMatrix[page][bank-1]->Page;
    pMFP->PageGroup=PageMatrix[page][bank-1]->PageGroup;
    pMFP->Bank=PageMatrix[page][bank-1]->Bank;
    pMFP->BankGroup=PageMatrix[page][bank-1]->BankGroup;
    pMFP->Ram=PageMatrix[page][bank-1]->fRAM;
    pMFP->WriteProtect=PageMatrix[page][bank-1]->fWriteProtect;
    pMFP->FAT=PageMatrix[page][bank-1]->fFAT;
    pack_image(PageMatrix[page][bank-1]->Image,pMFP->Image);
    count++;
    }

  // write file
  DWORD SizeWritten;
  HANDLE hFile;
  hFile=CreateFile(pszFullPath,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
  if (hFile==INVALID_HANDLE_VALUE)
    {
    free(pBuff);
    return(0);
    }
  WriteFile(hFile,pBuff,FileSize,&SizeWritten,NULL);
  CloseHandle(hFile);
  free(pBuff);
  if (FileSize!=SizeWritten)
    return(0);
#endif
  return(1);
  }


/****************************/
void Chp41::UnloadMOD(ModuleHeader *pModule) {

    if (pModule==NULL)
        return;
    // free associated ROM pages
    uint page,bank;
    for (page=0;page<=0xf;page++)
        for (bank=1;bank<=4;bank++)
            if (PageMatrix[page][bank-1]!=NULL && PageMatrix[page][bank-1]->pModule==pModule)
            {
                FreePage(page,bank);
                active_bank[page]=1;
            }
    // back out any hardware that this module contains
    if (MemModules>=pModule->MemModules)
        MemModules-=pModule->MemModules;
    else
        MemModules=0;
    if (XMemModules>=pModule->XMemModules)
        XMemModules-=pModule->XMemModules;
    else
        XMemModules=0;
    switch (pModule->Hardware)
    {
    case HARDWARE_PRINTER:
        fPrinter=-1;
        break;
    case HARDWARE_CARDREADER:
        fCardReader=-1;
        break;
    case HARDWARE_TIMER:
        DeInitTimer();      // clears fTimer
        break;
    case HARDWARE_WAND:
        fWand=-1;
        break;
    case HARDWARE_HPIL:
        fHPIL=-1;
        break;
    case HARDWARE_INFRARED:
        fInfrared=-1;
        break;
    case HARDWARE_HEPAX:
    case HARDWARE_NONE:
    default:
        break;
    }
    // delete pointer from list
    for (int i = 0; i< ModuleList.size(); i++)
    {
        if (pModule==ModuleList.at(i))
        {
            ModuleList.removeAt(i);
            delete pModule;
            return;
        }
    }
}


/****************************/
// Frees memory used by page and sets page pointer
/****************************/
void Chp41::FreePage(
        uint page,
        uint bank)
{
    if (PageMatrix[page][bank-1]==NULL)
        return;
    if (PageMatrix[page][bank-1]->pAltPage!=NULL)
        delete PageMatrix[page][bank-1]->pAltPage;
    delete PageMatrix[page][bank-1];
    PageMatrix[page][bank-1]=NULL;
    pCurPage=PageMatrix[CurPage][active_bank[CurPage]-1];
}


/****************************/
// Loads the config file (.LOD) and executes its commands
// returns 0 if successful, or number of errors
/****************************/
bool Chp41::LoadConfig(QXmlStreamReader *xmlIn) {

    if (xmlIn->readNextStartElement() && (xmlIn->name() == "config") ) {
        if (xmlIn->readNextStartElement() && xmlIn->name() == "modules" ) {
            while (xmlIn->readNextStartElement() && xmlIn->name() == "module" ) {
                QString _module = xmlIn->attributes().value("filename").toString();
                int _slotID = xmlIn->attributes().value("slot").toString().toInt();
                bool _custom = xmlIn->attributes().value("custom").toString()=="true" ? true:false;
                ModuleHeader *pModuleNew;
                int nRes=0;
                if  (_custom) {
                    if (xmlIn->readNextStartElement() && xmlIn->name() == "datahex" ) {
                        QByteArray ba = QByteArray::fromHex(xmlIn->readElementText().replace(QRegExp("......:"),"").toLatin1());
                        nRes = LoadMOD(pModuleNew,_module.toLatin1().data(),ba);
                    }
                }
                else {
                    nRes=LoadMOD(pModuleNew,P_RES(_module.toLatin1().data()));
                }
                //Returns 0 for success, 1 for open fail, 2 for read fail, 3 for invalid file, 4 for load conflict or no space
                qWarning()<<P_RES(_module.toLatin1().data())<<" Loaded:"<<nRes;
                //                qWarning()<<Chp41Mod(P_RES(_module.toLatin1().data())).output_mod_info(1,1);

                if (_slotID>=0) {
                    slot[_slotID].used=true;
                    slot[_slotID].label = pModuleNew->szTitle;
                    slot[_slotID].id = pModuleNew->szFullFileName;
                    slot[_slotID].pModule = pModuleNew;
                    slot[_slotID].custom = _custom;
                    slotChanged = true;
                }

                xmlIn->skipCurrentElement();
            }
        }
        if (xmlIn->readNextStartElement() && xmlIn->name() == "hardlinks" ) {
            fPrinter = xmlIn->attributes().value("printer").toString().toInt();
            qWarning()<<"fprinter="<<fPrinter;
            fCardReader = xmlIn->attributes().value("cardreader").toString().toInt(0,10);
            fTimer = xmlIn->attributes().value("timer").toString().toInt(0,10);
            fWand = xmlIn->attributes().value("wand").toString().toInt(0,10);
            fHPIL = xmlIn->attributes().value("hpil").toString().toInt(0,10);
            fInfrared = xmlIn->attributes().value("infrared").toString().toInt(0,10);
            xmlIn->skipCurrentElement();
        }
    }
    xmlIn->skipCurrentElement();

    return true;
}


/****************************/
// writes out .LOD file
// returns 0 if successful, or number of errors
/****************************/
bool Chp41::SaveConfig(QXmlStreamWriter *xmlOut)
  {
    xmlOut->writeStartElement("config");
    xmlOut->writeStartElement("modules");
    for (int i=0; i< ModuleList.size();i++) {
        ModuleHeader *pModule = ModuleList.at(i);
        //        qWarning()<<pModule->szFullFileName;
        if (!stdModule.contains(pModule->szFullFileName)) {
            xmlOut->writeStartElement("module");
            xmlOut->writeAttribute("title",pModule->szTitle);
            xmlOut->writeAttribute("filename",pModule->szFullFileName);
            int _slotID = -1;
            bool _custom = false;
            for (int j=0;j<4;j++) {
                if (pModule == slot[j].pModule) {
                    _slotID = j;
                    _custom = slot[j].custom;
                }
            }
            xmlOut->writeAttribute("slot",QString("%1").arg(_slotID));
            xmlOut->writeAttribute("custom",_custom ? QString("true"):QString("false"));
            if (_custom) {
                QFile file(slot[_slotID].id);
                file.open(QIODevice::ReadOnly);
                QByteArray ba = file.readAll();
                QString outHex = "\n";
                for (int a=0;a<ba.size();a+=16) {
                    outHex += QString("%1:").arg(a,6,16,QChar('0'))+ba.mid(a,16).toHex()+"\n";
                }
                xmlOut->writeTextElement("datahex",outHex);
            }
            xmlOut->writeEndElement();
        }
    }
    xmlOut->writeEndElement();
        xmlOut->writeStartElement("hardlinks");
            xmlOut->writeAttribute("printer",QString("%1").arg(fPrinter,2,16));
            xmlOut->writeAttribute("cardreader",QString("%1").arg(fCardReader));
            xmlOut->writeAttribute("timer",QString("%1").arg(fTimer));
            xmlOut->writeAttribute("wand",QString("%1").arg(fWand));
            xmlOut->writeAttribute("hpil",QString("%1").arg(fHPIL));
            xmlOut->writeAttribute("infrared",QString("%1").arg(fInfrared));
        xmlOut->writeEndElement();
    xmlOut->writeEndElement();
    return true;

#if 0
  WFile hLodFile;
  HANDLE hTempFile;
  DWORD dwBytesWritten;
  char szBuf[128];

  // get full path
  char szMsgFullPath[_MAX_PATH];
  char szLodFullPath[_MAX_PATH];
  char szAppFullPath[_MAX_PATH],app_drive[_MAX_DRIVE],app_dir[_MAX_DIR];
  char drive[_MAX_DRIVE],dir[_MAX_DIR],fname[_MAX_FNAME],ext[_MAX_EXT];
  ::GetModuleFileName(NULL,szAppFullPath,_MAX_PATH);
  _splitpath(szAppFullPath,app_drive,app_dir,fname,ext);
  _makepath(szMsgFullPath,app_drive,app_dir,"V41TEMP","lod");

  // create a temp file to write out to
  hTempFile=CreateFile(szMsgFullPath,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
  if (hTempFile==INVALID_HANDLE_VALUE)
    {
    AfxMessageBox(CString("Unable to create temporary file: ")+szMsgFullPath);
    return(1);
    }

  // if lod file already exists, copy it to new lod file minus blank, $module, $page, $reg and $get commands and : lines
  _splitpath(pszLodFile,drive,dir,fname,ext);
  _makepath(szLodFullPath,app_drive,app_dir,fname,ext);
  if (hLodFile.Open(szLodFullPath))
    {
    strcpy(szBuf,"\r\n");
    while (hLodFile.GetLine())
      {
      hLodFile.SkipWhiteSpace();
      if (hLodFile.EOLN())
        continue;
      else if (hLodFile.CurrentChar()=='$')
        {
        char *pszToken=hLodFile.GetToken();
        if (0==strcmpi(pszToken,"$PAGE"))
          continue;
        else if (0==strcmpi(pszToken,"$REG"))
          continue;
        else if (0==strcmpi(pszToken,"$MODULE"))
          continue;
        else if (0==strcmpi(pszToken,"$GET"))
          continue;
        }
      else if (hLodFile.CurrentChar()==':')
        continue;
      WriteFile(hTempFile,hLodFile.CurrentLine(),strlen(hLodFile.CurrentLine()),&dwBytesWritten,NULL);
      WriteFile(hTempFile,szBuf,2,&dwBytesWritten,NULL);
      }
    hLodFile.Close();
    }

  // write out MODULE config
  POSITION pos=ModuleList.GetHeadPosition();
  while (pos!=NULL)
    {
    ModuleHeader *pModule=(ModuleHeader *)ModuleList.GetAt(pos);
    char drive[_MAX_DRIVE],dir[_MAX_DIR],fname[_MAX_FNAME],ext[_MAX_EXT];
    _splitpath(pModule->szFullFileName,drive,dir,fname,ext);
    sprintf(szBuf,"$MODULE %s\t\t\t;%s\r\n",fname,pModule->szTitle);
    WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
    if (pModule->AppAutoUpdate)           // update the module file if this attribute is set (MLDL/RAM use)
      SaveMOD(pModule,pModule->szFullFileName);
    ModuleList.GetNext(pos);
    }

  // write out registers
  char Reg14Format[]=":%s %0X%0X%0X%0X%0X%0X%0X%0X%0X%0X%0X%0X%0X%0X%s\r\n";
  char Reg12Format[]=":%s %0X%0X%0X%0X%0X%0X%0X%0X%0X%0X%0X%0X%s\r\n";
  char RAMRegFormat[]=":RAM %03X %02X %02X %02X %02X %02X %02X %02X\r\n";
  char szRegName[20];
  byte *pbReg;

  // CPU regs
  strcpy(szRegName,"A_REG");  pbReg=A_REG;
  sprintf(szBuf,Reg14Format,szRegName,pbReg[13],pbReg[12],pbReg[11],pbReg[10],pbReg[9],pbReg[8],pbReg[7],pbReg[6],pbReg[5],pbReg[4],pbReg[3],pbReg[2],pbReg[1],pbReg[0],"\t\t;CPU A Register (56 bits)");
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  strcpy(szRegName,"B_REG");  pbReg=B_REG;
  sprintf(szBuf,Reg14Format,szRegName,pbReg[13],pbReg[12],pbReg[11],pbReg[10],pbReg[9],pbReg[8],pbReg[7],pbReg[6],pbReg[5],pbReg[4],pbReg[3],pbReg[2],pbReg[1],pbReg[0],"\t\t;CPU B Register (56 bits)");
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  strcpy(szRegName,"C_REG");  pbReg=C_REG;
  sprintf(szBuf,Reg14Format,szRegName,pbReg[13],pbReg[12],pbReg[11],pbReg[10],pbReg[9],pbReg[8],pbReg[7],pbReg[6],pbReg[5],pbReg[4],pbReg[3],pbReg[2],pbReg[1],pbReg[0],"\t\t;CPU C Register (56 bits)");
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  strcpy(szRegName,"M_REG");  pbReg=M_REG;
  sprintf(szBuf,Reg14Format,szRegName,pbReg[13],pbReg[12],pbReg[11],pbReg[10],pbReg[9],pbReg[8],pbReg[7],pbReg[6],pbReg[5],pbReg[4],pbReg[3],pbReg[2],pbReg[1],pbReg[0],"\t\t;CPU M Register (56 bits)");
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  strcpy(szRegName,"N_REG");  pbReg=N_REG;
  sprintf(szBuf,Reg14Format,szRegName,pbReg[13],pbReg[12],pbReg[11],pbReg[10],pbReg[9],pbReg[8],pbReg[7],pbReg[6],pbReg[5],pbReg[4],pbReg[3],pbReg[2],pbReg[1],pbReg[0],"\t\t;CPU N Register (56 bits)");
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);

  sprintf(szBuf,":G_REG %02X\t\t\t;CPU G Register (8 bits)\r\n",G_REG);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  sprintf(szBuf,":F_REG %02X\t\t\t;CPU Flag Out Register for Beeper (8 bits)\r\n",F_REG);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  sprintf(szBuf,":ST_REG %02X\t\t\t;CPU STATUS Register (14 bits)\r\n",ST_REG);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  sprintf(szBuf,":Q_REG %0X\t\t\t;CPU Q Pointer (0-D)\r\n",Q_REG);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  sprintf(szBuf,":P_REG %0X\t\t\t;CPU P Pointer (0-D)\r\n",P_REG);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  sprintf(szBuf,":KEY_REG %02X\t\t\t;CPU KEY Register (8 bits)\r\n",KEY_REG);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  sprintf(szBuf,":PT_REG %d\t\t\t;CPU Active Pointer P (0) or Q (1)\r\n",(PT_REG==&Q_REG));
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  sprintf(szBuf,":XST_REG %02X\t\t\t;CPU XST Register (6 bits)\r\n",XST_REG);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  sprintf(szBuf,":FI_REG %04X\t\t\t;CPU FI Register (14 bits)\r\n",FI_REG);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  sprintf(szBuf,":CARRY %0X\t\t\t;CPU CARRY Flag (0-1)\r\n",CARRY);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  sprintf(szBuf,":KEYDOWN %0X\t\t\t;CPU KEYDOWN Flag (0-1)\r\n",KEYDOWN);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  sprintf(szBuf,":BATTERY %0X\t\t\t;CPU BATTERY Flag (0-1)\r\n",BATTERY);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  sprintf(szBuf,":PC_REG %04X\t\t\t;CPU Program Counter (16 bits)\r\n",PC_REG);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  sprintf(szBuf,":RET_STK0 %04X\t\t\t;CPU Return Stack 0 Register (16 bits)\r\n",RET_STK0);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  sprintf(szBuf,":RET_STK1 %04X\t\t\t;CPU Return Stack 1 Register (16 bits)\r\n",RET_STK1);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  sprintf(szBuf,":RET_STK2 %04X\t\t\t;CPU Return Stack 2 Register (16 bits)\r\n",RET_STK2);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  sprintf(szBuf,":RET_STK3 %04X\t\t\t;CPU Return Stack 3 Register (16 bits)\r\n",RET_STK3);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);

  // display registers
  strcpy(szRegName,"DIS_A_REG");  pbReg=DIS_A_REG;
  sprintf(szBuf,Reg12Format,szRegName,pbReg[11],pbReg[10],pbReg[9],pbReg[8],pbReg[7],pbReg[6],pbReg[5],pbReg[4],pbReg[3],pbReg[2],pbReg[1],pbReg[0],"\t\t;LCD A Register (12 bits - bit 8 for starburst 1-12)");
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  strcpy(szRegName,"DIS_B_REG");  pbReg=DIS_B_REG;
  sprintf(szBuf,Reg12Format,szRegName,pbReg[11],pbReg[10],pbReg[9],pbReg[8],pbReg[7],pbReg[6],pbReg[5],pbReg[4],pbReg[3],pbReg[2],pbReg[1],pbReg[0],"\t\t;LCD B Register (48 bits - bits 7-4 for starburst 1-12)");
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  strcpy(szRegName,"DIS_C_REG");  pbReg=DIS_C_REG;
  sprintf(szBuf,Reg12Format,szRegName,pbReg[11],pbReg[10],pbReg[9],pbReg[8],pbReg[7],pbReg[6],pbReg[5],pbReg[4],pbReg[3],pbReg[2],pbReg[1],pbReg[0],"\t\t;LCD C Register (48 bits - bits 3-0 for starburst 1-12)");
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  sprintf(szBuf,":DIS_ANNUN_REG %03X\t\t;LCD Annunciator Register (12 bits)\r\n",DIS_ANNUN_REG);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);

  // state vars
  sprintf(szBuf,":sleep_mode %0X\t\t\t;Sleep State: 0=Awake, 1=Light, 2=Deep\r\n",eSleepMode);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  sprintf(szBuf,":dis_mode %0X\t\t\t;LCD On/Off\r\n",DisplayOn);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  sprintf(szBuf,":hex_mode %01X\t\t\t;CPU HEX Mode On/Off\r\n",(BASE==16));
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  sprintf(szBuf,":perph_in_control %0X\t\t;Peripheral On/Off\r\n",perph_in_control);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  sprintf(szBuf,":perph_selected %02X\t\t;Peripheral Value (00-FF)\r\n",perph_selected);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  sprintf(szBuf,":ram_selected %03X\t\t;RAM Register Selected (000-3FF)\r\n",ram_selected);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  strcpy(szRegName,"active_bank");  pbReg=active_bank;
  sprintf(szBuf,":%s %0X%0X%0X%0X%0X%0X%0X%0X%0X%0X%0X%0X%0X%0X%0X%0X\t;Enabled Bank Array (16 bits)\r\n",szRegName,pbReg[15],pbReg[14],pbReg[13],pbReg[12],pbReg[11],pbReg[10],pbReg[9],pbReg[8],pbReg[7],pbReg[6],pbReg[5],pbReg[4],pbReg[3],pbReg[2],pbReg[1],pbReg[0]);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);

  // ram registers
  for (uint RegIndex=0;RegIndex<MAX_RAM;RegIndex++)
    {
    if (RamExist(RegIndex))
      {
      sprintf(szBuf,RAMRegFormat,RegIndex,pRAM[RegIndex].Reg[6],pRAM[RegIndex].Reg[5],pRAM[RegIndex].Reg[4],pRAM[RegIndex].Reg[3],pRAM[RegIndex].Reg[2],pRAM[RegIndex].Reg[1],pRAM[RegIndex].Reg[0]);
      WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
      }
    }

  // timer registers
  SaveTimer();        // converts values
  strcpy(szRegName,"CLOCK_A");  pbReg=CLK_A;
  sprintf(szBuf,Reg14Format,szRegName,pbReg[13],pbReg[12],pbReg[11],pbReg[10],pbReg[9],pbReg[8],pbReg[7],pbReg[6],pbReg[5],pbReg[4],pbReg[3],pbReg[2],pbReg[1],pbReg[0],"\t\t;Timer CLOCK A Register (56 bits)");
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  strcpy(szRegName,"CLOCK_B");  pbReg=CLK_B;
  sprintf(szBuf,Reg14Format,szRegName,pbReg[13],pbReg[12],pbReg[11],pbReg[10],pbReg[9],pbReg[8],pbReg[7],pbReg[6],pbReg[5],pbReg[4],pbReg[3],pbReg[2],pbReg[1],pbReg[0],"\t\t;Timer CLOCK B Register (56 bits)");
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  strcpy(szRegName,"ALARM_A");  pbReg=ALM_A;
  sprintf(szBuf,Reg14Format,szRegName,pbReg[13],pbReg[12],pbReg[11],pbReg[10],pbReg[9],pbReg[8],pbReg[7],pbReg[6],pbReg[5],pbReg[4],pbReg[3],pbReg[2],pbReg[1],pbReg[0],"\t\t;Timer ALARM A Register (56 bits)");
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  strcpy(szRegName,"ALARM_B");  pbReg=ALM_B;
  sprintf(szBuf,Reg14Format,szRegName,pbReg[13],pbReg[12],pbReg[11],pbReg[10],pbReg[9],pbReg[8],pbReg[7],pbReg[6],pbReg[5],pbReg[4],pbReg[3],pbReg[2],pbReg[1],pbReg[0],"\t\t;Timer ALARM B Register (56 bits)");
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  strcpy(szRegName,"TIMER_SCR_A");  pbReg=SCR_A;
  sprintf(szBuf,Reg14Format,szRegName,pbReg[13],pbReg[12],pbReg[11],pbReg[10],pbReg[9],pbReg[8],pbReg[7],pbReg[6],pbReg[5],pbReg[4],pbReg[3],pbReg[2],pbReg[1],pbReg[0],"\t\t;Timer SCRATCH A Register (56 bits)");
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  strcpy(szRegName,"TIMER_SCR_B");  pbReg=SCR_B;
  sprintf(szBuf,Reg14Format,szRegName,pbReg[13],pbReg[12],pbReg[11],pbReg[10],pbReg[9],pbReg[8],pbReg[7],pbReg[6],pbReg[5],pbReg[4],pbReg[3],pbReg[2],pbReg[1],pbReg[0],"\t\t;Timer SCRATCH B Register (56 bits)");
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  strcpy(szRegName,"INT_TIMER_A");  pbReg=INTV_TV;
  sprintf(szBuf,Reg14Format,szRegName,pbReg[13],pbReg[12],pbReg[11],pbReg[10],pbReg[9],pbReg[8],pbReg[7],pbReg[6],pbReg[5],pbReg[4],pbReg[3],pbReg[2],pbReg[1],pbReg[0],"\t\t;Timer INTERVAL A Register (56 bits but only 20 used)");
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  strcpy(szRegName,"INT_TIMER_B");  pbReg=INTV_CNT;
  sprintf(szBuf,Reg14Format,szRegName,pbReg[13],pbReg[12],pbReg[11],pbReg[10],pbReg[9],pbReg[8],pbReg[7],pbReg[6],pbReg[5],pbReg[4],pbReg[3],pbReg[2],pbReg[1],pbReg[0],"\t\t;Timer INTERVAL A Register (56 bits but only 20 used)");
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  strcpy(szRegName,"ACC_FACTOR");  pbReg=ACC_F;
  sprintf(szBuf,Reg14Format,szRegName,pbReg[13],pbReg[12],pbReg[11],pbReg[10],pbReg[9],pbReg[8],pbReg[7],pbReg[6],pbReg[5],pbReg[4],pbReg[3],pbReg[2],pbReg[1],pbReg[0],"\t\t;Timer ACCURACY FACTOR Register");
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  strcpy(szRegName,"TIMER_STATUS");  pbReg=TMR_S;
  sprintf(szBuf,Reg14Format,szRegName,pbReg[13],pbReg[12],pbReg[11],pbReg[10],pbReg[9],pbReg[8],pbReg[7],pbReg[6],pbReg[5],pbReg[4],pbReg[3],pbReg[2],pbReg[1],pbReg[0],"\t\t;Timer STATUS Register");
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);
  sprintf(szBuf,":TIMER_SEL %0X\t\t\t;Timer Selected A or B (0-1)\r\n",!TimerSelA);
  WriteFile(hTempFile,szBuf,strlen(szBuf),&dwBytesWritten,NULL);

  CloseHandle(hTempFile);
  DeleteFile(szLodFullPath);
  BOOL bRes=MoveFile(szMsgFullPath,szLodFullPath);
  if (bRes==false)
    {
    CString sMsg;
    sMsg.Format("Unable to rename temporary file: %s to %s",szMsgFullPath,szLodFullPath);
    AfxMessageBox(sMsg);
    return(1);
    }
#endif
  return(0);
  }


/****************************/
// This decoder returns the type of the current byte
// based on its value and the previous byte types
/****************************/
enum
  {
  FIRST,          // unknown previous
  SINGLE,         // single byte inst
  DOUBLE1,        // first byte of double byte inst
  DOUBLE2,        // 2 of 2
  TRIPLE1,        // 1 of 3
  TRIPLE2,        // 2 of 3
  TRIPLE3,        // 3 of 3
  TEXT,           // alpha string
  JUMP_TEXT,      // first byte of gto, xeq, w
  GLOBAL1,        // first byte of global
  GLOBAL2,        // second byte of global
  GLOBAL_LBL,     // third byte of label
  GLOBAL_END,     // third byte of end
  GLOBAL_KEYCODE  // fourth byte of label
  };
/****************************/
int Chp41::DecodeUCByte(
  int PrevType,
  byte CurrByte)
  {
  static int TextCount=0;
  int CurrType;

  switch (PrevType)
    {
    case FIRST:
    case SINGLE:
      {
      if ((CurrByte>=0x90 && CurrByte<=0xBF) || (CurrByte>=0xCE && CurrByte<=0xCF))  // 2 byte
        CurrType=DOUBLE1;
      else if(CurrByte>=0xD0 && CurrByte<=0xEF)       // GTO, XEQ 3 byte
        CurrType=TRIPLE1;
      else if (CurrByte>=0xF0 && CurrByte<=0xFF)      // text
        {
        TextCount=CurrByte&0x0F;
        CurrType=TEXT;
        }
      else if (CurrByte>=0x1D && CurrByte<=0x1F)      // GTO, XEQ, W text
        CurrType=JUMP_TEXT;
      else if (CurrByte>=0xC0 && CurrByte<=0xCD)      // global
        CurrType=GLOBAL1;
      else
        CurrType=SINGLE;
      break;
      }

    case DOUBLE1:
      {
      CurrType=DOUBLE2;
      break;
      }
    case DOUBLE2:
      {
      CurrType=DecodeUCByte(FIRST,CurrByte);
      break;
      }

    case TRIPLE1:
      {
      CurrType=TRIPLE2;
      break;
      }
    case TRIPLE2:
      {
      CurrType=TRIPLE3;
      break;
      }
    case TRIPLE3:
      {
      CurrType=DecodeUCByte(FIRST,CurrByte);
      break;
      }

    case TEXT:
      {
      if (TextCount)
        {
        TextCount--;
        CurrType=TEXT;
        }
      else
        CurrType=DecodeUCByte(FIRST,CurrByte);
      break;
      }
    case JUMP_TEXT:
      {
      if (CurrByte>0xF0)
        {
        TextCount=CurrByte&0x0F;
        CurrType=TEXT;
        }
      else      // no text after
        CurrType=DecodeUCByte(FIRST,CurrByte);
      break;
      }

    case GLOBAL1:
      {
      CurrType=GLOBAL2;
      break;
      }
    case GLOBAL2:
      {
      if (CurrByte>=0xF0)       // its a LBL
        {
        TextCount=CurrByte&0x0F;
        CurrType=GLOBAL_LBL;
        }
      else                      // its the END
        CurrType=GLOBAL_END;
      break;
      }
    case GLOBAL_LBL:
      {
      if (TextCount)
        {
        TextCount--;
        CurrType=GLOBAL_KEYCODE;
        }
      else
        CurrType=DecodeUCByte(FIRST,CurrByte);
      break;
      }
    case GLOBAL_END:
      {
      CurrType=DecodeUCByte(FIRST,CurrByte);
      break;
      }
    case GLOBAL_KEYCODE:
      {
      CurrType=DecodeUCByte(TEXT,CurrByte);
      break;
      }

    }
  return(CurrType);
  }


/****************************/
// returns the relative offset between the two
/****************************/
void Chp41::CalcOffset(
  int LowReg,
  int LowByte,
  int HighReg,
  int HighByte,
  int &RegOff,
  int &ByteOff)
  {
  if (HighReg==0 && HighByte==0)   // this means there is nothing at the high addr
    {
    RegOff=0;
    ByteOff=0;
    }
  else
    {
    RegOff=HighReg-LowReg;
    ByteOff=HighByte-LowByte;
    if (ByteOff<0)
      {
      RegOff--;
      ByteOff+=7;
      }
    }
  }


/****************************/
// position on current global and this finds the previous global
// and returns its starting byte and register offset and addr
/****************************/
void Chp41::PrevGlobal(
  int CurrReg,
  int CurrByte,
  int &PrevReg,
  int &PrevByte,
  int &RegOff,
  int &ByteOff)
  {
  int RegIndex=CurrReg;
  int ByteIndex=CurrByte;

  // extract the offsets from the current global
  ByteOff=(hp41cpu->pRAM[RegIndex].Reg[ByteIndex]&0x0e)>>1;
  if (ByteOff>=7)         // just in case something is corrupted
    ByteOff=0;
  RegOff=(hp41cpu->pRAM[RegIndex].Reg[ByteIndex]&0x01)<<8;
  if (ByteIndex==0)
    {
    ByteIndex=6;
    RegIndex--;
    }
  else
    ByteIndex--;
  RegOff=RegOff | hp41cpu->pRAM[RegIndex].Reg[ByteIndex];

  // calc the absolute address of the prev global
  if (RegOff==0 && ByteOff==0)      // if there is no prev global
    {
    PrevReg=0;
    PrevByte=0;
    }
  else
    {
    PrevReg=CurrReg+RegOff;
    PrevByte=CurrByte+ByteOff;
    if (PrevByte>=7)
      {
      PrevReg++;
      PrevByte-=7;
      }
    }
  }


/****************************/
// Loads user code program
// returns 1 for success
/****************************/
int Chp41::GetUserCode(
  char *pszUCFile,
  char *pszError)
  {
#if 0
  HANDLE hUCFile;   // file handle
  DWORD FileSize; // size of file
  DWORD BytesRead;
  byte *pBuffer;    // program loaded into buffer
  int ProgSize;     // size of program in bytes up to and including any END inst
  int EndAddr;      // the .END. address
  int FreeAddr;     // bottom of available free space
  int ByteType;     // type of current byte
  int RegIndex;     // register index for loop
  int ByteIndex;    // byte index for loop
  int RegOff;       // register offset to previous global
  int ByteOff;      // byte offset to previous global
  int PrevReg;      // register where last global starts (or start of program memory)
  int PrevByte;     // byte where last global starts
  int i;            // loop counter

  // open file and copy it to a buffer
  hUCFile=CreateFile(pszUCFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
  if (hUCFile==INVALID_HANDLE_VALUE)
    {
    strcpy(pszError,"Unable to open file");
    return(0);
    }
  FileSize=GetFileSize(hUCFile,NULL);
  pBuffer=(byte*)malloc(FileSize);
  ReadFile(hUCFile,pBuffer,FileSize,&BytesRead,NULL);
  CloseHandle(hUCFile);
  if (FileSize!=BytesRead)
    {
    strcpy(pszError,"Error reading from file");
    free(pBuffer);
    return(0);
    }

  EndAddr=((pRAM[0x00d].Reg[1]&0x0f)<<8) | pRAM[0x00d].Reg[0];
  PrevGlobal(EndAddr,2,PrevReg,PrevByte,RegOff,ByteOff);

  // get exact program size - there can be junk after the END instruction
  ByteType=FIRST;
  for (ProgSize=0;ProgSize<(int)FileSize;ProgSize++)
    {
    if (ByteType==GLOBAL_END)
      break;
    ByteType=DecodeUCByte(ByteType,pBuffer[ProgSize]);
    }

  // find bottom of available memory - key assignments have 0xf0 in byte 6 - Zenrom pg 37-38
  FreeAddr=EndAddr;
  while ((hp41cpu->pRAM[FreeAddr-1].Reg[6]!=0xf0) && (FreeAddr!=0x0c0))
    FreeAddr--;

  // make sure there is enough space - not counting any bytes available on register with .end.
  if (ProgSize>(EndAddr-FreeAddr)*7)
    {
    strcpy(pszError,"Not enough free registers");
    free(pBuffer);
    return(0);
    }

  // set starting location
  RegIndex=EndAddr;
  for (ByteIndex=2;ByteIndex<6;ByteIndex++)  // bytes 2-0 were used by .END.
    {
    if (pRAM[RegIndex].Reg[ByteIndex+1])      // if non null
      break;                                 // cant use it - stop here
    }

  // move program to RAM
  ByteType=FIRST;
  for (i=0;i<ProgSize;i++)
    {
    ByteType=DecodeUCByte(ByteType,pBuffer[i]);

    // fixup global chain as we go
    switch (ByteType)
      {
      case GLOBAL1:          // if global, link it into previous global
        {
        CalcOffset(RegIndex,ByteIndex,PrevReg,PrevByte,RegOff,ByteOff);
        PrevReg=RegIndex;
        PrevByte=ByteIndex;
        pBuffer[i]=0xc0 | (ByteOff<<1) | ((RegOff&0x100)>>8);
        break;
        }
      case GLOBAL2:
        {
        pBuffer[i]=RegOff&0xff;
        break;
        }
      case GLOBAL_LBL:
        {
        break;
        }
      case GLOBAL_END:
        {
        pBuffer[i]=0x0d;
        break;
        }
      case GLOBAL_KEYCODE:             // clear any key assignment from global label
        {
        pBuffer[i]=0;
        break;
        }
      }

    // copy byte
    if (RamExist(RegIndex))                      // just to be safe
      pRAM[RegIndex].Reg[ByteIndex]=pBuffer[i];

    // go to next location
    if (ByteIndex==0)       // go to next ram reg
      {
      ByteIndex=6;
      RegIndex--;
      }
    else
      ByteIndex--;
    }
  free(pBuffer);

  for (i=ByteIndex;i>=0;i--)         // null out the rest of this register
    pRAM[RegIndex].Reg[i]=0;
  if (ByteIndex<2)                   // grab another register for if necessary
    {
    RegIndex--;
    ByteIndex=6;
    for (i=ByteIndex;i>=0;i--)       // null out this register too
      pRAM[RegIndex].Reg[i]=0;
    }

  // create new .END.
  // see Zenrom pg 33-34
  CalcOffset(RegIndex,2,PrevReg,PrevByte,RegOff,ByteOff);
  pRAM[RegIndex].Reg[2]=0xc0 | (ByteOff<<1) | ((RegOff&0x100)>>8);
  pRAM[RegIndex].Reg[1]=RegOff&0xff;
  pRAM[RegIndex].Reg[0]=0x2d;

  // save new .END. address
  pRAM[0x00d].Reg[1]=(pRAM[0x00d].Reg[1]&0xf0) | ((RegIndex>>8)&0x0f);
  pRAM[0x00d].Reg[0]=RegIndex&0xff;
#endif
  return(1);
  }


/****************************/
// traverse catalog and return labels one at a time (reverse order)
// returns 0 when there are no more
/****************************/
enum              // for LoopStatus variable in Catalog1()
  {
  FIND,           // looking for a LBL
  DONE,           // finished decoding LBL
  NEXT            // go to next global
  };
/****************************/
int Chp41::Catalog1(         // returns 0 when no more labels
  flag &fFirst,             // set to 1 for the first invocation
  Cat1Label *pLbl)
  {
  static int CurrReg;
  static int CurrByte;
  static int LastEndReg;    // save the last END addr
  static int LastEndByte;
  int ByteType;     // type of current byte
  int RegIndex;     // register index for loop
  int ByteIndex;    // byte index for loop
  int RegOff;       // register offset to previous global
  int ByteOff;      // byte offset to previous global
  int PrevReg;      // register where prev global starts (or start of program memory)
  int PrevByte;     // byte where prev global starts
  int LabelIndex;   // counts from 0..n for indexing text string
  int TextCount;    // counts from n..0 for counting text length
  int LoopStatus;   // state variable for loop

  // first time we call this
  if (fFirst)
    {
    int EndAddr=((hp41cpu->pRAM[0x00d].Reg[1]&0x0f)<<8) | hp41cpu->pRAM[0x00d].Reg[0];
    CurrReg=EndAddr;
    CurrByte=2;
    fFirst=false;
    }

  LoopStatus=FIND;
  while (LoopStatus!=DONE)
    {
    if (CurrReg==0 && CurrByte==0)
      return(0);

    // decode current instruction - should be a global LBL or END
    RegIndex=CurrReg;
    ByteIndex=CurrByte;
    ByteType=FIRST;
    LabelIndex=0;
    TextCount=0;
    LoopStatus=FIND;
    while (LoopStatus==FIND)
      {
      ByteType=DecodeUCByte(ByteType,hp41cpu->pRAM[RegIndex].Reg[ByteIndex]);
      switch (ByteType)
        {
        case GLOBAL1:
        case GLOBAL2:
          break;
        case GLOBAL_LBL:
          {
          TextCount=hp41cpu->pRAM[RegIndex].Reg[ByteIndex]&0x0f;
          LabelIndex=0;
          break;
          }
        case GLOBAL_END:
          {
          LoopStatus=NEXT;
          LastEndReg=CurrReg;
          LastEndByte=CurrByte;
          break;
          }
        case GLOBAL_KEYCODE:
          break;
        case TEXT:
          {
          if (TextCount)
            {
            pLbl->szText[LabelIndex++]=hp41cpu->pRAM[RegIndex].Reg[ByteIndex];
            TextCount--;
            }
          if (TextCount==1)
            {
            pLbl->szText[LabelIndex]=0;     // null terminate
            pLbl->StartReg=CurrReg;
            pLbl->StartByte=CurrByte;
            pLbl->EndReg=LastEndReg;
            pLbl->EndByte=LastEndByte;
            LoopStatus=DONE;            // ready to return this value
            }
          break;
          }
        default:    // we have landed on something other than global so get out
          {
          pLbl->szText[0]=0;
          pLbl->StartReg=0;
          pLbl->StartByte=0;
          pLbl->EndReg=0;
          pLbl->EndByte=0;
          LoopStatus=DONE;
          break;
          }
        }

      // next byte
      if (ByteIndex==0)
        {
        ByteIndex=6;
        RegIndex--;
        }
      else
        ByteIndex--;
      }

    // setup for next time around
    PrevGlobal(CurrReg,CurrByte,PrevReg,PrevByte,RegOff,ByteOff);
    CurrReg=PrevReg;
    CurrByte=PrevByte;
    }

  return(1);
  }


/****************************/
// write user code program out
/****************************/
int Chp41::PutUserCode(
  char *pszUCFile,
  char *pszError,
  Cat1Label *pLbl)
  {
#if 0
  HANDLE hUCFile;
  DWORD dwBytesWritten;
  byte *pBuffer;
  int RegIndex;
  int ByteIndex;
  int StopReg;         // points to byte after last valid byte
  int StopByte;
  int ByteType;
  int ProgSize=0;

  pBuffer=(byte*)malloc(pLbl->StartReg*7-pLbl->EndReg*7+20);   // overestimate space needed
  RegIndex=pLbl->StartReg;
  ByteIndex=pLbl->StartByte;
  StopReg=pLbl->EndReg;
  StopByte=pLbl->EndByte;
  ByteType=FIRST;

  // eliminate any null bytes at end of program
  while (pRAM[StopByte==6?StopReg+1:StopReg].Reg[StopByte==6?0:StopByte+1]==0)       // look at previous byte
    {
    if (StopByte==6)
      {
      StopByte=0;
      StopReg++;
      }
    else
      StopByte++;
    }

  // copy program to buffer
  while (RegIndex!=StopReg || ByteIndex!=StopByte)
    {
    // copy byte
    if (RamExist(RegIndex))                      // just to be safe
      pBuffer[ProgSize]=pRAM[RegIndex].Reg[ByteIndex];

    ByteType=DecodeUCByte(ByteType,pBuffer[ProgSize]);
    switch (ByteType)
      {
      case GLOBAL_KEYCODE:             // clear any key assignment from global label
        {
        pBuffer[ProgSize]=0;
        break;
        }
      }

    // next byte
    if (ByteIndex==0)
      {
      ByteIndex=6;
      RegIndex--;
      }
    else
      ByteIndex--;
    ProgSize++;
    }

  // write an END
  pBuffer[ProgSize++]=0xc0;
  pBuffer[ProgSize++]=0;
  pBuffer[ProgSize++]=0x0d;

  // create file and write out buffer
  hUCFile=CreateFile(pszUCFile,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
  if (hUCFile==INVALID_HANDLE_VALUE)
    {
    strcpy(pszError,"Unable to create file");
    free(pBuffer);
    return(0);
    }
  WriteFile(hUCFile,pBuffer,ProgSize,&dwBytesWritten,NULL);
  CloseHandle(hUCFile);
  if (ProgSize!=(int)dwBytesWritten)
    {
    strcpy(pszError,"Error writing to file");
    free(pBuffer);
    return(0);
    }

  free(pBuffer);
#endif
  return(1);
  }


#endif
