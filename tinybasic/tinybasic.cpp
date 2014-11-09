//TODO: OPERATOR conversion to internal code to have an accurate memory usage
//TODO: Manage Editor
//TODO: Check all instructions
//TODO: Speed management

//FIXME: DMS & DEG missing
//FIXME: 4^2^3 produce (4^3)^2 instead of 4^(3^2)

#include <QDebug>
#include <errno.h>
#include <math.h>

#include "tinybasic.h"
#include "pcxxxx.h"
#include "sharp/pc1211.h"
#include "Keyb.h"
#include "Inter.h"



CTinyBasic::CTinyBasic(CPObject *parent):CCPU(parent)
{
    angleMode = DEGREE;
    runMode = RUN;//STANDARD;
    pPC1211 = (Cpc1211 *)parent;
}


bool CTinyBasic::exit()
{
    return true;
}

void CTinyBasic::step()
{
//    qWarning("step");
    loop();
    pPC->pTIMER->state+=10;

}

void CTinyBasic::Load_Internal(QXmlStreamReader *xmlIn)
{
    if (xmlIn->readNextStartElement()) {
        if ( (xmlIn->name()=="cpu") &&
             (xmlIn->attributes().value("model").toString() == "tinybasic")) {
            program_end = program_start + xmlIn->attributes().value("program_end").toString().toInt(0,16);
            QByteArray ba_program = QByteArray::fromBase64(xmlIn->attributes().value("memory").toString().toLatin1());
            memcpy((char *) &program,ba_program.data(),kRamSize);
        }
        xmlIn->skipCurrentElement();
    }
}

void CTinyBasic::save_internal(QXmlStreamWriter *xmlOut)
{
    // save program and  several pointer
    xmlOut->writeStartElement("cpu");
     xmlOut->writeAttribute("model","tinybasic");
     xmlOut->writeAttribute("program_end",QString("%1").arg(program_end-program_start,4,16));
     QByteArray ba_imem((char*)program,kRamSize);
     xmlOut->writeAttribute("memory",ba_imem.toBase64());
     xmlOut->writeEndElement();
}
void CTinyBasic::clearOutput() {
    outputBuffer.clear();
}

void CTinyBasic::outputChar(QByteArray ba) {
    outputBuffer.append(ba);
    qWarning()<<ba;
}




void CTinyBasic::Reset()
{
}

#if 1
////////////////////////////////////////////////////////////////////////////////
// TinyBasic Plus
////////////////////////////////////////////////////////////////////////////////
//
// Authors: Mike Field <hamster@snap.net.nz>
//	    Scott Lawrence <yorgle@gmail.com>
//

#define kVersion "v0.09"

// v0.09: 2012-10-12
//      Fixed directory listings.  FILES now always works. (bug in the SD library)
//      ref: http://arduino.cc/forum/index.php/topic,124739.0.html
//      fixed filesize printouts (added printUnum for unsigned numbers)
//      #defineable baud rate for slow connection throttling
//
// v0.08: 2012-10-02
//      Tone generation through piezo added (TONE, TONEW, NOTONE)
//
// v0.07: 2012-09-30
//      Autorun buildtime configuration feature
//
// v0.06: 2012-09-27
//      Added optional second serial input, used for an external keyboard
//
// v0.05: 2012-09-21
//      CHAIN to load and run a second file
//      RND,RSEED for random stuff
//      Added "!=" for "<>" synonym
//      Added "END" for "STOP" synonym (proper name for the functionality anyway)
//
// v0.04: 2012-09-20
//      DELAY ms   - for delaying
//      PINMODE <pin>, INPUT|IN|I|OUTPUT|OUT|O
//      DWRITE <pin>, HIGH|HI|1|LOW|LO|0
//      AWRITE <pin>, [0..255]
//      fixed "save" appending to existing files instead of overwriting
// 	Updated for building desktop command line app (incomplete)
//
// v0.03: 2012-09-19
//	Integrated Jurg Wullschleger whitespace,unary fix
//	Now available through github
//	Project renamed from "Tiny Basic in C" to "TinyBasic Plus"
//
// v0.02b: 2012-09-17  Scott Lawrence <yorgle@gmail.com>
// 	Better FILES listings
//
// v0.02a: 2012-09-17  Scott Lawrence <yorgle@gmail.com>
// 	Support for SD Library
// 	Added: SAVE, FILES (mostly works), LOAD (mostly works) (redirects IO)
// 	Added: MEM, ? (PRINT)
// 	Quirk:  "10 LET A=B+C" is ok "10 LET A = B + C" is not.
// 	Quirk:  INPUT seems broken?



// IF testing with Visual C, this needs to be the first thing in the file.
//#include "stdafx.h"


char eliminateCompileErrors = 1;  // fix to suppress arduino build errors

#define ARDUINO 0


// hack to let makefiles work with this file unchanged
#ifdef FORCE_DESKTOP
#undef ARDUINO
#endif

////////////////////////////////////////////////////////////////////////////////
// Feature configuration...

// This enables LOAD, SAVE, FILES commands through the Arduino SD Library
// it adds 9k of usage as well.
#define ENABLE_FILEIO 0
//undef ENABLE_FILEIO

// this adds a second serial port for input (keyboard, terminal)
// it adds 1.5k of usage as well.
//#define ENABLE_SECOND_SERIAL 1
#undef ENABLE_SECOND_SERIAL

// this turns on "autorun".  if there's FileIO, and a file "autorun.bas",
// then it will load it and run it when starting up
//#define ENABLE_AUTORUN 1
#undef ENABLE_AUTORUN
#define kAutorunFilename  "autorun.bas"

// this will enable the "TONE", "NOTONE" command using a piezo
// element on the specified pin.  Wire the red/positive/piezo to the kPiezoPin,
// and the black/negative/metal disc to ground.
// it adds 1.5k of usage as well.
#define ENABLE_TONES 0
//#undef ENABLE_TONES
#define kPiezoPin 5

// Sometimes, we connect with a slower device as the console.
// Set your console D0/D1 baud rate here (9600 baud default
#define kConsoleBaud 9600

////////////////////////////////////////////////////////////////////////////////
#if ARDUINO
  // includes, and settings for Arduino-specific functionality
  #if ENABLE_SECOND_SERIAL
  #include <SoftwareSerial.h>
  #define kSS_RX 2
  #define kSS_TX 33

  SoftwareSerial ssSerial( kSS_RX, kSS_TX );
  #endif


  #if ENABLE_FILEIO
  #include <SD.h>

  // Arduino-specific configuration
  // set this to the card select for your SD shield
  #define kSD_CS 10

  #define kSD_Fail  0
  #define kSD_OK    1
  #endif

  // size of our program ram
  #define kRamSize   255

  // for file writing
  #if ENABLE_FILEIO
  File fp;
  #endif


#else
  // Not arduino setup
  #include <stdio.h>



  #if ENABLE_FILEIO
  FILE * fp;
  // functions defined elsehwere
  void cmd_Files( void );
  #endif


#endif


#if ENABLE_FILEIO
  unsigned char * filenameWord(void);

  static boolean sd_is_initialized = false;
  static boolean outToFile = false;
  static boolean inFromFile = false;

#endif



////////////////////////////////////////////////////////////////////////////////
// ASCII Characters
#define CR	'\r'
#define NL	'\n'
#define LF      0x0a
#define TAB	'\t'
#define BELL	'\b'
#define SPACE   ' '
#define SQUOTE  '\''
#define DQUOTE  '\"'
#define CTRLC	0x03
#define CTRLH	0x08
#define CTRLS	0x13
#define CTRLX	0x18


#if ARDUINO
  #define ECHO_CHARS 1
#else
  #define ECHO_CHARS 0
#endif




/***********************************************************/
// Keyword table and constants - the last character has 0x80 added to it
static unsigned char keywords[] = {
    'B','E','E','P'+0x80,
    'B','Y','E'+0x80,
    'I','N','P','U','T'+0x80,
    'I','F'+0x80,
    'F','O','R'+0x80,
    'G','O','T','O'+0x80,
    'G','O','S','U','B'+0x80,
    'L','I','S','T'+0x80,
    'L','E','T'+0x80,
    'L','O','A','D'+0x80,
    'M','E','M'+0x80,
    'N','E','X','T'+0x80,
    'N','E','W'+0x80,
    'P','R','I','N','T'+0x80,
    'P','A','U','S','E'+0x80,
    'P','O','K','E'+0x80,
    'R','U','N'+0x80,
    'R','E','T','U','R','N'+0x80,
    'R','E','M'+0x80,
    'S','T','O','P'+0x80,
    'S','A','V','E'+0x80,
    'U','S','I','N','G'+0x80,
    'F','I','L','E','S'+0x80,
    '?'+ 0x80,
    '\''+ 0x80,
    'D','W','R','I','T','E'+0x80,
    'P','I','N','M','O','D','E'+0x80,
    'D','E','L','A','Y'+0x80,
    'E','N','D'+0x80,
    'R','S','E','E','D'+0x80,
    'C','H','A','I','N'+0x80,
    'R','A','D','I','A','N'+0x80,
    'D','E','G','R','E','E'+0x80,
    'G','R','A','D'+0x80,

    'T','O'+0x80,
    'S','T','E','P'+0x80,

    'S','I','N'+0x80,
    'C','O','S'+0x80,
    'T','A','N'+0x80,
    'A','S','N'+0x80,
    'A','C','S'+0x80,
    'A','T','N'+0x80,
    'L','N'+0x80,
    'L','O','G'+0x80,
    'E','X','P'+0x80,
    'P','E','E','K'+0x80,
    'A','B','S'+0x80,
    'A','R','E','A','D'+0x80,
    'D','R','E','A','D'+0x80,
    'R','N','D'+0x80,
    'S','G','N'+0x80,
    'I','N','T'+0x80,
    'D','E','G'+0x80,
    'D','M','S'+0x80,
    0x18+0x80,      // Square root


    0
};

