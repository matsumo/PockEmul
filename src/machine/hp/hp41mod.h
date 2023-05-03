#ifndef HP41MOD_H
#define HP41MOD_H

/*=======================================================================

Author: Warren Furlow (email: warren@furlow.org)

License: PUBLIC DOMAIN - May be freely copied and incorporated into any work

Description:  Describes the structure of the MODULE file for HP-41 ROM images

.MOD File Structure:
These structures define the .MOD file format which replaces the older ROM
image dump formats (ROM, BIN).  MOD format allows the definition of entire
plug-in modules which may be composed of ROM images, RAM, special hardware
etc.  The HP-41C, -CV and -CX base operating system is defined in one MOD
file (ie The CX base includes 4 memory modules, the timer hardware, and
XFuns/XMem registers as well as 6 ROM images).  Additionally, Single Memory,
Quad Memory, XFuns/XMem, XMem, Timer modules can be defined each in their own
MOD file.  Obviously certain configurations do not make sense any more than
with the real hardware and may return an error (ie an HP-41CV AND a Quad
Memory Module).  It is also possible to define MLDL RAM using a blank page.

Strings are null terminated and all unused My_bytes are set to zero.  Fields are
strictly limited to valid values defined below.  Some combinations of values
would make no sense and not represent any actual hardware.
File size=sizeof(ModuleFileHeader)+NumPages*sizeof(ModuleFilePage)
=======================================================================*/

#include <QString>

typedef unsigned char My_byte;
typedef unsigned short word;

#define MOD_FORMAT "MOD1"

/* Module type codes */
#define CATEGORY_UNDEF          0  /* not categorized */
#define CATEGORY_OS             1  /* base Operating System for C,CV,CX */
#define CATEGORY_APP_PAC        2  /* HP Application PACs */
#define CATEGORY_HPIL_PERPH     3  /* any HP-IL related modules and devices */
#define CATEGORY_STD_PERPH      4  /* standard Peripherals: Wand, Printer, Card Reader, XFuns/Mem, Service, Time, IR Printer */
#define CATEGORY_CUSTOM_PERPH   5  /* custom Peripherals: AECROM, CCD, HEPAX, PPC, ZENROM, etc */
#define CATEGORY_BETA           6  /* BETA releases not fully debugged and finished */
#define CATEGORY_EXPERIMENTAL   7  /* test programs not meant for normal usage */
#define CATEGORY_MAX            7  /* maximum CATEGORY_ define value */

/* Hardware codes */
#define HARDWARE_NONE                0  /* no additional hardware specified */
#define HARDWARE_PRINTER             1  /* 82143A Printer */
#define HARDWARE_CARDREADER          2  /* 82104A Card Reader */
#define HARDWARE_TIMER               3  /* 82182A Time Module or HP-41CX built in timer */
#define HARDWARE_WAND                4  /* 82153A Barcode Wand */
#define HARDWARE_HPIL                5  /* 82160A HP-IL Module */
#define HARDWARE_INFRARED            6  /* 82242A Infrared Printer Module */
#define HARDWARE_HEPAX               7  /* HEPAX Module - has special hardware features (write protect, relocation) */
#define HARDWARE_WWRAMBOX            8  /* W&W RAMBOX - has special hardware features (RAM block swap instructions) */
#define HARDWARE_MLDL2000            9  /* MLDL2000 */
#define HARDWARE_CLONIX              10 /* CLONIX-41 Module */
#define HARDWARE_MAX                 10 /* maximum HARDWARE_ define value */

/* relative position codes- do not mix these in a group except ODD/EVEN and UPPER/LOWER */
/* ODD/EVEN, UPPER/LOWER can only place ROMS in 16K blocks */
#define POSITION_MIN      0x1f   /* minimum POSITION_ define value */
#define POSITION_ANY      0x1f   /* position in any port page (8-F) */
#define POSITION_LOWER    0x2f   /* position in lower port page relative to any upper image(s) (8-F) */
#define POSITION_UPPER    0x3f   /* position in upper port page */
#define POSITION_EVEN     0x4f   /* position in any even port page (8,A,C,E) */
#define POSITION_ODD      0x5f   /* position in any odd port page (9,B,D,F) */
#define POSITION_ORDERED  0x6f   /* position sequentially in order of MOD file loading, one image per page regardless of bank */
#define POSITION_MAX      0x6f   /* maximum POSITION_ define value */



