#ifndef CASIODISK_H
#define CASIODISK_H

#include <QFile>

#include "pobject.h"

#define SIZE_SECTOR     256
#define SIZE_DIR_ENTRY  16
#define SIZE_RECORD     256

#define SECTORS_FAT     4
#define SECTORS_DIR     12
#define START_FAT       0
#define START_DIR       4
#define START_DATA      16
#define MAX_DIR_ENTRY   (SECTORS_DIR * SIZE_SECTOR / SIZE_DIR_ENTRY)
#define MAX_FILES       16			// number of handles }
#define SIZE_FILE_NAME  (8+3)
#define SIZE_BLOCK      4			// 4 sectors per block }


#define FB_IN_USE   0x8000  // marks a used entry }
#define FB_LAST     0x4000  // marks end of chain }
#define FB_SECTORS  0x3000  // number of last sector in last block }
#define FB_BLOCK    0x0FFF  // number of block (this or next in chain) }


class CcasioDisk: public CPObject
{
    Q_OBJECT
public:
    CcasioDisk(CPObject *parent = 0);
    ~CcasioDisk();

    QString filename;
protected:
    bool SectorWrite(int number);
    bool SectorRead(int number);
    bool DiskClose();
    bool DiskOpen();

    char *secbuf;
    int sectors;	// available number of sectors
    int secnum;	// number of the sector in the 'secbuf', otherwise -1 }
    QFile diskFile;
};

// Structure of directory entry }
typedef struct {
      BYTE kind;			// file type }
      BYTE name[8];         //: array [0..7] of byte;
      BYTE ext[3];          //: array [0..2] of byte;
      BYTE unknown;			// unknown meaning, probably unused }
      BYTE block[2];        // starting block number, MSB first }
      BYTE unused;			// always zero }
    } TDirEntry;




class CcasioDOS:public CcasioDisk {

public:

    enum TStorageProperty {
        spError,
        spFree,
        spOccupied
    };
    enum TDosStatusCode {
        dsNoError,
        dsRenameFailed,		// file of specified new name already exists }
        dsFileNotFound,		// open, delete, rename }
        dsFileNotOpened,	// attempted to access a not opened file }
        dsHandleInUse,		// attempted to reopen an already opened file }
        dsNoRoom,           // no room in the directory or data space }
        dsHandleInvalid,	// invalid file handle }
        dsNoData,           // attempted to read past the last record }
        dsIoError           // error in the Bios unit }
    };

    // file information }
    typedef struct { //    TFileInfo = record
          int dirindex; //: integer;		{ index of associated directory entry,
                         // value -1 when file not opened }
          int nextrec; //: cardinal;		{ next record number }
          int firstsec; //: cardinal;		{ starting sector }
          int lastrec; //: cardinal;		{ last accessed record... }
          int lastsec; //: cardinal;		{ ...and corresponding sector }
          int tag; //: integer;			{ no predefined meaning }
        } TFileInfo;

    CcasioDOS(CPObject *parent = 0);
    ~CcasioDOS();



    bool FormatDisk(void);
    bool CloseDiskFile(int handle);
    CcasioDOS::TDosStatusCode CheckFileHandle(int handle);

    TDosStatusCode DosStatus;
    TFileInfo fileinfo[MAX_FILES]; //: array [0..MAX_FILES-1] of TFileInfo;
    TDirEntry direntrybuf;

    int GetDiskFileTag(int handle);
    void PutDiskFileTag(int handle, int value);
    int DosSecRead(int x, char *data);
    int DosSecWrite(int x, char *data);
    bool MySecRead(int x);
    bool MySecWrite(int x);
    bool DosInit();
    bool DosClose();
    bool WriteDirEntry(char *ptr, int i);
    CcasioDOS::TStorageProperty ReadDirEntry(TDirEntry *dir, int i);
    int FindDirEntry(char *filename);
    bool WriteFatEntry(qint32 x, qint32 y);
    qint32 ReadFatEntry(qint32 x);
    qint32 SizeOfDiskFile(qint32 handle);
    qint32 FatNextSector(qint32 x, bool allocate);
    qint32 FindFreeBlock();
    int OpenDiskFile(qint32 handle, char *filename);
    int CreateDiskFile(qint32 handle, char *filename, BYTE filekind);
    void SeekAbsDiskFile(qint32 handle, qint32 position);
    qint32 ReadDiskFile(qint32 handle, char *data);
    void DeleteDiskFile(char *filename);
    bool FatFreeChain(qint32 x);
    void SeekRelDiskFile(qint32 handle, qint32 offset);
    bool IsEndOfDiskFile(qint32 handle);
    void RenameDiskFile(char *oldname, char *newname);
    qint32 WriteDiskFile(qint32 handle, char *data);

    qint32 GetFreeDiskSpace();
};

#endif // CASIODISK_H