// by moving the command list to an enum, we can easily remove sections
// above and below simultaneously to selectively obliterate functionality.
enum {
    KW_BEEP=0,
    KW_BYE,
    KW_INPUT,
    KW_IF,
    KW_FOR,
    KW_GOTO,
    KW_GOSUB,
    KW_LIST,
    KW_LET,
    KW_LOAD,
    KW_MEM,
    KW_NEXT,
    KW_NEW,
    KW_PRINT,
    KW_PAUSE,
    KW_POKE,      // 0x10
    KW_RUN,
    KW_RETURN,
    KW_REM,
    KW_STOP,
    KW_SAVE,
    KW_USING,
    KW_FILES,
    KW_QMARK,
    KW_QUOTE,
    KW_DWRITE,
    KW_PINMODE,
    KW_DELAY,
    KW_END,
    KW_RSEED,
    KW_CHAIN,
    KW_RADIAN,
    KW_DEGREE,
    KW_GRAD,

    KW_TO,            // 0x21
    KW_STEP,        // 0x22

    KF_SIN,          // 0x23
    KF_COS,
    KF_TAN,
    KF_ASN,
    KF_ACS,
    KF_ATN,
    KF_LN,
    KF_LOG,
    KF_EXP,
    KF_PEEK,
    KF_ABS,
    KF_AREAD,
    KF_DREAD,
    KF_RND,
    KF_SGN,
    KF_INT,
    KF_DEG,
    KF_DMS,
    KF_SQR,


  KW_DEFAULT /* always the final one*/
};


static unsigned char relop_tab[] = {
    '>','='+0x80,
    '<','>'+0x80,
    '>'+0x80,
    '='+0x80,
    '<','='+0x80,
    '<'+0x80,
    '!','='+0x80,
    0
};

#define RELOP_GE		0
#define RELOP_NE		1
#define RELOP_GT		2
#define RELOP_EQ		3
#define RELOP_LE		4
#define RELOP_LT		5
#define RELOP_NE_BANG	6
#define RELOP_UNKNOWN	7


#define STACK_GOSUB_FLAG 'G'
#define STACK_FOR_FLAG 'F'
static unsigned char table_index;
static int table_lenght;


static const unsigned char okmsg[]            = "OK";
static const unsigned char whatmsg[]          = "What? ";
static const unsigned char howmsg[]           =	"How?";
static const unsigned char sorrymsg[]         = "Sorry!";
static const unsigned char initmsg[]          = "TinyBasic Plus " kVersion;
static const unsigned char memorymsg[]        = " BYTES FREE.";
static const unsigned char breakmsg[]         = "BREAK AT ";
static const unsigned char unimplimentedmsg[] = "UNIMPLEMENTED";
static const unsigned char backspacemsg[]     = "\b \b";
static const unsigned char indentmsg[]        = "    ";
static const unsigned char sderrormsg[]       = "SD card error.";
static const unsigned char sdfilemsg[]        = "SD file error.";
static const unsigned char dirextmsg[]        = "(dir)";
static const unsigned char slashmsg[]         = "/";
static const unsigned char spacemsg[]         = " ";



bool CTinyBasic::init()
{
    inhibitOutput = false;
    runAfterLoad = false;
    triggerRun = false;
    running = false;
    processingInput = false;
    breakFlag = false;
    errorNumber = 0;
    printMode = CTinyBasic::DISPLAY;

    inLIST = false;
    alsoWait = false;
    nextStep = WARMSTART;
    cursorPos = 0;
    inputMode = true;

    waitForRTN = false;
    pauseFlag = false;

    memset(program,0,sizeof(program));
    program_start = program;
    program_end = program_start;
    sp = program+sizeof(program);  // Needed for printnum
    stack_limit = program+sizeof(program)-STACK_SIZE;
    variables_begin = stack_limit - 27*VAR_SIZE;
//    printmsg(initmsg);
//    printnum(variables_begin-program_end);
//    printmsg(memorymsg);

    LoadTable(keywords);

    return true;
}

/***************************************************************************/
void CTinyBasic::ignore_blanks(void)
{
    while(*txtpos == SPACE || *txtpos == TAB)
        txtpos++;
}

// TODO convert instruction to internal code until NL
// Be aware of Strings
void CTinyBasic::convertLine() {

//    return;
    qWarning()<<"CONVERT LINE";

    unsigned char* saved_txtpos = txtpos;
    while (1) {

        qWarning()<<QString("%1:").arg(txtpos[0])<<QString(QByteArray((char*)txtpos,20));
        if (txtpos[0]=='"') {
            qWarning()<<"found quotes";
            txtpos++;
            while(txtpos[0] != '"')
            {
                qWarning()<<"skip "<<txtpos[0];
                if(txtpos[0] == NL) break;
                txtpos++;
            }
        }
        if (txtpos[0]==NL) break;
        if (txtpos[0]==(0x80+KW_REM)) break;

        scantable(keywords,ALL);
        if (table_original) continue;
        if (table_index==KW_DEFAULT ) txtpos++;
        else {

            txtpos-=table_lenght;
            qWarning()<<"found keyword:"<<txtpos[0];

            for (int c=0;c<table_lenght-1;c++) {
                txtpos[0]=0x20;
                txtpos++;
            }
            txtpos[0]= 0x80+table_index;
            if (txtpos[0]==(0x80+KW_REM)) break;
        }
    }
    // remove spaces
    txtpos=saved_txtpos;
    while(1) {
        if (txtpos[0]== (0x80+KW_REM)) {
            txtpos = saved_txtpos;
            return;
        }
        if (txtpos[0] == '"') { // '"' && delim != '\''
            txtpos++;
            while(txtpos[0] != '"')
            {
                if(txtpos[0] == NL) {
                    txtpos=saved_txtpos;
                    return;
                }
                txtpos++;
            }
        }
        if(txtpos[0] == NL) {
            txtpos=saved_txtpos;
            return;
        }
        if (txtpos[0] == 0x20) {
            int i=0;
            while (txtpos[i] != NL) {
                txtpos[i] = txtpos[i+1];
                i++;
            }
            txtpos--;
        }

        txtpos++;
    }


    txtpos=saved_txtpos;
}

void CTinyBasic::LoadTable(unsigned char *table) {
    table_index=0;

    QByteArray ba;
    while(1) {
        // Run out of table entries?
        if(table[0] == 0)
            return;

        // do we match the last character of keywork (with 0x80 added)? If so, return
        if(table[0] & 0x80)
        {
            ba.append(table[0] &0x7F);
            keywordsMap.insert(0x80+table_index,ba);
            ba.clear();
            table_index++;
        }
        else {
            ba.append(table[0]);
        }
        table++;
    }
}


/***************************************************************************/
void CTinyBasic::scantable(unsigned char *table,KEYWORD_TYPE type)
{
//    qWarning()<<"SCANTABLE"<< txtpos[0];
    int i = 0;
    table_original = false;
    table_lenght = 0;
    table_index = 0;
    int offset_begin = 0;
    int offset_end = 0;
    switch (type) {
    case ALL:       offset_begin = 0x00; offset_end = KW_DEFAULT; break;
    case KEYWORD:   offset_begin = 0x00; offset_end = KW_TO-1; break;
    case FOR_TO:    offset_begin = KW_TO; offset_end = KW_TO; break;
    case FOR_STEP:  offset_begin = KW_STEP; offset_end = KW_STEP; break;
    case FUNC:      offset_begin = KF_SIN; offset_end = KW_DEFAULT; break;
    case OPE:       break;
    }
    if ( (txtpos[0]>=0x80+offset_begin) && (txtpos[0]<=0x80+offset_end)) {
        table_index = txtpos[0]-0x80;
        txtpos++;
        table_original = true;
        //        ignore_blanks();
        return;
    }

    while(1)
    {
        // Run out of table entries?
        if(table[0] == 0)
            return;

        // Do we match this character?
        if(txtpos[i] == table[0])
        {
            i++;
            table++;
        }
        else
        {
            // do we match the last character of keywork (with 0x80 added)? If so, return
            if( (txtpos[i]+0x80 == table[0]) || (txtpos[i]=='.') )
            {
                // If we are out of the scope return NOT FOUND
                if ((table_index< offset_begin)||(table_index> offset_end)) {
                    table_index = KW_DEFAULT;
                    return;
                }
                txtpos += i+1;  // Advance the pointer to following the keyword
                table_lenght = i+1;
//                ignore_blanks();
                return;
            }

            // Forward to the end of this keyword
            while((table[0] & 0x80) == 0)
                table++;

            // Now move on to the first character of the next word, and reset the position index
            table++;
            table_index++;
            ignore_blanks();
            i = 0;
        }
    }
}

/***************************************************************************/
void CTinyBasic::pushb(unsigned char b)
{
    sp--;
    *sp = b;
}

/***************************************************************************/
unsigned char CTinyBasic::popb()
{
    unsigned char b;
    b = *sp;
    sp++;
    return b;
}