/* Module header */
typedef struct
  {
  char FileFormat[5];     /* constant value defines file format and revision */
  char Title[50];         /* the full module name (the short name is the name of the file itself) */
  char Version[10];       /* module version, if any */
  char PartNumber[20];    /* module part number */
  char Author[50];        /* author, if any */
  char Copyright[100];    /* copyright notice, if any */
  char License[200];      /* license terms, if any */
  char Comments[255];     /* free form comments, if any */
  My_byte Category;          /* module category, see codes below */
  My_byte Hardware;          /* defines special hardware that module contains */
  My_byte MemModules;        /* defines number of main memory modules (0-4) */
  My_byte XMemModules;       /* defines number of extended memory modules (0=none, 1=Xfuns/XMem, 2,3=one or two additional XMem modules) */
  My_byte Original;          /* allows validation of original contents: 1=images and data are original, 0=this file has been updated by a user application (data in RAM written back to MOD file, etc) */
  My_byte AppAutoUpdate;     /* tells any application to: 1=overwrite this file automatically when saving other data, 0=do not update */
  My_byte NumPages;          /* the number of pages in this file (0-256, but normally between 1-6) */
  My_byte HeaderCustom[32];  /* for special hardware attributes */
  } ModuleFileHeader;

/* page struct */
typedef struct
  {
  char Name[20];       /* normally the name of the original .ROM file, if any */
  char ID[9];          /* ROM ID code, normally two letters and a number are ID and last letter is revision - if all zeros, will show up as @@@@ */
  My_byte Page;           /* the page that this image must be in (0-F, although 8-F is not normally used)
                          or defines each page's position relative to other images in a page group, see codes below */
  My_byte PageGroup;      /* 0=not grouped, otherwise images with matching PageGroup values (1..8) are grouped according to POSITION code */
  My_byte Bank;           /* the bank that this image must be in (1-4) */
  My_byte BankGroup;      /* 0=not grouped, otherwise images with matching BankGroup values (1..8) are bankswitched with each other */
  My_byte Ram;            /* 0=ROM, 1=RAM - normally RAM pages are all blank if Original=1 */
  My_byte WriteProtect;   /* 0=No or N/A, 1=protected - for HEPAX RAM and others that might support it */
  My_byte FAT;            /* 0=no FAT, 1=has FAT */
  My_byte Image[5120];    /* the image in packed format (.BIN file format) */
  My_byte PageCustom[32]; /* for special hardware attributes */
  } ModuleFilePage;

class Chp41;
class ModuleHeader;

class Chp41Mod {

public:
    Chp41Mod();
    Chp41Mod(QString pszFullPath);
    ~Chp41Mod() {}

    ModuleHeader *pModule;
    QByteArray pBuff;


    int getStatus() { return status;}

    word *read_rom_file(QString FullFileName);
    int write_rom_file(char *FullFileName, word *Rom);
    word *read_bin_file(char *FullFileName,int Page);
    int write_bin_file(char *FullFileName, word *Rom);
    word *read_lst_file(char *FullFileName);
    int write_lst_file(char *FullFileName, word *Rom, int Page);
    int compare_rom_files(char *FullFileName1,char *FullFileName2);
     static void unpack_image(word *Rom, My_byte *Bin);
    void pack_image(word *rom, My_byte *BIN);
    QString output_mod_info(int Verbose, int DecodeFat);
    int extract_roms(char *FullFileName);
    static word compute_checksum(word *Rom);
    void get_rom_id(word *Rom, char *ID);
    void decode_lcdchar(word lcdchar,char *ch,char *punct);
    void decode_fatchar(word tyte,char *ch,char *punct,int *end);



    int LoadMOD(QString pszFullPath);
private:
    int status;

};

/*********************/
const char LCDtoASCII[]=
  {
  '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
  'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\',']', '^', '_',
  ' ', '!', '\"','#', '$', '%', '&', '\'','(', ')', '*', '+', '{', '-', '}', '/',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '~', ';', '<', '=', '>', '?',
  '~', 'a', 'b', 'c', 'd', 'e', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~',
  '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~',
  '~', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
  'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '~', '~', '~', '~', '~',
  };

#endif // HP41MOD_H
