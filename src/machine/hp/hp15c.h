#ifndef HP15C_H
#define HP15C_H


#include "pcxxxx.h"
#include "modelids.h"

class CHPNUT;
class CConnector;
class Cbus;

typedef unsigned char flag;
typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned int uint;
typedef quint64 UINT64;


#define NORMAL 0                      // for keys - normal position
#define MOVE 1                        // for keys - moved position

// Processor performance characteristics
// ProcCycles / ProcInterval=5.8 for a real machine: 578 cycles / 100 ms per interval
// 5780 inst/sec = 1E6 / 173 ms for a halfnut HP-41CX instruction (older models run at 158 ms)
// time in milliseconds between processor runs:
#define DEFAULT_PROC_INTERVAL 50
#define MAX_PROC_INTERVAL 100
#define MIN_PROC_INTERVAL 10
// number of processor cycles to run each time:
#define DEFAULT_PROC_CYCLES 578

#define DEFAULT_INST_SPEED 173        // milliseconds of actual instruction cycle to simulate for Speaker sound
#define DEFAULT_CONTRAST 5            // contrast value
#define DISPLAY_BOOST 75              // this value is necessary for CAT 2 in SST mode to prevent display glitches
#define MAX_RAM 0x400                 // maximum number of ram registers




class Chp15c:public CpcXXXX{

Q_OBJECT

public:
  Chp15c(CPObject *parent=0,Models mod=HP15);
  ~Chp15c();

  CHPNUT *nutcpu;

  bool Chk_Adr(UINT32 *d, UINT32 data);
  bool Chk_Adr_R(UINT32 *d, UINT32 *data);

  virtual bool	init(void);				// initialize
  virtual bool	run(void);					// emulator main
  virtual UINT8 in(UINT8 address);
  virtual UINT8 out(UINT8 address,UINT8 value);
  virtual void TurnON();
  virtual void TurnOFF();
  virtual void BuildContextMenu(QMenu * menu);
  virtual void ComputeKey(KEYEVENT ke = KEY_PRESSED,int scancode=0);

  UINT8 getKey();

  bool LoadConfig(QXmlStreamReader *xmlIn);
  bool SaveConfig(QXmlStreamWriter *xmlOut);
public slots:
  void TurnNext();

private:
  bool turnOnNext;
  int firstkey;
};



#endif // HP15C_H