/***************************************************************************/
void CTinyBasic::printnum(VAR_TYPE num,int size,DEVICE output)
{

    switch (size) {
    case 2: {

        int digits = 0;

        if(num < 0)
        {
            num = -num;
            outchar('-',output);
        }
        do {
            pushb(((int)num)%10+'0');
            num = (int) (num/10);
            digits++;
        }
        while (num > 0);

        while(digits > 0)
        {
            outchar(popb(),output);
            digits--;
        }
    }
        break;
    case 8:{
        char buffer[32];

        memset(buffer, 0x20, sizeof(buffer));
        if (num==0) sprintf(buffer,"0");
        else if (abs(num)<1)         snprintf(buffer, 32, "%11.9E", num);

        else snprintf(buffer, 32, "%.10G", num);
        qWarning()<<buffer;
        bool exp = false;
        bool decimalPt = false;
        bool out = false;
        int expPos = -1;
        for (int digit= 0;digit < 18;digit++) {
            switch (buffer[digit]) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '-': break;
            case 'E': exp = true; expPos =digit;break;
            case '.': decimalPt = true; break;
            case '+': buffer[digit]=' ';break;
            default: out = true;break;
            }
            if (out) break;
            if (exp && (digit == expPos+2)) digit++;   // don't print first Exp digit
            outchar(buffer[digit],output);
        }
        if (!exp && !decimalPt) outchar('.',output);

    }
    }
}

void CTinyBasic::printUnum(unsigned int num)
{
    int digits = 0;

    do {
        pushb(num%10+'0');
        num = num/10;
        digits++;
    }
    while (num > 0);

    while(digits > 0)
    {
        outchar(popb());
        digits--;
    }
}

/***************************************************************************/
unsigned short CTinyBasic::testnum(void)
{
    unsigned short num = 0;
    ignore_blanks();

    while(*txtpos>= '0' && *txtpos <= '9' )
    {
        // Trap overflows
        if(num >= 0xFFFF/10)
        {
            num = 0xFFFF;
            break;
        }

        num = num *10 + *txtpos - '0';
        txtpos++;
    }
    return	num;
}

/***************************************************************************/
void CTinyBasic::printmsgNoNL(const unsigned char *msg,DEVICE output)
{
    while(*msg)
    {
        outchar(*msg,output);
        msg++;
    }
}

/***************************************************************************/
unsigned char CTinyBasic::print_quoted_string(DEVICE output)
{
    int i=0;
    unsigned char delim = *txtpos;
    if(delim != '"' && delim != '\'')
        return 0;
    txtpos++;

    // Check we have a closing delimiter
    while(txtpos[i] != delim)
    {
        if(txtpos[i] == NL)
            return 0;
        i++;
    }

    // Print the characters
    while(*txtpos != delim)
    {
        outchar(*txtpos,output);
        txtpos++;
    }
    txtpos++; // Skip over the last delimiter

    return 1;
}

/***************************************************************************/
void CTinyBasic::printmsg(const unsigned char *msg)
{
    printmsgNoNL(msg);

    line_terminator();
}

/***************************************************************************/
void CTinyBasic::getln(char prompt)
{
//    if (prompt>0) outchar(prompt);
//    txtpos = program_end+sizeof(LINENUM);
    nextStep = GETLN;



    unsigned char c = inchar();
    if (c==0) return;

    switch(c)
    {
    case NL:
        break;
    case CR:
        //                                line_terminator();
        // Terminate all strings with a NL
        txtpos[0] = NL;
        nextStep = GETLN_END;
//        inputMode = false;
        return;
    case K_BS: // BackSpace
        txtpos--;

        break;
    case CTRLH:
        if(txtpos == program_end)
            break;
        txtpos--;

        printmsgNoNL(backspacemsg);
        break;
    case K_LA:
        cursorPos--;
        if (cursorPos<0) cursorPos=0;

        break;
    case K_RA: cursorPos++; if (cursorPos>= commandBuffer.size()) cursorPos=commandBuffer.size();

        break;
    default:
        // We need to leave at least one space to allow us to shuffle the line into order
        if(txtpos == variables_begin-2) {
//            outchar(BELL);
        }
        else
        {
            txtpos[0] = c;
            txtpos++;
//            outchar(c);
            qWarning()<<"Char:"<<c;
        }
    }
}

void CTinyBasic::scanLines(void)
{
    unsigned char *line = program_start;
    lineMap.clear();
    lineDefMap.clear();
    while(1)
    {
        if(line == program_end)
            return ;

        lineMap.insert(((LINENUM *)line)[0],line);

        // Check if a label is present at the begining

        if (line[3]=='"') {
            int i=1;
            QByteArray ba;
            while (line[3+i]!='"') {
                ba.append(line[3+i]);
                i++;
            }
            lineDefMap.insert(ba,line);
        }

        // Add the line length onto the current address, to get to the next line;
        line += line[sizeof(LINENUM)];
    }
}
/***************************************************************************/
unsigned char *CTinyBasic::findline(double e)
{
#if 0
unsigned char *line = program_end;
if (lineMap.contains(linenum))

#else

    bool searchLbl = false;
    if (checkType(&e)==STRING) searchLbl = true;
    else linenum = MIN(999,floor(abs(e)));

    unsigned char *line = program_start;
    while(1)
    {
        if(line == program_end)
            return line;

        if (searchLbl) {
            labelLineNum = 0;
            if (line[3]=='"') {
                int i=1;
                QByteArray ba;
                while (line[3+i]!='"') {
                    ba.append(line[3+i]);
                    i++;
                }
                char *pt = ba.leftJustified(7,0,true).append(0xF5).data();
                if (((VAR_TYPE *)pt)[0] == e) {
                    labelLineNum = ((LINENUM *)line)[0];
                    return line;
                }

            }
        }
        else if(((LINENUM *)line)[0] >= linenum) {
//            linenum = ((LINENUM *)line)[0];
            return line;
        }

        // Add the line length onto the current address, to get to the next line;
        line += line[sizeof(LINENUM)];
    }
#endif
}

/***************************************************************************/
void CTinyBasic::toUppercaseBuffer(void)
{
    unsigned char *c = program_end+sizeof(LINENUM);
    unsigned char quote = 0;

    while(*c != NL)
    {
        // Are we in a quoted string?
        if(*c == quote)
            quote = 0;
        else if(*c == '"' || *c == '\'')
            quote = *c;
        else if(quote == 0 && *c >= 'a' && *c <= 'z')
            *c = *c + 'A' - 'a';
        c++;
    }
}

/***************************************************************************/
void CTinyBasic::printline(DEVICE output)
{
    LINENUM line_num;

    line_num = *((LINENUM *)(list_line));
    list_line += sizeof(LINENUM) + sizeof(char);

    if (output == DISPLAY) waitForRTN = true;
    qWarning()<<"Wait for RTN ";

    // Output the line */
    if (output==PRINTER) {
        if (linenum<100) outchar(0x20,output);
        if (linenum<10) outchar(0x20,output);
    }
    printnum(line_num,2,output);
    outchar(':',output);
    int charNb=0;
    while(*list_line != NL)
    {

        if (output==DISPLAY) {
            outchar(*list_line,output);
        }
        if (output==PRINTER) {
            if (*list_line >= 0x80) {
                QByteArray ba = keywordsMap[*list_line]+" ";
                for(int i=0;i<ba.size();i++) {
                    outchar(ba.at(i),output);
                    charNb++;
                    if (charNb==12) {
                        outchar(NL,output);
                        outchar(' ',output);
                        outchar(' ',output);
                        outchar(' ',output);
                        outchar(' ',output);
                        charNb=0;
                    }

                }
            }
            else {
                outchar(*list_line,output);
                charNb++;
                if (charNb==12) {
                    outchar(NL,output);
                    outchar(' ',output);
                    outchar(' ',output);
                    outchar(' ',output);
                    outchar(' ',output);
                    charNb=0;
                }

            }

        }
        list_line++;
    }
    list_line++;
    line_terminator(output);
}

