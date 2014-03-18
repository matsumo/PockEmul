#ifndef TINYBASIC_H
#define TINYBASIC_H

#include <QObject>
#include <QByteArray>
#include <QMap>




#define true 1
#define false 0
//#endif
#include "cpu.h"
// size of our program ram
#define kRamSize   1800

#define INPUTOUTPUT_IN  2
#define INPUTOUTPUT_UNKNOWN 6

#define STACK_SIZE (sizeof(struct stack_for_frame)*5)
#define VAR_TYPE double    //short int
#define VAR_SIZE sizeof(VAR_TYPE) // Size of variables in bytes

#define PI 3.141592653

class Cpc1211;

class CKeyword {
public:
    CKeyword(QString name = QString(),quint8 code = 0);

    QString Name;
    quint8  Code;
};

typedef short unsigned LINENUM;

struct stack_for_frame {
    char frame_type;
    char for_var;
    VAR_TYPE terminal;
    VAR_TYPE step;
    unsigned char *current_line;
    unsigned char *txtpos;
};

struct stack_gosub_frame {
    char frame_type;
    unsigned char *current_line;
    unsigned char *txtpos;
};

class CTinyBasic : public CCPU
{
    Q_OBJECT
public:

    enum DEVICE{DISPLAY,PRINTER};
    enum ExpTYP{NUMERIC,STRING,UNDEFINED};
    enum Mode{RUN,PRO,RESERVE,DEF};
    enum Action{INITIAL,RUNNING,UP_ARROW,DOWN_ARROW,ENTER,BREAK,NO_ACTION};
    enum NEXT_STEP {
        WARMSTART,PROMPT,DIRECT,QHOW,EXECLINE,
        EXECNEXTLINE,INTERPERATEATTXTPOS,
        FILES,LIST,CHAIN,LOAD,MEM,QWHAT,QSORRY,SAVE,NEXT,ASSIGNMENT,
        UNIMPLEMENTED,GOSUB,GOSUB_RETURN,FORLOOP,INPUT,INPUT_CR,PRINT,POKE,BYE,GETLN,GETLN_END,
        RUN_NEXT_STATEMENT,LIST_NEXT,LIST_PREV,
        BEEP

    };
    enum KEYWORD_TYPE {
        ALL,KEYWORD,OPE,FUNC,FOR_TO,FOR_STEP
    };

    NEXT_STEP nextStep;
    Mode        runMode;

       CTinyBasic(CPObject *parent = 0);

       Cpc1211 *pPC1211;

       virtual	bool	init(void);						//initialize
       virtual	bool	exit(void);						//end
       virtual void	step(void);						//step SC61860

       virtual	void	Load_Internal(QXmlStreamReader *xmlIn);
       virtual	void	save_internal(QXmlStreamWriter *xmlOut);
       virtual	bool	Get_Xin(void){return true;}
       virtual	void	Set_Xin(bool){}
       virtual	bool	Get_Xout(void){return true;}
       virtual	void	Set_Xout(bool){}

       virtual	UINT32	get_PC(void){return 0;}					//get Program Counter
       virtual void	Regs_Info(UINT8){}

       virtual const char*	GetClassName(){ return("CTINYBASIC");}

       virtual void	Reset(void);

       QByteArray commandBuffer;
       QByteArray backupCommandBuffer;
       QByteArray outputBuffer;
       QByteArray inputBuffer;



       void inputChar(quint8 c);
       void outputChar(QByteArray ba);

       int curDispLine;
       void clearOutput();
       QByteArray convertToDisplay(QByteArray ba);
       QByteArray go_PI();
       void outchar(unsigned char c, DEVICE output=DISPLAY);
       void ignore_blanks();
       void scantable(unsigned char *table,KEYWORD_TYPE type = KEYWORD);
       int isValidFnChar(char c);
       unsigned char *filenameWord();
       void line_terminator(DEVICE output=DISPLAY);
       void setup();
       unsigned char breakcheck();
       int inchar();
       unsigned short testnum();
       void pushb(unsigned char b);
       unsigned char popb();
       void printnum(VAR_TYPE num, int size=8, DEVICE output=DISPLAY);
       void printUnum(unsigned int num);
       void printmsgNoNL(const unsigned char *msg, DEVICE output=DISPLAY);
       void loop();
       VAR_TYPE expression(ExpTYP type=NUMERIC);
       unsigned char print_quoted_string(DEVICE output=DISPLAY);
       unsigned char *findline(double);
       void toUppercaseBuffer();
       void printline(DEVICE output=DISPLAY);
       VAR_TYPE expr5(ExpTYP type=NUMERIC);
       VAR_TYPE expr4(ExpTYP type=NUMERIC);
       VAR_TYPE expr3(ExpTYP type=NUMERIC);
       VAR_TYPE expr2(ExpTYP type=NUMERIC);
       void printmsg(const unsigned char *msg);
       void getln(char prompt);

       unsigned char *start;
       unsigned char *newEnd;
       unsigned char linelen;
           bool isDigital;
           bool alsoWait;
           int val;

           unsigned char var_for;
           VAR_TYPE initial_for, step_for, terminal_for;

           bool breakFlag;
           bool waitForRTN;

           void go_MEM();
           void go_NEW();
           void go_RUN();
           void go_LIST(LINENUM lineNb = 0);
           void go_LIST_NEXT();
           void go_LIST_PREV();
           void go_PRINT();
           void go_USING();
           void go_GOTO();
           void go_SAVE();
           void go_RETURN();
           void go_NEXT();
           void go_GOSUB();
           void go_FORLOOP();
           void go_IF();
           void go_INPUT();
           void go_ASSIGNMENT();
           void go_BEEP(bool initial = true);
           void go_PAUSE();

           unsigned char program[kRamSize];
           unsigned char *txtpos,*list_line;
           unsigned char expression_error;
           unsigned char *tempsp;

           enum ANGLE_MODE { DEGREE,RADIAN,GRAD};
           ANGLE_MODE angleMode;
           double convertToRad(double angle);
           void convertLine();

           int cursorPos;
           void switchMode();
           bool inputMode;
           qint64 waitState;
           bool pauseFlag;

           QMap<LINENUM,unsigned char*> lineMap;
           QMap<QByteArray,unsigned char*> lineDefMap;
           void scanLines();
           bool inLIST;
           LINENUM linenum;

           QMap<unsigned short,QByteArray> keywordsMap;

           bool inhibitOutput;
         bool runAfterLoad;
         bool triggerRun;

         bool table_original;

         unsigned char *stack_limit;
         unsigned char *program_start;
         unsigned char *program_end;        // Save into status
         unsigned char *stack; // Software stack for things that should go on the CPU stack
         unsigned char *variables_begin;
         unsigned char *current_line;
         unsigned char *sp;
         void LoadTable(unsigned char *table);
         double convertFromRad(double angle);

         QByteArray usingFormat;
         int errorNumber;
         int nbBeep,beepTP;
         bool running;

         bool CheckRunnig();
         bool CheckMode(Mode mode);
         bool expAlpha;
         ExpTYP checkType(double *var);
         void printVar(double e, DEVICE output=DISPLAY);
         bool leftPosition;
         bool processingInput;
         LINENUM labelLineNum;
         DEVICE printMode;

};

#endif // TINYBASIC_H