/***************************************************************************/
VAR_TYPE CTinyBasic::expr5(ExpTYP type)
{
    qWarning()<<"Exp5";
    // fix provided by J?rg Wullschleger wullschleger@gmail.com
    // fixes whitespace and unary operations
    ignore_blanks();

    if( *txtpos == '-' ) {
        txtpos++;
        return -expr5();
    }
    // end fix

//    if(*txtpos == '0')
//    {
//        txtpos++;
//        return 0;
//    }


    if (*txtpos=='"') {
        if (type == NUMERIC) goto expr4_error;
        expAlpha=true;
        txtpos++;
        int i=0;
        QByteArray ba;
        while (*txtpos!='"') {
            ba.append(*txtpos);
            txtpos++;
            i++;
        }
        txtpos++;
        char *pt = ba.leftJustified(7,0,true).append(0xF5).data();
        return ((VAR_TYPE *)pt)[0];
    }

    if((*txtpos >= '0' && *txtpos <= '9') || *txtpos == '.' )
    {
        VAR_TYPE a = 0;
        // TODO Manage float 999.99999E+99. Replace this bloc by atof function
        switch (sizeof(a)) {
        case 2:
            do 	{
                a = a*10 + *txtpos - '0';
                txtpos++;
            } while(*txtpos >= '0' && *txtpos <= '9');
            break;
        case 8: {
            a = atof((const char*)txtpos);
            qWarning()<<"read float:"<<a;
            bool exp = false;
            do {
                txtpos++;
                if (*txtpos=='E') {
                    txtpos++;
                    if (*txtpos=='+' || *txtpos=='-') txtpos++;
                    exp = true;
                }
            } while ( (*txtpos >= '0' && *txtpos <= '9')||
                      *txtpos=='.' ||
                      *txtpos=='E' );
        }
            break;

        }
        return a;
    }

    if (txtpos[0] == 0x19) {
        txtpos++;
        return M_PI;
    }

    // Is it a function or variable reference?
    if ((txtpos[0] >= 'A' && txtpos[0] <= 'Z') ||
            (txtpos[0]==0x18))// Hack for SQR Function.
    {
        VAR_TYPE a;
        // Is it a variable reference (single alpha)
        if (  (txtpos[0]!=0x18)&&(txtpos[1] < 'A' || txtpos[1] > 'Z'))
        {
            unsigned char var = *txtpos;
            expAlpha = false;
            if (txtpos[1]=='$') {
                // manage string var
//                if (type == NUMERIC) goto expr4_error;
                expAlpha=true;
                txtpos++;
            }
            txtpos++;
            if  ( (var=='A') && (*txtpos=='(')) {       // A Array. the index can be an expression
                int ind = expression();
                if (expression_error) goto expr4_error;
                var +=ind - 1;
            }
            a = ((VAR_TYPE *)variables_begin)[var - 'A'];
            // Check variable type
            if (expAlpha && (checkType(&a)==NUMERIC) && (a!=0)) {
                goto expr4_error;
            }
            if (!expAlpha && checkType(&a)==STRING) {
                goto expr4_error;
            }
            return a;
        }

        // Is it a function with a single parameter
        scantable(keywords,FUNC);
        ignore_blanks();
        if(table_index == KW_DEFAULT)
            goto expr4_error;

        unsigned char f = table_index;

        // If not ( analyse next token
        if(*txtpos == '(') {
            //            goto expr4_error;

            txtpos++;
            a = expression();
            if(*txtpos != ')' || expression_error)
                goto expr4_error;
            txtpos++;

        }
        else
            a= expr5();


        switch(f)
        {
        case KF_PEEK:
            return 0;//program[a];
        case KF_ABS:
            if(a < 0)
                return -a;
            return a;
        case KF_SIN:
            return sin(convertToRad(a));
        case KF_COS:
            return cos(convertToRad(a));
        case KF_TAN:
            return tan(convertToRad(a));
        case KF_ASN: {
            double r= convertFromRad(asin(a));
            if (errno==EDOM) goto expr4_error;
            return r;
        }
        case KF_ACS:{
            double r= convertFromRad(acos(a));
            if (errno==EDOM) goto expr4_error;
            return r;
        }
        case KF_ATN:{
            double r= convertFromRad(atan(a));
            if (errno==EDOM) goto expr4_error;
            return r;
        }
        case KF_LN:{
            double r= log(a);
            if (errno==EDOM) goto expr4_error;
            return r;
        }
        case KF_LOG:{
            double r= log10(a);
            if (errno==EDOM) goto expr4_error;
            return r;
        }
        case KF_EXP:
            return exp(a);
        case KF_SGN:
            if (a<0) return -1;
            if (a>0) return 1;
            return 0;
        case KF_INT:
            return floor(a);
        case KF_SQR:
            double r= sqrt(a);
            if (errno==EDOM) goto expr4_error;
            return r;
            //        case FUNC_AREAD:
            //            return analogRead( a );
            //        case FUNC_DREAD:
            //            return digitalRead( a );

            //        case FUNC_RND:
            //            return( random( a ));
        }
    }

    if(*txtpos == '(')
    {
        VAR_TYPE a;
        txtpos++;
        a = expression();
        if(*txtpos != ')')
            goto expr4_error;

        txtpos++;
        return a;
    }

expr4_error:
    errorNumber = 1;
    expression_error = 1;
    return 0;

}

double CTinyBasic::convertToRad(double angle) {

    switch (angleMode) {
    case DEGREE: return (angle *M_PI/180.0);
    case RADIAN: return angle;
    case GRAD:   return (angle *M_PI/200.0);
    default: return angle;
    }

    return angle;
}
double CTinyBasic::convertFromRad(double angle) {

    switch (angleMode) {
    case DEGREE: return (angle *180 /M_PI);
    case RADIAN: return angle;
    case GRAD:   return (angle *200 /M_PI);
    default: return angle;
    }

    return angle;
}

/***************************************************************************/
VAR_TYPE CTinyBasic::expr4(ExpTYP type)
{
    VAR_TYPE a,b;
qWarning()<<"Exp4";
    a = expr5(type);

    ignore_blanks(); // fix for eg:  100 a = a + 1

    if (type == STRING) return a;

    while(1)
    {
        if(*txtpos == '^')
        {
            txtpos++;
            b = expr5();
            a = pow(a,b);
        }
        else
            return a;
    }
}

/***************************************************************************/
VAR_TYPE CTinyBasic::expr3(ExpTYP type)
{
    VAR_TYPE a,b;
qWarning()<<"Exp3";
    a = expr4(type);

    ignore_blanks(); // fix for eg:  100 a = a + 1

    if (type == STRING) return a;

    while(1)
    {
        if(*txtpos == '*')
        {
            txtpos++;
            b = expr4();
            a *= b;
        }
        else if(*txtpos == '/')
        {
            txtpos++;
            b = expr4();
            if(b != 0)
                a /= b;
            else
                expression_error = 1;
        }
        else
            return a;
    }
}

/***************************************************************************/
VAR_TYPE CTinyBasic::expr2(ExpTYP type)
{
    VAR_TYPE a,b;
qWarning()<<"Exp2";
    if(*txtpos == '-' || *txtpos == '+')
        a = 0;
    else
        a = expr3(type);

    if (type==STRING) return a;

    while(1)
    {
        if(*txtpos == '-')
        {
            txtpos++;
            b = expr3();
            a -= b;
        }
        else if(*txtpos == '+')
        {
            txtpos++;
            b = expr3();
            a += b;
        }
        else
            return a;
    }
}
/***************************************************************************/
VAR_TYPE CTinyBasic::expression(ExpTYP type)
{
    VAR_TYPE a,b;

    if (*txtpos=='"') type=STRING;

    a = expr2(type);
    qWarning()<<"expression:"<<a;
    // Check if we have an error
    if(expression_error)	return a;

    //FIXME: scantable seems not working for OPE
    scantable(relop_tab,OPE);
    ignore_blanks();
    if(table_index == RELOP_UNKNOWN)
        return a;

    if ((type == STRING) && (table_index==RELOP_EQ)) {
        b = expr2(type);
        if(a == b) return 1;
        else return 0;
    }

    switch(table_index)
    {
    case RELOP_GE:
        b = expr2();
        if(a >= b) return 1;
        break;
    case RELOP_NE:
    case RELOP_NE_BANG:
        b = expr2();
        if(a != b) return 1;
        break;
    case RELOP_GT:
        b = expr2();
        if(a > b) return 1;
        break;
    case RELOP_EQ:
        b = expr2(type);
        if(a == b) return 1;
        break;
    case RELOP_LE:
        b = expr2();
        if(a <= b) return 1;
        break;
    case RELOP_LT:
        b = expr2();
        if(a < b) return 1;
        break;
    }
    return 0;
}



/***************************************************************************/
void CTinyBasic::loop()
{


#if ENABLE_TONES
        noTone( kPiezoPin );
#endif

        switch (nextStep) {
        case WARMSTART: goto warmstart;break;
        case PROMPT:    goto prompt; break;
        case EXECLINE:  goto execline; break;
        case DIRECT:    goto direct;
        case QHOW:      goto qhow;
        case EXECNEXTLINE:          goto execnextline;
        case INTERPERATEATTXTPOS:   goto interperateAtTxtpos;
        case FILES:     goto files;
        case CHAIN:goto chain;
        case LOAD:goto load;
        case QWHAT:goto qwhat;
        case QSORRY:goto qsorry;
        case ASSIGNMENT: go_ASSIGNMENT(); return; break;
        case LIST_NEXT: go_LIST_NEXT(); return; break;
        case LIST_PREV: go_LIST_PREV(); return; break;
        case UNIMPLEMENTED:goto unimplemented;
        case POKE:goto poke;
        case GETLN:goto getln;
        case GETLN_END:goto getln_end;
        case BEEP: go_BEEP(false); return; break;
        case RUN_NEXT_STATEMENT: goto run_next_statement;
        case INPUT_CR: go_INPUT(); return; break;
        default: break;
        }

warmstart:
    // this signifies that it is running in 'direct' mode.
    current_line = 0;
    sp = program+sizeof(program);
//    printmsg(okmsg);

prompt:
    if (!waitForRTN) inputMode = true;
    qWarning()<<"prompt :inputMode true";

        if( triggerRun ){
          triggerRun = false;
          current_line = program_start;
          nextStep = EXECLINE;
          return;
          goto execline;
        }
        outchar('>');
        txtpos = program_end+sizeof(LINENUM);
getln:
    getln('>');
    return;
getln_end:
//    inputMode = false;
    outputBuffer.clear();
    toUppercaseBuffer();

    txtpos = program_end+sizeof(unsigned short);

    // Find the end of the freshly entered line
    while(*txtpos != NL)
        txtpos++;

    // Move it to the end of program_memory
    {
        unsigned char *dest;
        dest = variables_begin-1;
        while(1)
        {
            *dest = *txtpos;
            if(txtpos == program_end+sizeof(unsigned short))
                break;
            dest--;
            txtpos--;
        }
        txtpos = dest;
    }
    if (processingInput) {
        nextStep=INPUT_CR;
        return;
    }
convertLine();
    if (runMode==RUN) {
        nextStep=DIRECT;
        return;
    }
    // Now see if we have a line number
    linenum = testnum();
    ignore_blanks();
    if(linenum == 0) {
        nextStep = DIRECT; return;
    }
    qWarning()<<"Line number found:"<<linenum;

    if(linenum > 999) {
        errorNumber = 2;
        nextStep = QWHAT;
        return;
    }

    // Convert INSTRUCTIONS TO internal code
    qWarning()<<"Convert Keywords to internal code";
//    convertLine();


    // Find the length of what is left, including the (yet-to-be-populated) line header
    linelen = 0;
    while(txtpos[linelen] != NL)
        linelen++;
    linelen++; // Include the NL in the line length
    linelen += sizeof(unsigned short)+sizeof(char); // Add space for the line number and line length

    qWarning()<< "Buffer:"<<QByteArray((char *)txtpos,linelen).toHex();
    // Now we have the number, add the line header.
    qWarning()<<"Line number found before line header:"<<linenum;
    txtpos -= 3;
    *((unsigned short *)txtpos) = linenum;
    txtpos[sizeof(LINENUM)] = linelen;

    // Merge it into the rest of the program
    start = findline(linenum);
    qWarning()<<"Line number found after findline:"<<linenum;

    // If a line with that number exists, then remove it
    if(start != program_end && *((LINENUM *)start) == linenum)
    {
        qWarning()<<"remove line nb:"<<linenum;
        unsigned char *dest, *from;
        unsigned tomove;

        from = start + start[sizeof(LINENUM)];
        dest = start;

        tomove = program_end - from;
        while( tomove > 0)
        {
            *dest = *from;
            from++;
            dest++;
            tomove--;
        }
        program_end = dest;
    }

    if(txtpos[sizeof(LINENUM)+sizeof(char)] == NL) { // If the line has no txt, it was just a delete
        nextStep = PROMPT;

        return;
        goto prompt;
    }



    // Make room for the new line, either all in one hit or lots of little shuffles
    qWarning()<<"make room of "<<linelen<<" bytes for the new line";
    while(linelen > 0)
    {
        unsigned int tomove;
        unsigned char *from,*dest;
        unsigned int space_to_make;

        space_to_make = txtpos - program_end;

        if(space_to_make > linelen)
            space_to_make = linelen;
        newEnd = program_end+space_to_make;
        tomove = program_end - start;


        // Source and destination - as these areas may overlap we need to move bottom up
        from = program_end;
        dest = newEnd;
        qWarning()<<"move from:"<<from<<" to:"<<dest;
        while(tomove > 0)
        {
            from--;
            dest--;
            *dest = *from;
            tomove--;
        }

        // Copy over the bytes into the new space
        qWarning()<<"copy "<<space_to_make<<" bytes to the new space";
        for(tomove = 0; tomove < space_to_make; tomove++)
        {
            *start = *txtpos;
            txtpos++;
            start++;
            linelen--;
        }
        program_end = newEnd;
    }
    go_LIST(linenum);
    nextStep = PROMPT;
    return;

unimplemented:
    printmsg(unimplimentedmsg);
    nextStep = PROMPT;
    return;

qhow:
    waitForRTN = true;
    printmsg(howmsg);
    nextStep = PROMPT;
    return;

qwhat:
    waitForRTN = true;

    printmsgNoNL((unsigned char*)QString("%1................").arg(errorNumber).rightJustified(24,QChar(' ')).toLatin1().data());
    if(current_line != NULL)
    {
           unsigned char tmp = *txtpos;
           if(*txtpos != NL)
                *txtpos = '^';
           list_line = current_line;
           printline();
           *txtpos = tmp;
    }
    line_terminator();
    nextStep = PROMPT;
    return;
    goto prompt;

qsorry:
    waitForRTN = true;
    printmsg(sorrymsg);
    nextStep = WARMSTART;
    return;
    goto warmstart;

run_next_statement:
    while ( (*txtpos == ':') || (*txtpos == ',') ) txtpos++;
    ignore_blanks();
    if(*txtpos == NL) {
        nextStep = EXECNEXTLINE;
        return;
        goto execnextline;
    }
    nextStep = INTERPERATEATTXTPOS;
    return;
    goto interperateAtTxtpos;

direct:
    qWarning("DIRECT");
    txtpos = program_end+sizeof(LINENUM);
    if(*txtpos == NL) {
        nextStep = PROMPT;
        return;
    }
    qWarning("continue");
interperateAtTxtpos:

        if(breakcheck())
        {
            // TODO: BREAK ON LINE MSG
            printmsgNoNL(breakmsg);
            printnum(*current_line,2);
            line_terminator();
            waitForRTN = true;
            nextStep = WARMSTART;
            running=false;
//          nextStep = WARMSTART;
          return;
          goto warmstart;
        }


    scantable(keywords,KEYWORD);
    ignore_blanks();

    expAlpha = false;
    switch(table_index)
    {
    case KW_DELAY:
    {
#if ARDUINO
        expression_error = 0;
        val = expression();
        delay( val );
        goto execnextline;
#else
        nextStep = UNIMPLEMENTED;
        return;
        goto unimplemented;
#endif
    }

    case KW_FILES:
        nextStep = FILES;
        return;
        goto files;
    case KW_LIST: go_LIST(); return;
    case KW_CHAIN:
        nextStep = CHAIN;
        return;
        goto chain;
    case KW_LOAD:
        nextStep = LOAD;
        return;
        goto load;
    case KW_MEM: go_MEM(); return;
    case KW_BEEP: go_BEEP(); return;
    case KW_NEW: go_NEW(); return;
    case KW_RUN: go_RUN(); return;
    case KW_SAVE: go_SAVE(); return;
    case KW_NEXT: go_NEXT(); return;
    case KW_LET: go_ASSIGNMENT(); return;
    case KW_IF: go_IF(); return;
    case KW_GOTO: go_GOTO(); return;
    case KW_GOSUB: go_GOSUB(); return;
    case KW_RETURN: go_RETURN(); return;
    case KW_REM:
    case KW_QUOTE: nextStep = EXECNEXTLINE; return;     // Ignore line completely
    case KW_FOR: go_FORLOOP(); return;
    case KW_INPUT: go_INPUT(); return;
    case KW_PAUSE: go_PAUSE(); return;
    case KW_PRINT:
    case KW_QMARK: go_PRINT(); return;
    case KW_POKE:
        nextStep = POKE;
        return;
        goto poke;
    case KW_END:
    case KW_STOP:
        // This is the easy way to end - set the current line to the end of program attempt to run it
        if(txtpos[0] != NL) {
            nextStep = QWHAT;
            return;
            goto qwhat;
        }
        current_line = program_end;
        nextStep = EXECLINE;
        return;
        goto execline;
    case KW_BYE:
        // Leave the basic interperater
        nextStep = BYE;
        return;

    case KW_PINMODE:  // PINMODE <pin>, INPUT/OUTPUT
        goto pinmode;
    case KW_DWRITE:  // DWRITE <pin>, HIGH|LOW
        isDigital = true;
        goto dwrite;

        //                case KW_RSEED:
        //                        goto rseed;

//    case KW_TONEW:
//        alsoWait = true;
//    case KW_TONE:
//        goto tonegen;
//    case KW_NOTONE:
//        goto tonestop;

    case KW_RADIAN : angleMode = RADIAN; nextStep = RUN_NEXT_STATEMENT; return; break;
    case KW_DEGREE : angleMode = DEGREE; nextStep = RUN_NEXT_STATEMENT; return; break;
    case KW_GRAD   : angleMode = GRAD  ; nextStep = RUN_NEXT_STATEMENT; return; break;
    case KW_DEFAULT:
        qWarning()<<"Default Assignment :"<<*txtpos;
        nextStep = ASSIGNMENT;
        return;
    default:
        nextStep = ASSIGNMENT;
        return;
        break;
    }


execnextline:
    if(current_line == NULL) {		// Processing direct commands?
        nextStep = PROMPT;
        return;
        goto prompt;
    }
    current_line +=	 current_line[sizeof(LINENUM)];

execline:
    if(current_line == program_end) {// Out of lines to run
        running = false;
        goto warmstart;
    }
    txtpos = current_line+sizeof(LINENUM)+sizeof(char);
    // discard label
    if (*txtpos=='"') {
        qWarning()<<"Skip Label";
        txtpos++;
        while (*txtpos!='"') txtpos++;
        txtpos++;
    }
    while( (*txtpos == ':')  ) txtpos++;

    nextStep = INTERPERATEATTXTPOS;
    return;



poke:
    {
#if 0
        quint16 value;
        quint16 *address;

        // Work out where to put it
        expression_error = 0;
        value = expression();
        if(expression_error)
            goto qwhat;
        address = value;

        // check for a comma
        ignore_blanks();
        if (*txtpos != ',')
            goto qwhat;
        txtpos++;
        ignore_blanks();

        // Now get the value to assign
        expression_error = 0;
        value = expression();
        if(expression_error)
            goto qwhat;
        //printf("Poke %p value %i\n",address, (unsigned char)value);
        // Check that we are at the end of the statement
        if(*txtpos != NL && *txtpos != ':')
            goto qwhat;
#endif
    }
    nextStep = RUN_NEXT_STATEMENT;
    return;
    goto run_next_statement;




/*************************************************/

#if ARDUINO
pinmode: // PINMODE <pin>, I/O
        {
        short int pinNo;

        // Get the pin number
        expression_error = 0;
        pinNo = expression();
        if(expression_error)
            goto qwhat;

        // check for a comma
        ignore_blanks();
        if (*txtpos != ',')
            goto qwhat;
        txtpos++;
        ignore_blanks();

                // look for the keyword for Input/Output
        scantable(inputoutput_tab);
        if(table_index == INPUTOUTPUT_UNKNOWN)
            goto qwhat;

                if( table_index <= INPUTOUTPUT_IN )
                {
                  pinMode( pinNo, INPUT );
                } else {
                  pinMode( pinNo, OUTPUT );
                }
        }
    nextStep = RUN_NEXT_STATEMENT;
    return;
        goto run_next_statement;

awrite: // AWRITE <pin>,val
dwrite:
        {
        short int pinNo;
                short int value;
                unsigned char *txtposBak;

        // Get the pin number
        expression_error = 0;
        pinNo = expression();
        if(expression_error)
            goto qwhat;

        // check for a comma
        ignore_blanks();
        if (*txtpos != ',')
            goto qwhat;
        txtpos++;
        ignore_blanks();


                txtposBak = txtpos;
                scantable(highlow_tab);
        if(table_index != HIGHLOW_UNKNOWN)
                {
                  if( table_index <= HIGHLOW_HIGH ) {
                    value = 1;
                  } else {
                    value = 0;
                  }
                } else {

                  // and the value (numerical)
                  expression_error = 0;
          value = expression();
          if(expression_error)
            goto qwhat;
                }

                if( isDigital ) {
                  digitalWrite( pinNo, value );
                } else {
                  analogWrite( pinNo, value );
                }
        }
    nextStep = RUN_NEXT_STATEMENT;
    return;
        goto run_next_statement;
#else
pinmode: // PINMODE <pin>, I/O
dwrite:
        goto unimplemented;
#endif

/*************************************************/

files:
        // display a listing of files on the device.
        // version 1: no support for subdirectories

#if ENABLE_FILEIO
    cmd_Files();
    goto warmstart;
#else
    goto unimplemented;
#endif


chain:
        runAfterLoad = true;

load:
        // clear the program
        program_end = program_start;

        // load from a file into memory
#if ENABLE_FILEIO
        {
          unsigned char *filename;

          // Work out the filename
          expression_error = 0;
          filename = filenameWord();
          if(expression_error)
            goto qwhat;

#if ARDUINO
      // Arduino specific
          if( !SD.exists( (char *)filename ))
          {
            printmsg( sdfilemsg );
          } else {

            fp = SD.open( (const char *)filename );
            inFromFile = true;
            inhibitOutput = true;
#else
      // Desktop specific
#endif
            // this will kickstart a series of events to read in from the file.
          }

        }
        goto warmstart;
#else
        goto unimplemented;
#endif


}

// returns 1 if the character is valid in a filename
int CTinyBasic::isValidFnChar( char c )
{
  if( c >= '0' && c <= '9' ) return 1; // number
  if( c >= 'A' && c <= 'Z' ) return 1; // LETTER
  if( c >= 'a' && c <= 'z' ) return 1; // letter (for completeness)
  if( c == '_' ) return 1;
  if( c == '+' ) return 1;
  if( c == '.' ) return 1;
  if( c == '~' ) return 1;  // Window~1.txt

  return 0;
}

unsigned char * CTinyBasic::filenameWord(void)
{
  // SDL - I wasn't sure if this functionality existed above, so I figured i'd put it here
  unsigned char * ret = txtpos;
  expression_error = 0;

  // make sure there are no quotes or spaces, search for valid characters
  //while(*txtpos == SPACE || *txtpos == TAB || *txtpos == SQUOTE || *txtpos == DQUOTE ) txtpos++;
  while( !isValidFnChar( *txtpos )) txtpos++;
  ret = txtpos;

  if( *ret == '\0' ) {
    expression_error = 1;
    return ret;
  }

  // now, find the next nonfnchar
  txtpos++;
  while( isValidFnChar( *txtpos )) txtpos++;
  if( txtpos != ret ) *txtpos = '\0';

  // set the error code if we've got no string
  if( *ret == '\0' ) {
    expression_error = 1;
  }

  return ret;
}

/***************************************************************************/
void CTinyBasic::line_terminator(DEVICE output)
{
    outchar(NL,output);
//    outchar(CR);
}

/***********************************************************/
void CTinyBasic::setup()
{
#if ARDUINO
  Serial.begin(kConsoleBaud);	// opens serial port
  while( !Serial ); // for Leonardo
#if ENABLE_FILEIO
  initSD();


#if ENABLE_AUTORUN
  if( SD.exists( kAutorunFilename )) {
    program_end = program_start;
    fp = SD.open( kAutorunFilename );
    inFromFile = true;
    inhibitOutput = true;
    runAfterLoad = true;
  }
#endif /* ENABLE_AUTORUN */

#endif /* ENABLE_FILEIO */

#if ENABLE_SECOND_SERIAL
  ssSerial.begin(9600);
#endif /* ENABLE_SECOND_SERIAL */
#endif /* ARDUINO */


}


/***********************************************************/
unsigned char CTinyBasic::breakcheck(void)
{
#if ARDUINO
  if(Serial.available())
    return Serial.read() == CTRLC;
#if ENABLE_SECOND_SERIAL
  if(ssSerial.available())
    return ssSerial.read() == CTRLC;
#endif
  return 0;
#else
#ifdef __CONIO__
  if(kbhit())
    return getch() == CTRLC;
   else
#endif
      if (breakFlag) {
          breakFlag = false;
          return true;
      }
     return false;
#endif
}
/***********************************************************/
int CTinyBasic::inchar()
{

    if (!commandBuffer.isEmpty()) {
        int got = commandBuffer.at(0);

        if( got == LF ) got = CR;
        if (got != CR) backupCommandBuffer.append(got);
        commandBuffer.remove(0,1);
        qWarning()<<"READ:"<<got;
        return got;
    }

    return 0;
}

/***********************************************************/
void CTinyBasic::outchar(unsigned char c,DEVICE output)
{
  if( inhibitOutput ) return;

  if (output == DISPLAY) {
      outputChar(QByteArray(1,c).toUpper());
      if (c=='\n') {
          //        waitForRTN = true;
      }
      putchar(c);
  }
  if (output==PRINTER) {
//      qWarning()<<"PRINTER"<<c;
      pPC1211->print(c);
  }

}

/***********************************************************/
/* SD Card helpers */

#if ARDUINO && ENABLE_FILEIO

static int initSD( void )
{
  // if the card is already initialized, we just go with it.
  // there is no support (yet?) for hot-swap of SD Cards. if you need to
  // swap, pop the card, reset the arduino.)

  if( sd_is_initialized == true ) return kSD_OK;

  // due to the way the SD Library works, pin 10 always needs to be
  // an output, even when your shield uses another line for CS
  pinMode(10, OUTPUT); // change this to 53 on a mega

  if( !SD.begin( kSD_CS )) {
    // failed
    printmsg( sderrormsg );
    return kSD_Fail;
  }
  // success - quietly return 0
  sd_is_initialized = true;

  // and our file redirection flags
  outToFile = false;
  inFromFile = false;
  inhibitOutput = false;

  return kSD_OK;
}
#endif

#if ARDUINO

void cmd_Files( void )
{
    File dir = SD.open( "/" );
    dir.seek(0);

    while( true ) {
    File entry = dir.openNextFile();
    if( !entry ) {
      entry.close();
      break;
    }

    // common header
    printmsgNoNL( indentmsg );
    printmsgNoNL( (const unsigned char *)entry.name() );
    if( entry.isDirectory() ) {
        printmsgNoNL( slashmsg );
    }

    if( entry.isDirectory() ) {
      // directory ending
      for( int i=strlen( entry.name()) ; i<16 ; i++ ) {
        printmsgNoNL( spacemsg );
      }
      printmsgNoNL( dirextmsg );
    } else {
      // file ending
      for( int i=strlen( entry.name()) ; i<17 ; i++ ) {
        printmsgNoNL( spacemsg );
      }
      printUnum( entry.size() );
    }
    line_terminator();
    entry.close();
    }
    dir.close();
}
#endif
#endif

bool CTinyBasic::CheckRunnig() {
    if (running) return true;

    nextStep = QWHAT;
    errorNumber = 1;
    return false;
}

bool CTinyBasic::CheckMode(Mode mode) {
    if (runMode==mode) return true;

    nextStep = QWHAT;
    errorNumber = 1;
    return false;

}

//BUG extend printnum to integer
void CTinyBasic::go_MEM() {
    waitForRTN = true;
//    inputMode = false;
    qWarning()<<"Wait for RTN , inputMode false";

    printnum(variables_begin-program_end,2);
    printmsgNoNL((unsigned char*) "STEPS ");
    printnum((variables_begin-program_end)/8,2);
    printmsg((unsigned char*) "MEMORIES");
    nextStep = RUN_NEXT_STATEMENT;
}

void CTinyBasic::go_BEEP(bool initial) {
    static quint64 refstate = pPC->pTIMER->state;

    if (!CheckRunnig()) return;

    if (initial) {
        nbBeep = testnum(); // Retuns 0 if no line found.
        if(nbBeep==0) {
            errorNumber = 1;
            nextStep = QWHAT;
            return;
        }
        qWarning()<<"BEEP:"<<nbBeep;
        refstate = pPC->pTIMER->state;

        beepTP = pPC->pTIMER->initTP(4000);
    }

    if (pPC->pTIMER->msElapsed(refstate)<200) {
        pPC->fillSoundBuffer(pPC->pTIMER->GetTP(beepTP)?0xFF:0x00);
    }
    else
        pPC->fillSoundBuffer(0);

    if (pPC->pTIMER->msElapsed(refstate)>400) {
        nbBeep--;
        qWarning()<<"BEEP:"<<nbBeep;
        if (nbBeep==0) {
            nextStep = RUN_NEXT_STATEMENT;
            return;
        }

        refstate = pPC->pTIMER->state;
        nextStep = BEEP;
    }
        nextStep = BEEP;
}

void CTinyBasic::go_NEW() {
    if (!CheckMode(PRO)) return;

    if(txtpos[0] != NL) {
        nextStep = QWHAT;
        return;
    }
    program_end = program_start;
    nextStep = PROMPT;
}

void CTinyBasic::go_RUN() {
    if (!(CheckMode(RUN) || CheckMode(DEF)) ) return;

    running = true;
    current_line = program_start;
    //TODO: add prog line management. Like LIST
#if 0


    linenum = testnum();
    if (linenum!=0)
        current_line = findline(linenum);
#else
    if (*txtpos!=NL) {
        expression_error = 0;
        expAlpha = false;
        double e = expression();
        if(expression_error ) {
            nextStep = QHOW;
            return;
        }

        current_line = findline(e);
    }
#endif
    nextStep = EXECLINE;
}

void CTinyBasic::go_LIST(LINENUM lineNb) {

    if (!CheckMode(PRO)) return;

    // Store Linenum and pointer into a Map
    scanLines();


    if (lineNb==0) {
        linenum = testnum(); // Retuns 0 if no line found.

        // Should be EOL
        if(txtpos[0] != NL) {
            nextStep = QWHAT;
            return;
        }
    }
    else linenum = lineNb;

    // Find the line
    list_line = findline(linenum);
    linenum= ((LINENUM *)list_line)[0];
    //    while(list_line != program_end)
    if (list_line != program_end) {
        inLIST = true;
        printline(printMode);
    }


//    waitForRTN = true;
    qWarning()<< "go_LIST(): waitForRTN true";
    if (printMode == DISPLAY) {
        nextStep = WARMSTART;
    }
    if (printMode == PRINTER) {
        nextStep = LIST_NEXT;
    }
}

void CTinyBasic::go_LIST_NEXT() {

    scanLines();
    if (lineMap.isEmpty()) {
        nextStep = WARMSTART;
        return;
    }
    qWarning()<<"LIST_NEXT:"<<linenum;
    QMap<LINENUM,unsigned char*>::const_iterator  i = lineMap.lowerBound(linenum);
    if (i.key()==linenum) i++;
    if (i ==lineMap.end()){
        if (printMode==PRINTER) {
            nextStep = WARMSTART;
            return;
        }
        i--;
    }
    qWarning()<<"go_LIST_NEXT(): "<<i.key();
    linenum = i.key();
    list_line = i.value();
    inLIST = true;
    printline(printMode);

//    waitForRTN = true;
    if (printMode == DISPLAY) {
        nextStep = WARMSTART;
    }
    if (printMode == PRINTER) {
        nextStep = LIST_NEXT;
    }
}

void CTinyBasic::go_LIST_PREV() {

    scanLines();

    if (lineMap.isEmpty()) {
        nextStep = WARMSTART;
        return;
    }

    QMap<LINENUM,unsigned char*>::const_iterator  i = lineMap.lowerBound(linenum);

    if (i != lineMap.begin()) {
        i--;
    }
    qWarning()<<"go_LIST_PREV(): "<<i.key();
    list_line = i.value();
    linenum = i.key();
    inLIST = true;
    printline();

    waitForRTN = true;
    nextStep = WARMSTART;
}

void CTinyBasic::go_PAUSE() {
    if (!CheckRunnig()) return;

    qWarning()<<"PAUSE";
    waitState = pPC->pTIMER->state;
    if (printMode==DISPLAY) pauseFlag = true;
    go_PRINT();
}

void CTinyBasic::go_USING() {
    if (!CheckRunnig()) return;

    qWarning()<<"USING";

    // Extrac format string and parse it
    ignore_blanks();        // Shouldn't be necessary because all lines are tokenized


    if ( (*txtpos != '"') &&
         (*txtpos != NL) &&
         (*txtpos != ';') &&
         (*txtpos != ',') &&
         (*txtpos != ':')) {
        nextStep = QWHAT;
        return;
    }
    if(*txtpos != '"' ) {
        // Reset format
        qWarning()<<"Reset USING format";
        usingFormat.clear();
        return;
    }
    else {

        txtpos++;
        while ((*txtpos !='"') && (*txtpos!=NL)) {
            usingFormat.append(*txtpos);
            txtpos++;
        }
        if (*txtpos!=NL) txtpos++;
        qWarning()<<"USING FORMAT:"<<usingFormat;
    }

}
CTinyBasic::ExpTYP CTinyBasic::checkType(VAR_TYPE *var) {
    if (((unsigned char*)var)[7]==0xF5) return STRING;

    return NUMERIC;

}

void CTinyBasic::printVar(VAR_TYPE e,DEVICE output) {
    qWarning()<<"printVar"<<e;
    if (checkType(&e)==STRING) {
        qWarning()<<"print string:";
        QByteArray ba((const char*)&e, sizeof(e));
        leftPosition = true;
        printmsgNoNL((unsigned char*)ba.left(7).data(),output);
    }
    else {
        leftPosition = false;
        printnum(e,8,output);
    }

    if (printMode==DISPLAY) waitForRTN = true;
}

void CTinyBasic::go_PRINT() {

//    if (!CheckRunnig()) return;

    DEVICE output = printMode;

    outputBuffer.clear();
    leftPosition=false;

    // If we have an empty list then just put out a NL
    if( (*txtpos == ':') || (*txtpos == ',') )
    {
        line_terminator(output);
        txtpos++;
        nextStep = RUN_NEXT_STATEMENT;
        return;
    }
    if(*txtpos == NL)
    {
        nextStep = EXECNEXTLINE;
        return;
    }

    while(1)
    {
        ignore_blanks();

        if (*txtpos == 0x80+KW_USING) { // Test for USING Keyword
            txtpos++;
            go_USING();

        }
        else if(print_quoted_string(output))
        {
            leftPosition = true;
        }
        else if(*txtpos == '"' || *txtpos == '\'') {
            nextStep = QWHAT;
            return;
        }
        else
        {
            VAR_TYPE e;
            expression_error = 0;
            expAlpha=false;
            e = expression();
//            qWarning()<<"exp:"<<e;
            if(expression_error){
                errorNumber = 1;
                nextStep = QWHAT;
                return;
            }

            if (expAlpha && (e == 0)) {
                ((unsigned char *)&e)[7]=0xF5;
            }
            printVar(e,output);
//            if (checkType(&e)==STRING) {
//                QByteArray ba((const char*)&e, sizeof(e));
//                leftPosition = true;
//                printmsg((unsigned char*)ba.mid(1).data());
//            }
//            else
//                printnum(e);
        }



        // At this point we have three options, a comma or a new line
        if(*txtpos == ';')
            txtpos++;	// Skip the comma and move onto the next
        else if(txtpos[0] == ';' && (txtpos[1] == NL || txtpos[1] == ':'))
        {
            txtpos++; // This has to be the end of the print - no newline
            break;
        }
        else if(*txtpos == NL || *txtpos == ':')
        {
            if (!leftPosition) {
                outputBuffer = outputBuffer.rightJustified(24,' ');
            }
            line_terminator(output);	// The end of the print statement
            // Left or right justify ?

            break;
        }
        else{
            nextStep = QWHAT;
            return;
        }
    }
    if (printMode==DISPLAY) waitForRTN = true;
    qWarning()<<"Wait for RTN , inputMode false";
//    inputMode = false;
    nextStep = RUN_NEXT_STATEMENT;
}

void CTinyBasic::go_GOTO() {

    if (!CheckRunnig()) return;

    expression_error = 0;
    expAlpha = false;
    double e = expression();
    if(expression_error || *txtpos != NL) {
        nextStep = QHOW;
        return;
    }

    current_line = findline(e);
    nextStep = EXECLINE;

}

void CTinyBasic::go_SAVE() {
        // save from memory out to a file
#if ENABLE_FILEIO
        {
          unsigned char *filename;

          // Work out the filename
          expression_error = 0;
          filename = filenameWord();
          if(expression_error)
            goto qwhat;

#if ARDUINO
          // remove the old file if it exists
          if( SD.exists( (char *)filename )) {
            SD.remove( (char *)filename );
          }

          // open the file, switch over to file output
          fp = SD.open( (const char *)filename, FILE_WRITE );
          outToFile = true;

      // copied from "List"
      list_line = findline();
      while(list_line != program_end)
            printline();

          // go back to standard output, close the file
          outToFile = false;
          fp.close();
#else
    // desktop
#endif

          nextStep = WARMSTART;
          return;
       }
#else
    nextStep = UNIMPLEMENTED;

#endif
}

void CTinyBasic::go_NEXT() {

    if (!CheckRunnig()) return;

    // Fnd the variable name
    ignore_blanks();
    if(*txtpos < 'A' || *txtpos > 'Z') {
        nextStep = QHOW;
        return;
    }
    txtpos++;
    ignore_blanks();
    if(*txtpos != ':' && *txtpos != ',' && *txtpos != NL) {
        nextStep = QWHAT;
        return;
    }

    go_RETURN();
}

void CTinyBasic::go_RETURN() {

    if (!CheckRunnig()) return;

    // Now walk up the stack frames and find the frame we want, if present
    tempsp = sp;
    while(tempsp < program+sizeof(program)-1)
    {
        switch(tempsp[0])
        {
            case STACK_GOSUB_FLAG:
                if(table_index == KW_RETURN)
                {
                    struct stack_gosub_frame *f = (struct stack_gosub_frame *)tempsp;
                    current_line	= f->current_line;
                    txtpos			= f->txtpos;
                    sp += sizeof(struct stack_gosub_frame);
                    nextStep = RUN_NEXT_STATEMENT;
                    return;
                }
                // This is not the loop you are looking for... so Walk back up the stack
                tempsp += sizeof(struct stack_gosub_frame);
                break;
            case STACK_FOR_FLAG:
                // Flag, Var, Final, Step
                if(table_index == KW_NEXT)
                {
                    qWarning()<<"NEXT:";
                    struct stack_for_frame *f = (struct stack_for_frame *)tempsp;
                    // Is the the variable we are looking for?
                    if(txtpos[-1] == f->for_var)
                    {
                        VAR_TYPE *varaddr = ((VAR_TYPE *)variables_begin) + txtpos[-1] - 'A';
                        *varaddr = *varaddr + f->step;
                        // Use a different test depending on the sign of the step increment
                        if((f->step > 0 && *varaddr <= f->terminal) || (f->step < 0 && *varaddr >= f->terminal))
                        {
                            // We have to loop so don't pop the stack
                            txtpos = f->txtpos;
                            current_line = f->current_line;
                            nextStep = RUN_NEXT_STATEMENT;
                            return;
                        }
                        // We've run to the end of the loop. drop out of the loop, popping the stack
                        sp = tempsp + sizeof(struct stack_for_frame);
                        nextStep = RUN_NEXT_STATEMENT;
                        return;
                    }
                }
                // This is not the loop you are looking for... so Walk back up the stack
                tempsp += sizeof(struct stack_for_frame);
                break;
            default:
                //printf("Stack is stuffed!\n");
            nextStep = WARMSTART;
            return;
        }
    }
    // Didn't find the variable we've been looking for
    nextStep = QHOW;
}


//TODO: extend to Lables like GOTO
void CTinyBasic::go_GOSUB() {

    qWarning()<<"go_GOSUB";
    if (!CheckRunnig()) return;

    expression_error = 0;
    expAlpha = false;
    double e = expression();
    if(expression_error || *txtpos != NL) {
        nextStep = QHOW;
        return;
    }

    if (checkType(&e)==NUMERIC) {
        linenum = e;
    }
    else {
        findline(e);
        linenum = labelLineNum;
    }

    if(!expression_error && *txtpos == NL)
    {
        struct stack_gosub_frame *f;
        if(sp + sizeof(struct stack_gosub_frame) < stack_limit) {
            nextStep = QSORRY;
            return;
        }

        sp -= sizeof(struct stack_gosub_frame);
        f = (struct stack_gosub_frame *)sp;
        f->frame_type = STACK_GOSUB_FLAG;
        f->txtpos = txtpos;
        f->current_line = current_line;
        current_line = findline(linenum);
        nextStep=EXECLINE;
        return;
    }
    nextStep=QHOW;
}

void CTinyBasic::go_FORLOOP() {

    if (!CheckRunnig()) return;

    ignore_blanks();
    if(*txtpos < 'A' || *txtpos > 'Z') {qWarning("un");nextStep=QWHAT; return;}
    var_for = *txtpos;
    txtpos++;
    ignore_blanks();
    if(*txtpos != '=') {qWarning("deux");nextStep=QWHAT; return;}
    txtpos++;
    ignore_blanks();

    expression_error = 0;
    initial_for = expression();
    if(expression_error) {
        qWarning("trois");
        errorNumber = 1;
        nextStep=QWHAT;
        return;
    }

    scantable(keywords,FOR_TO);
    ignore_blanks();
    if(table_index != KW_TO){qWarning("quatres");nextStep=QWHAT; return;}

    terminal_for = expression();
    if(expression_error) {
        qWarning("cinq");
        errorNumber = 1;
        nextStep=QWHAT;
        return;
    }

    scantable(keywords,FOR_STEP);
    ignore_blanks();
    if(table_index == KW_STEP)
    {
        step_for = expression();
        if(expression_error) {
            qWarning("six");
            errorNumber = 1;
            nextStep=QWHAT;
            return;
        }
    }
    else
        step_for = 1;
    ignore_blanks();
    if(*txtpos != NL && *txtpos != ':' && *txtpos != ',') {
        qWarning("sept");
        nextStep=QWHAT;
        return;
    }


    if(!expression_error && *txtpos == NL)
    {
        struct stack_for_frame *f;
        if(sp + sizeof(struct stack_for_frame) < stack_limit){nextStep=QSORRY; return;}

        sp -= sizeof(struct stack_for_frame);
        f = (struct stack_for_frame *)sp;
        ((VAR_TYPE *)variables_begin)[var_for-'A'] = initial_for;
        f->frame_type = STACK_FOR_FLAG;
        f->for_var = var_for;
        f->terminal = terminal_for;
        f->step     = step_for;
        f->txtpos   = txtpos;
        f->current_line = current_line;
        nextStep = RUN_NEXT_STATEMENT;
        return;
    }

    nextStep=QHOW;

}

void CTinyBasic::go_IF() {
    if (!CheckRunnig()) return;

    VAR_TYPE val;
    expression_error = 0;
    val = expression();
    if(expression_error || *txtpos == NL) {
        nextStep = QHOW;
        return;
    }
    if(val != 0) {
        nextStep = INTERPERATEATTXTPOS;
        return;
    }
    nextStep = EXECNEXTLINE;
}

//TODO: INPUT data (numeric and string)
void CTinyBasic::go_INPUT() {

    static unsigned char* programcounter;

    if (!CheckRunnig()) return;

    unsigned char var = 'A';
    if (nextStep!=INPUT_CR) {
        qWarning()<<"INPUT";
        ignore_blanks();
        if(*txtpos < 'A' || *txtpos > 'Z'){
            nextStep = QWHAT; return;
        }
        var = *txtpos;
        txtpos++;
        ignore_blanks();
        if(*txtpos != NL && *txtpos != ':') {
            nextStep = QWHAT; return;
        }
        programcounter = txtpos;
        processingInput=true;
        inputMode = true;
        nextStep=PROMPT;
        return;
    }
    qWarning()<<"INPUT_CR";
    commandBuffer.clear();
    inputMode = false;
    processingInput = false;
    double e= expression();
    ((VAR_TYPE *)variables_begin)[var-'A'] = e;
    txtpos = programcounter;
    nextStep = RUN_NEXT_STATEMENT;

    /*
    //////////////////////////////////////////////////////

    unsigned char isneg=0;
    unsigned char *temptxtpos;
    short int *var;
    ignore_blanks();
    if(*txtpos < 'A' || *txtpos > 'Z')
        goto syntaxerror;
    var = ((short int *)variables_table)+*txtpos-'A';
    txtpos++;
    if(!check_statement_end())
        goto syntaxerror;
again:
    temptxtpos = txtpos;
    if(!getln('?'))
        goto warmstart;

    // Go to where the buffer is read
    txtpos = program_end+sizeof(LINENUM);
    if(*txtpos == '-')
    {
        isneg = 1;
        txtpos++;
    }

    *var = 0;
    do 	{
        *var = *var*10 + *txtpos - '0';
        txtpos++;
    } while(*txtpos >= '0' && *txtpos <= '9');
    ignore_blanks();
    if(*txtpos != NL)
    {
        printmsg(badinputmsg);
        goto again;
    }

    if(isneg)
        *var = -*var;

    nextStep  = RUN_NEXT_STATEMENT;
    goto run_next_statement;

    */
}

void CTinyBasic::go_ASSIGNMENT() {
    if (!(CheckMode(RUN)||CheckMode(DEF))) return;

    qWarning("ASSIGNMENT");
    bool alpha = false;
    VAR_TYPE value;
    VAR_TYPE *var;

    unsigned char* savepos = txtpos;
    if(*txtpos < 'A' || *txtpos > 'Z') {
        go_PRINT();
        return;
    }

    var = (VAR_TYPE *)variables_begin + *txtpos - 'A';
    txtpos++;

    if (*txtpos=='$') {
        alpha = true;
        txtpos++;
    }
    ignore_blanks();

    if (*txtpos != '=') {
        txtpos=savepos;
        go_PRINT();
        return;
    }
    txtpos++;
    ignore_blanks();
    expression_error = 0;
    value = expression();//alpha?STRING:NUMERIC);
    qWarning()<<"expression_error:"<<expression_error;
    if(expression_error) {
        errorNumber = 1;
        nextStep=QWHAT;
        return;
    }
    // Check that we are at the end of the statement
    if(*txtpos != NL && *txtpos != ':'&& *txtpos != ',') {
        nextStep=QWHAT;
        return;
    }

    *var = value;
    // Print the assigned value
    if (!running ) { //&& ( *txtpos != ',')) {
        printVar(value);
    }

    if (!leftPosition) {
        outputBuffer = outputBuffer.rightJustified(24,' ');
    }
    line_terminator();	// The end of the print statement
    nextStep = RUN_NEXT_STATEMENT;

}

void CTinyBasic::switchMode() {
    switch(runMode) {
            case RUN: runMode = PRO;break;
            case PRO: runMode = RESERVE;break;
            case RESERVE: runMode = DEF;break;
            case DEF: runMode = RUN;break;
            }
}


