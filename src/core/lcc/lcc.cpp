#include <cctype>
#include <QString>



#define ENABLE_COMPILATION 1

#if ENABLE_COMPILATION

#include "lcc.h"


extern MainWindowPockemul *mainwindow;

//FIXME: if ( iiii[0] > 0 ) { return;}
//FIXME: remove unused variables

/*!
  \brief Little C Compiler transorm a C source code to SC61860 ASM code.
  \class Clcc

<B>Writing Code in littleC</B>

General code layout:
\code
#org <start address>

[#nosave]	// No register saves (Only if you know what you're doing! In certain cases this will crash your Pocket Computer!)

[#include ..]	// Includes

[#define .. ..]	// Defines

[byte | char | word <varlist>]	// Var declarations

function1()	// Function declarations
{
    ...
}

...

main()
{
    ...
}
\endcode

<B>Differences from big (ANSI) C:</B>
- Do not use the word void! It will produce errors.
- Do not create names beginning with variable type names like charout() bytefetch() etc!
- Pointer can't be used for parameters, sorry!
- for loops will execute the third argument before the inner part of the loop!
- You can assign variables to register and memory addresses using "at".
- Names for local variables have to be global unique!
- Pointers can point to RAM card or CPU mem, use xram to point to xram. The compiler doesn't know to what kind
- You can't change the address of a variable using a pointer!
- No signed types.
- Operator priorities: [( )] >> [! && ||] >> [== != > < >= <=] >> [* / % &] >> [+ - << >> ~ |]
- Type conversion: A word assigned to a byte will store the LB of the word, a byte assigned to a word will get the byte into the LB of the word.
- Maximum array size is 256 for byte and 128 for word arrays.
- Local variables and function parameters can't be arrays.
- Function parameter declaration must be as follows: [type] function(type name, type name, ...) {...}
- You can't use comparisons in assignments.
- Every block must be enclosed with {..} but you need not if there is only one statement inside. (Example: "if (a>0) b++;")
- The for (;;) can hold three or two of its parts. Three statements: "for(a=1;a<100;a++)", two: "for(a=1;a<100;)". You must place two ';' inside the brackets!
- NO ADDITIONAL LIBRARIES, you can't use fancy stuff like printf, getc, strcpy, strcmp, ... and even the math ops * / % won't work because I have no code written for this! Feel free to help me out...
- Variable & function names are case sensitive but the assembler is not, so you can't name different vars or functions like "Afunction" and "afunction". Each name will have a label so don't create labels with the same name!
- Some libs are using own labels and the compiler will generate labels LB1, LB2, etc. Make sure you don't get a conflict!

Notice: Go through the delivered demo c code files to get a feeling for littleC, compile them and look at the assembler code produced!
  If something's not working correctly, please send me your files and tell me what result you've expected (contact info at the bottom of this file).

<B>Preprocessor commands:</B>
- #define (as you know it, but without parameters)
- #include (without the ".." or <..>)
- #ifdef .. #endif (without #else)
- #org (to define the start address of your program in your pocket computer)
- #asm .. #endasm (inline assembler)

<B>Supported methods and statements are:</B>
- Variable declarations (the known types are byte, char, word)
- Local variable restrictions: no arrays, no "at" and no init values!
- Preload variables (Examples: byte a = 10; char s[8]="String!"; byte array[4]=(1,2,3,4);)
- Assign register address: byte pbuf at 8; word y at 6 = 60400;
- Assign variables in ram card memory: word xram baspnt at 0xFFD7;
- Pointer declaration: xram will make a pointer point to a xram address (the pointer will be also in xram), no xram will have the pointer to point to CPU memory and the pointer itself is also there. You can't init a pointer with a value!
- Declare functions with "[type] name([parameters]) {}", [type] is "word", "char", "byte" or nothing.
- Call functions with "name();"
- if (condition) {} [else {}]
- while (condition) {}
- do {} while(condition);
- for (start; condition [; assignments]) {}
- loop (counter value from expression or constant) {} // will loop value + 1 times
  Never jump out of loop without "pop"
- break; // leaves the innermost loop
- return[ expression or constant]; // exits the current function, in the main program it end the whole program
- switch (expression) {value: procedure(); [value: procedure(); ...] [else: procedure();] }
- goto labelname; // jumps to label
- label labelname; // places a label
- Assignments are like this: "var = expression;" or "var += expression" (allowed are += -= *= /= <<= >>= %= |= &=)
- Expressions are formulas or constants, can contain variables and functions
- Operators are + - * / & | % ! ~ << >>
- conditions are similar and have these operators: == != < > >= <=
Please notice that you always must define a main function "main()"!



<B>Adding Libraries:</B>
You can write any code and store it as a file in the directory to include it in your programs.
However, some internal functions for 8bit and 16bit math need libraries which are included dynamically when the desired operation occurs.
These file names are:

8bit math:
- Multiplication: mul8.lib
- Division and Modulo: divmod8.lib
- XOR: xor8.lib
- Shift left: sl8.lib
- Shift right: sr8.lib

16bit math:
o Modulo: mod16.lib
- Multiplication: mul16.lib
o Division: div16.lib
- Shift left: sl16.lib
- Shift right: sr16.lib
- XOR: xor16.lib
- Compare equal: cpe16.lib
- Compare not equal: cpne16.lib
- Compare greater or equal: cpge16.lib
- Compare smaller or equal: cpse16.lib
- Compare greater: cpg16.lib
- Compare smaller: cps16.lib

Legend:
o = not coded yet
- = available

Library call 8bit:
Register A holds the first operand and B the second, as: A * B, the result must come back in A.
Then the routine is called. The name of the label must be like "LIB_MUL8:", replace MUL8 with the lib file names.

Library call 16bit:
Register A:B holds HB:LB of the second operand and J:I the first, the result must come back in B:A.
Then the routine is called. The name of the label must be like "LIB_MUL16:", replace MUL16 with the lib file names.
Comparisons must return the result in A as 255 for "true" and 0 for "false".



<B>Memory Management:</B>

littleC fills the CPU register space bottom->up with variables and top->down with stack.
Word variables are LB lower register address and HB higher address.

CPU memory:
\code
+-----+--------------+
|Addr.|	Name         |
+-----+--------------+
|0    |	Register I   |
|1    |	Register J   |
|2    |	Register A   |
|3    |	Register B   |
|4    |	Register XL  |
|5    |	Register XH  |
|6    |	Register YL  |
|7    |	Register YH  |
|8    |	Register K   |	<- littleC starts assigning register variables and arrays here
|9    |	Register L   |
|10   |	Register M   |
|11   |	Register N   |
|12   |	free	     |
|...  |	...	     |
|90   |	Stack memory |
|91   |	Stack top    |  <- The stack increases downwards
|92   |	Port A	     |
|93   |	Port B	     |
|94   |	Port F	     |
|95   |	Port C	     |
+-----+--------------+
\endcode
<B>Local variables and parameters:</B>
They will be pushed onto the stack, first the parameters and then the local variables.
Then the function is called. You can even do recursive calls! (But a stack overflow will crash your little PC :-)



<B>Warning:</B>
Please notice that littleC is not as powerful and safe as a big C compiler!
littleC is still at an early development state and it is not assured that the produced code will be fully operational!
Also it won't point out coding mistakes like assigning a value to an array without giving an array index.
Even if there is an error detected in a statement, lcc won't give you the line number because littleC makes a "line-less" compilation using several passes.
And surely NO guarantee for always correctly working software, sorry :-)



<B>Contact:</B>
Visit my homepage "www.pocket.2xs.de" or mail me directly at "simpc@gmx.de"!




  */


const QByteArray Alpha="_ABCDEFGHIJKLMNOPQRSTUVWXYZ"; /*!< TODO */

/*!
 \brief

 \fn Clcc::Clcc
 \param QMap<QString
 \param sources
 \param QMap<QString
 \param out
*/
Clcc::Clcc(QMap<QString,QByteArray> *sources,QMap<QString,QByteArray> *out,bool showErrors) {
    this->sources = sources;
    this->out = out;
    this->showErrors = showErrors;
    proccount = 0;
    VarCount = 0;
    VarPos = 0x20;        // Start adr assigning cpu variable
    level = 0;
    Look = 0;
    nosave = false;
    showErrors = true;
    sourceinASM = false;


    outfile = true;
    LCount = 0;

    asmcnt = 0;

    libcnt = 0;

    libname[MUL8] = "mul8";
    libname[DIVMOD8] = "divmod8";
    libname[SR8] = "sr8";
    libname[SL8] = "sl8";
    libname[XOR8] = "xor8";

    libname[MUL16] = "mul16";
    libname[DIVMOD16] = "divmod16";
    libname[AND16] = "and16";
    libname[OR16] = "or16";
    libname[XOR16] = "xor16";
    libname[SR16] = "sr16";
    libname[SL16] = "sl16";
    libname[NOT16] = "not16";
    libname[NEG16] = "neg16";

    libname[CPE16] = "cpe16";
    libname[CPNE16] = "cpne16";
    libname[CPS16] = "cps16";
    libname[CPG16] = "cpg16";
    libname[CPSE16] = "cpse16";
    libname[CPGE16] = "cpge16";

//    cg = new Clcg("outputasm");

    stdOp[OP_RTN]="RTN";
    stdOp[OP_INCLUDE]=".include";
}


/*!
 \brief

 \fn Clcc::writln
 \param fname
 \param s
*/
void Clcc::writln(QString fname,QString s) {
    if (fname !="LOG")
    {
        if (outfile) asmtext.append(s + "\r\n");
        else addasm(s.toLatin1());
    }
    else writeln(fname,s);

}



/*!
 \brief

 \fn Clcc::writeln
 \param fname
 \param s
*/
void Clcc::writeln(QString fname,QString s){
    write(fname,s+"\r\n");
}
/*!
 \brief

 \fn Clcc::write
 \param fname
 \param s
*/
void Clcc::write(QString fname,QString s){
    // emit signal ?
    // or fill an arraybuffer ?
//    if (fname !="LOG")
    {
        QByteArray actual = out->value(fname) +s.toLatin1();
        out->insert(fname, actual);
    }
//    emit outputSignal(fname,s);
}

//{--------------------------------------------------------------}
//{ Write error Message and Halt }

/*!
 \brief

 \fn Clcc::Error
 \param s
*/
void Clcc::Error(QString t) {
    QString s;
  if (!Tok.isEmpty()) s.append("\r\nToken: "+Tok);
  if (!dummy.isEmpty())s.append("\r\nCode: "+dummy);
  writeln("LOG","ERROR:"+t+"-"+s);
  if (showErrors) QMessageBox::about(mainwindow,"ERROR",t+"\n"+s);

}

//{--------------------------------------------------------------}
//{  }

/*!
 \brief Write "<something> Expected"

 \fn Clcc::Expected
 \param s
*/
void Clcc::Expected(QString s) {
    Error(s + " Expected");
}


const int MODEKEYB = 0; /*!< TODO */
const int MODEFILE = 1; /*!< TODO */
const int MODESTR = 2; /*!< TODO */


QList<char> SpacesList = QList<char> () << ' ' << 0x09 << 0x0A << 0x0B << 0x0C << 0x0D << 0x0E; /*!< TODO */
QList<char> OpsList = QList<char> () << '<' << '>' << '+' << '-' << '*' << '/' << '~' << '&' << '|' << '!' << '%' << '^'; /*!< TODO */
QList<char> Ops2List = QList<char> () << '<' << '>' << '+' << '-' << '*' << '/' << '~' << '&' << '|' << '!' << '%' << '^' << '='; /*!< TODO */
QList<char> HexList = QList<char> () << 'A' << 'B' << 'C' << 'D' << 'E' << 'F'; /*!< TODO */
QList<char> NumList = QList<char> () << '0' << '1' << '2' << '3' << '4' << '5' << '6' << '7' << '8' << '9'; /*!< TODO */


/*!
 \brief

 \fn Clcc::FindProc
 \param t
 \return bool
*/
bool Clcc::FindProc(QByteArray t) {
    bool result = false;
    for (int i = 0; i < proclist.size(); i++) {
        if (proclist[i].ProcName.toLower() == t.toLower())
        {
            result = true;
            ProcFound = i;
            return result;
        }
    }
    return result;
}

/*!
 \brief

 \fn Clcc::FindVar
 \param t
 \return bool
*/

bool Clcc::FindVarCurrProc(QByteArray t) {
    bool result = false;
    for (int i = 0 ; i< varlist.size(); i++) {
        if ( (varlist.at(i).varname.toLower()== t.toLower()) &&
             varlist.at(i).local &&
             varlist.at(i).locproc == currproc) {

            result = true;
            VarFound = i;
            return result;
        }
    }
    return result;
}

bool Clcc::FindVar(QByteArray t) {
    bool result = false;
    // Make a local search and if not found a global search.

    if (FindVarCurrProc(t)) return true;

    for (int i = 0 ; i< varlist.size(); i++) {
        if ( (varlist.at(i).varname.toLower()== t.toLower()) &&
              !varlist.at(i).local) {
            result = true;
            VarFound = i;
            return result;
        }
    }
    return result;
}

/*{--------------------------------------------------------------}
{  }
*/

/*!
 \brief Test if variable is at address

 \fn Clcc::IsVarAtAdr
 \param adr
 \param size
 \return bool
*/
bool Clcc::IsVarAtAdr(int adr,int size) {
    VarFound = -1;
    for (int i = 0; i< varlist.size(); i++) {
        if (varlist.at(i).address == adr) {
            VarFound=i;
            return true;
        }
    }
    return false;
}

/*!
 \brief

 \fn Clcc::printvarlist
 \param out
*/
void Clcc::printvarlist(QString out) {
    int  initn, adr, size, lproc;
    QByteArray s, name, typ;
    QByteArray inits;//QList<unsigned char> inits;
    bool xr, arr, loc;

    writeln(out,"");
    writeln(out,"VARIABLES DECLARED:");
    writeln(out,"");
    for (int i=0; i< varlist.size();i++) {
        Cvar v = varlist.at(i);
        name = v.varname;
        typ = v.typ;
        xr = v.xram;
        arr = v.array;
        adr = v.address;
        size = v.size;
        initn = v.initn;
        inits = v.inits;
        if (v.pointer) s="*"; else s="";
        loc = v.local;
        lproc = v.locproc;

        write(out,tr("VAR %1: ").arg(i+1));
        if (v.pointer) write(out,"*"+name+", PNTTYP:"+v.pnttyp+",TYP: "+typ+", ADR: "+QString("%1, XRAM: ").arg(adr));
        else write(out,name+", TYP: "+typ+", ADR: "+QString("%1, XRAM: ").arg(adr));
        if (xr) write(out,"yes, LOCAL: "); else write(out,"no, LOCAL: ");
        if (loc) s = proclist[lproc].ProcName;
        if (loc) write(out,"yes, FUNC: "+s+", SIZE: "); else write(out,"no, SIZE: ");
        write(out,QString("%1").arg(size));
        if (initn != -1) {
            write(out,", INIT=");
            if (arr) {
                if (typ == "char") {
//                    QString s = "";
//                    for (int i =0;i<inits.size();i++) s.append(inits[i]);
                    writeln(out,'"'+inits+'"');
                }
                else if (typ == "byte") {
                    for (int c = 0; c<size;c++) {
                        write(out,QString("%1").arg(inits[c]));
                        if (c < (size-1))
                            write(out,", ");
                        else
                            writeln(out,"");
                    }
                }
                else if (typ == "word")
                    for (int c = 0 ;c < size;c++) {
                    write(out,QString("%1").arg(256*inits[2*c]+inits[2*c+1]));

                    //write(QString("%1").arg(256*inits[c*2-1].toAscii()+inits[c*2].toAscii()));
                    if (c < (size-1))
                        write(out,", ");
                    else
                        writeln(out,"");
                }
            }
            else
                if (typ == "char")
                    writeln(out,QString((char)initn));
            else
                writeln(out,QString("%1").arg(initn));
        }
        else
            writeln(out,"");
    }
}
/*{--------------------------------------------------------------}
{  }
*/
/*!
 \brief Print Proc Table

 \fn Clcc::printproclist
 \param out
*/
void Clcc::printproclist(QString out) {

    writeln(out,"");
    writeln(out,"PROCEDURES DECLARED:");
    writeln(out,"");
    //QMapIterator<QString, Cproc> i(proclist);
    for (int i=0; i<proclist.size();i++) {
        write(out,"PROC "+proclist.at(i).ProcName+": ");
        write(out,proclist.at(i).ProcName);//,', CODE: ',proclist[i].proccode);
        if (proclist.at(i).hasreturn) {
            write(out,", RETURNS: ");
            if (proclist.at(i).ReturnIsWord) write(out,"word");
            else write(out,"byte");
        }
        if (proclist.at(i).ParCnt > 0) writeln(out,", PARAMS: "+proclist.at(i).Params);
        else writeln(out,"");
    }
    writeln(out,"");
}

/*!
 \brief

 \fn Clcc::vardecl
 \return QByteArray
*/
QByteArray Clcc::vardecl(void) {

    QByteArray t="";
    QByteArray Typ="";
    QByteArray result="";
    bool p,xr,l;
    QByteArray locname="";

    locname = ExtrWord(&Tok);
    Typ = locname;
    Tok.append(',');
    xr = false;
    do {
        t = ExtrList(&Tok);
        locname=ExtrWord(&t).trimmed();
        p = false;
        if (locname.startsWith("xram*")) {
            p = true;
            xr=true;
            locname.remove(0,5);//delete(name, 1, 1);
            locname=locname.trimmed();
        }
        if (locname.startsWith('*')) {
            p = true;
            locname.remove(0,1);//delete(name, 1, 1);
            locname=locname.trimmed();
        }
        if (locname.startsWith("xram")) {
            xr = true;
            locname=ExtrWord(&t);
        }
        if (t.size()>0) locname.append(" " + t);
        l = (level==0 ? false : true);

        AddVar(locname, Typ, xr, p, l,(l?currproc:-1));  // Global var definition
    }
    while (Tok.size() >0);

    return locname;
}

/*!
 \brief

 \fn Clcc::AddProc
 \param t
 \param c
 \param par
 \param pc
 \param hr
 \param wd
 \param partyp
 \param parname
*/
bool Clcc::AddProc(QByteArray t, QByteArray c, QByteArray par, int pc, bool hr, bool wd,QList<QByteArray> partyp,QList<QByteArray> parname) {

    QByteArray s = ExtrWord(&t);
    if (! FindProc(s)) {
        Cproc p;
        p.ProcName = s;
        p.ProcCode = c;
        p.Params = par;
        p.ParCnt = pc;
        p.hasreturn = hr;
        p.ReturnIsWord = wd;
        p.partyp = partyp;
        p.parname = parname;

        proclist.append(p);
        proccount++;
        //                writeln('Proc add: NAME: ' + s);
    }
    else {
        if (showErrors) QMessageBox::about(mainwindow,"ERROR","Procedure already declared: " + s);
        return false;
    }

    return true;
}

//{ Add Variable Declaration }

/*!
 \brief Add Variable Declaration

 \fn Clcc::AddVar
 \param t
 \param typ
 \param xr
 \param pnt
 \param loc
 \param proc
*/
bool Clcc::AddVar(QByteArray t,QByteArray typ, bool xr, bool pnt, bool loc,int proc) {

    QByteArray litem;
    QByteArray s = ExtrWord(&t);
    // Test if var still exist
    if (FindVarCurrProc(s))
    {
        if (showErrors) QMessageBox::about(mainwindow,"ERROR","Variable already declared: " + s);
        return false;
    }


    Cvar v;
    v.varname = s;
    v.pointer = pnt;
    v.xram = xr;
    v.local = loc;
    v.locproc = proc;
    v.typ = typ;
    if (pnt) {
        v.pnttyp = typ;
        v.typ = (xr ? "word" : "byte");
    }

    if ((typ == "byte") || (typ == "char")) v.size = 1;
    else if (typ == "word") v.size = 2;

    v.array = false;
    if (t.startsWith('[')) {

        QByteArray s = ExtrCust(&t, ']');
        if (s=="[") {
            v.size  = 0;
        }
        else {
            s.remove(0,1);
            int arraysize = mathparse(s, 16);
            if (arraysize >= 256) {
                if (showErrors) QMessageBox::about(mainwindow,"ERROR","Array too big! 256 bytes max.");
                return false;
            }
            v.size = arraysize;
        }
        v.array = true;
    }

    v.at = false;
    if (!loc) {
        if (t.startsWith("at")) {
            if (loc) {
                if (showErrors) QMessageBox::about(mainwindow,"ERROR","Local vars can't have 'at' assignments!");
                return false;
            }
            s = ExtrWord(&t);
            if (t.indexOf('=') >= 0) {
                s = ExtrCust(&t, '=');
                if (t.startsWith('(')) t = " " + t;
                t = "=" + t;
            }
            else s = t;
            //val(s, temp, c);
            int temp = mathparse(s, 16);
            if (temp > 95) xr = true;
            v.xram = xr;
            v.at = true;
            v.address = temp;
            if (pnt) {
                if (xr) {
                    v.typ = "word";
                    v.size = 2;
                }
                else {
                    v.typ = "byte";
                    v.size = 1;
                }
            }
            AllocVar(xr, true, v.size, temp);
            //s := ExtrWord(t);
        }
        else {
            if (typ == "word")
                v.address = AllocVar(xr, v.at, v.size * 2, -1);
            else
                v.address = AllocVar(xr, v.at, v.size, -1);
        }
        if (t.startsWith('=')) {
            if (loc) {
                if (showErrors) QMessageBox::about(mainwindow,"ERROR","Local vars cant have init values!");
                return false;
            }
            if (pnt) {
                if (showErrors) QMessageBox::about(mainwindow,"ERROR","pointers cant have init values!");
                return false;
            }
            t.remove(0,1);
            if (v.array && (typ == "char") ) {
                t.remove(0,1);
                t.remove(t.length()-1,1);
                for (int i=0;i<t.length();i++) v.inits.append(t[i]);
                if (v.size == 0) {
                    v.size = t.length()+1;
                    v.inits.append(QChar(0));
                }
                if (v.size != (t.length()+1)) {
                    if (showErrors) QMessageBox::about(mainwindow,"ERROR","Array size different from initial value size");
                    return false;
                }
                v.initn = 0;
            }
            else if (v.array && (typ == "byte") ) {
                t.remove(0,2);
                t.remove(t.length()-1,1);
                t = t + ',';
                while ((litem = ExtrList(&t)) != "") {
                    v.inits.append((char)mathparse(litem, 8));
                }
                v.initn = 0;
            }
            else if (v.array && (typ == "word")) {
                t.remove(0,2);
                t.remove(t.length()-1,1);//delete(t, length(t), 1);
                t = t + ',';
                while ((litem = ExtrList(&t)) != "") {
                    v.inits.append((char)mathparse(litem+"/256", 8));
                    v.inits.append((char)mathparse(litem+"%256", 8));
                    //val(litem, temp, c);
                    //VarList[VarCount].inits := VarList[VarCount].inits + chr(temp div 256) + chr(temp mod 256);
                }
                v.initn = 0;
            }
            else
                //val(t, VarList[VarCount].initn, temp);
                v.initn = mathparse(t, 16);
        }
        else
            v.initn = -1;

    }
    else {
        v.initn = -1;
        if (! procd) {
            proclist[currproc].LocName.append(v.varname);
            proclist[currproc].LocTyp.append(v.typ);
            proclist[currproc].LocCnt++;

        }
    }

    VarCount++;

    varlist.append(v);

    return true;
}




/*!
 \brief Split String in Words

 \fn Clcc::ExtrWord
 \param word
 \return QByteArray
*/
QByteArray Clcc::ExtrWord(QByteArray *word) {

    char c;

    QByteArray result = "";
    if (word->isEmpty()) return "";

    c = ' ';
    while ( (!word->isEmpty() && ! SpacesList.contains(word->at(0))) || (c != ' ')) {
        if (word->startsWith('"') || word->startsWith("'")) {
            if (word->startsWith(c)) c = ' ';
            else c = word->at(0);
        }
        result = result + word->at(0);
        word->remove(0,1);
    }
    if (!word->isEmpty())
        if (SpacesList.contains(word->at(0))) word->remove(0,1);
    result = result.trimmed();
    //        if word[1] in ['[', '('] then
    //                word := word + ExtrWord(word);

    return result;
}

//{ Split String in Words }

/*!
 \brief

 \fn Clcc::ExtrCust
 \param word
 \param c
 \return QByteArray
*/
QByteArray Clcc::ExtrCust(QByteArray *word,char c) {
    char sc;
    QByteArray result = "";
    if (word->isEmpty()) return "";

    sc = ' ';
    while ( (!word->isEmpty() && !word->startsWith(c)) || (sc != ' ') ) { //and not (word[1] in ['[', '(']) do
        if (word->startsWith('"') || word->startsWith("'")) {
            if (word->startsWith(sc)) sc = ' ';
            else sc = word->at(0);
        }
        result = result + word->at(0);
        word->remove(0,1);//delete(word, 1, 1);
    }
    if (!word->isEmpty()) {
        if (word->startsWith(c)) word->remove(0,1);//delete(word, 1, 1);
        if (!word->isEmpty())
            if (SpacesList.contains(word->at(0))) word->remove(0,1);// delete(word, 1, 1);
        result = result.trimmed();
    }
    result = result.trimmed();
    //        if word[1] in ['[', '('] then
    //                word := word + ExtrWord(word);
    return result;
}

//{  }

/*!
 \brief Split List

 \fn Clcc::ExtrList
 \param list
 \return QByteArray
*/
QByteArray Clcc::ExtrList(QByteArray *list) {

    QByteArray result = "";
    int l = 0;

    if (list->isEmpty()) return "";
    char c = ' ';
    while ( ((list->size()>0) && !(list->at(0) == ',')) || (l > 0) || (c != ' ') ) {
        if (list->startsWith('(')) l++;
        if (list->startsWith(')')) l--;
        if (list->startsWith('"') || list->startsWith("'")) {
            if (list->startsWith(c)) c = ' ';
            else c = list->at(0);
        }
        result.append(list->at(0));
        list->remove(0,1);//delete(list, 1, 1);
        if (list->isEmpty()) return "";
    }
    list->remove(0,1);//delete(list, 1, 1);
    result = result.trimmed();

    return result;
}

//{  }

/*!
 \brief Allocate Variable Declaration

 \fn Clcc::AllocVar
 \param xr
 \param at
 \param size
 \param adr
 \return int
*/
int Clcc::AllocVar(bool xram,bool at,int  size, int adr) {
    QByteArray s;
    int result;
    if (!xram) {
        if (at) {
            result = adr;
            if (IsVarAtAdr(result, size)) {
                if (varlist[VarFound].at) {
                    s= QByteArray::number(varlist[VarFound].address);
                    if (showErrors) QMessageBox::about(mainwindow,"ERROR","Previous var "+varlist[VarFound].varname+" at "+s+" already declared!");
                }
                varlist[VarFound].address = VarPos;
                VarPos += varlist[VarFound].size;
                if (size == 2) {
                    if (IsVarAtAdr(result+1, 1)) {
                        if (varlist[VarFound].at) {
                            s= QByteArray::number(varlist[VarFound].address);
                            if (showErrors) QMessageBox::about(mainwindow,"ERROR","Overlap with "+varlist[VarFound].varname+" at "+s+"!");
                        }
                        varlist[VarFound].address = VarPos;
                        VarPos += varlist[VarFound].size;
                    }
                }
                result = adr;
            }
        }
        else {
            while (IsVarAtAdr(VarPos, size)) {
                VarPos += varlist[VarFound].size;
            }
            result = VarPos;
            VarPos += size;
        }
        //for i:=AllocVar to AllocVar+size-1 do memimg[i] := VarCount;
    }
    else result = -1;

    return result;
}

/*!
 \brief

 \fn Clcc::Read
 \return char
*/
char Clcc::Read(void) {
    return 0;
}

/*!
 \brief

 \fn Clcc::rd
 \param c
 \param s
*/
void Clcc::rd(char *c, QByteArray *s) {
    if (md == 0) *c=Read();
    else
    if (md == 1) {
        *c = Read();
        //if (eof(f))  *c = chr(27);
    }
    else if (md == 2) {
        if (!s->isEmpty()) *c = s->at(0);
        else *c = 27;
        s->remove(0,1);
    }

    if (currentQuote == ' ')
    {
        if (*c == '}') level--;
        if (*c == '{') level++;
    }
    if (*c == '\'')
    {
        if (currentQuote == ' ') currentQuote = '\'';
        else if (currentQuote == '\'') currentQuote = ' ';
    }
    if (*c == '"')
    {
        if (currentQuote == ' ') currentQuote = '"';
        else if (currentQuote == '"') currentQuote = ' ';
    }

}

/*!
 \brief

 \fn Clcc::GetToken
 \param mode
 \param s
*/
void Clcc::GetToken(int mode, QByteArray *s) {

    writln("LOG",";GetToken:"+*s);
    Tok="";
    currentQuote = ' ';
    md = mode;

    do {
        rd(&Look, s);
        if (Look == 27) return;
    }
    while (SpacesList.contains(Look) || QByteArray("{}").contains(Look));

    //for  ( ; (l != ' ') || !QString(";{}").contains(Look) ; )
    while((currentQuote != ' ') || !QByteArray(";{}").contains(Look))// || (Look!=27))
    {
        if (currentQuote == ' ')
        {
            if ( !SpacesList.contains(Look) && !QByteArray("{}").contains(Look))
            {
                Tok = Tok + Look;
                rd(&Look,s);
                if (Look == 27) return;
            } else
            {
                Tok = Tok + ' ';
                while (SpacesList.contains(Look)) {
                    rd(&Look, s);
                    if (Look == 27) return;
                }
            }
        } else
        {
            Tok = Tok + Look;
            rd(&Look, s);
            if (Look == 27) return;
        }
    }

    //        if (Tok != "")  if (Tok[0] == ' ') Tok.remove(0,1);
    //        if (Tok.right(1) == ' ' ) delete(Tok, length(Tok), 1);
    Tok = Tok.trimmed();
    writln("LOG",";GetToken(middle):"+Tok);
    if (sourceinASM) writln(outf,"\t; Source: "+Tok);
    int i = 1;
    currentQuote = ' ';
    //for (int i = 1 ; i< (Tok.size()-1); )
    while (i<Tok.size()-1)
    {
        if (Tok[i] == '\'') {
            if (currentQuote == ' ')  currentQuote = '\'';
            else if (currentQuote == '\'') currentQuote = ' ';
        }
        if (Tok[i] == '"') {
            if (currentQuote == ' ') currentQuote = '"';
            else if (currentQuote == '"') currentQuote = ' ';

        }
        if (currentQuote == ' ')
        {
            if ((Tok[i-1] != ' ') && (QByteArray("[(=").contains(Tok[i])))
            {
                Tok.insert(i,' ');
                i++;
            }
            if (QByteArray("])").contains(Tok[i]) && (Tok[i+1] != ' '))
            {
                Tok.insert(i+1,' ');
                i++;
            }
            if (Ops2List.contains(Tok[i]) && (Tok[i+1] == ' '))
            {
                Tok.remove(i+1, 1);
                i--;
            }
            else if ((Tok[i-1] == ' ') && OpsList.contains(Tok[i]))
            {
                Tok.remove(i-1, 1);
                i--;
            }
        }
        i++;
    }
writln("LOG",";GetToken(end):"+Tok);
}


//{--------------------------------------------------------------}
//{  }

/*!
 \brief Get an Identifier

 \fn Clcc::GetName
 \return QByteArray
*/
QByteArray Clcc::GetName(void) {
    QByteArray n;

    n = "";
    if (!Alpha.contains(toupper(Look))) Expected("Name");
    while (Alpha.contains(toupper(Look)) || NumList.contains(toupper(Look))) {
        n.append(Look);
        rd(&Look, &Tok);
        Tok = Tok.trimmed();
    }
    writln("LOG",";GetName:"+n);
    return (n);
}

/*!
 \brief

 \fn Clcc::skiphex
 \param hs
 \param hi
 \return int
*/
int Clcc::skiphex(QByteArray hs,int hi) {
    while ( (hi < hs.length()) && QByteArray("0123456789ABCDEFabcdef").contains(hs[hi])) {
        hi++;
    }
    return hi;
}
/*!
 \brief

 \fn Clcc::skipbin
 \param hs
 \param hi
 \return int
*/
int Clcc::skipbin(QByteArray hs,int hi) {
    while ( (hi < hs.length()) && QByteArray("0123456789").contains(hs[hi])) {
        hi++;
    }
    return hi;
}

//{--------------------------------------------------------------}

/*!
 \brief

 \fn Clcc::find_text
 \param such
 \param text
 \return int
*/
int Clcc::find_text(QByteArray such, QByteArray text) {
    int i;
    char c;
    int result = 0;
    if (such.isEmpty() || text.isEmpty()) return 0;
    text = " "  + text + " ";
    i = 1;
    c = ' ';
    while (i < (text.length())) {
        if (QByteArray("'\"").contains(text[i])) {
            if (c == ' ') c = text[i];
            else c = ' ';
        }
        if ((c == ' ') &&
            text.mid(i).startsWith(such) &&
            !  ( QByteArray("_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ").contains(toupper(text[i-1])) ||
                 QByteArray("_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ").contains(text[i+such.length()]) ) ) {
            return(i);
            break;
        }
        i++;
    }
    return result;
}

//{--------------------------------------------------------------}
//{ Recognize an Addition Operator }
/*!
 \brief

 \fn Clcc::IsAddop
 \param c
 \return bool
*/
bool Clcc::IsAddop(char c) {
    return (QByteArray("+-|~$§").contains(c));
}

//{--------------------------------------------------------------}
/*!
 \brief

 \fn Clcc::Expression
*/
bool Clcc::ExpressionType(QByteArray e) {
    bool result = false;
    QList<QString> identifiedVars;
    // Look for local var
    for (int i= 0 ; i<varlist.size(); i++) {
        Cvar v = varlist.at(i);
        if (v.local && (v.locproc==currproc) && (find_text(v.varname, e) > 0) ){

            //isword = false;
            if (varlist[i].pointer && (varlist[i].pnttyp == "word")) isword = true;
            else
            if (varlist[i].typ == "word") isword = true;

            identifiedVars.append(v.varname);
            result = true;
        }
    }
    // Look non local
    for (int i= 0 ; i<varlist.size(); i++) {
        Cvar v = varlist.at(i);
        if (!v.local && !identifiedVars.contains(v.varname) && (find_text(v.varname, e) > 0) ){

            //isword = false;
            if (varlist[i].pointer && (varlist[i].pnttyp == "word")) isword = true;
            else
            if (varlist[i].typ == "word") isword = true;
            result = true;

        }
    }
    return result;
}

void Clcc::Expression(void) {
    int i;

    Tok = Look + Tok;
    writln("LOG",";Expression:"+Tok);

#if 1
    ExpressionType(Tok);
#else
    for (int i= 0 ; i<varlist.size(); i++) {
        if (find_text(varlist[i].varname, Tok) > 0) {

            //isword = false;
            if (varlist[i].pointer && (varlist[i].pnttyp == "word")) isword = true;
            else
            if (varlist[i].typ == "word") isword = true;
        }
    }
#endif
    i = 0;

    // replace << by $
    // replace >> by §
    // replace ++ by PP
    // replace -- by MM
    Tok.replace("<<","$").replace(">>","§");
    Tok.replace("++",QByteArray(1,PP));
    Tok.replace("--",QByteArray(1,MM));
    Tok.replace("\\n","\n").replace("\\r","\r");
//    else if copy(Tok,i,1) = '''' then
//         begin c := copy(tok, i+1,1)[1]; delete(tok,i,3); insert(inttostr(ord(c)),tok,i); dec(i); end
// A REVOIR !!!!!!!!!!!!!!!!!!
    while (i < (Tok.length()-1)) {
        if (Tok.at(i) == '\'') {
            char c = Tok.at(i+1);
            Tok.remove(i,3);
            Tok.insert(i,QByteArray::number(c));
            i--;
        }
        else i++;
    }
    rd(&Look, &Tok);

    SignedTerm();
    while (IsAddop(Look)) {
        switch (Look) {
        case '+': Add(); break;
        case '-': Subtract(); break;
        case '|': _Or(); break;
        case '~': _Xor(); break;
        case '§': ShiftR(); break;
        case '$': ShiftL(); break;
        }
    }
}

//{--------------------------------------------------------------}
//{  }

/*!
 \brief Recognize a Numeric Character

 \fn Clcc::IsDigit
 \param c
 \return bool
*/
bool Clcc::IsDigit(char c) {
    return ( (c >='0') && (c<='9') );
}

//{--------------------------------------------------------------}
//{ Get a Number }

/*!
 \brief

 \fn Clcc::GetNumber
 \return QByteArray
*/
QByteArray Clcc::GetNumber(void) {
QByteArray n;
int p;
bool isbin, ishex, ischr;

    n = "";
        p = 0;
        ishex = false;
        isbin = false;
        ischr = false;

    if (!IsDigit(Look) && (Look != '\'')) Expected("Integer");

    while (IsDigit(Look) ||
                ( (p=0) && (Look=='\'') ) ||
                ( (p=1) && ((toupper(Look)=='B') || (toupper(Look)=='X')) ) ||
                ( ishex && HexList.contains(toupper(Look)) ) ||
                ( isbin && NumList.contains(toupper(Look)) ) ||
                ischr) {
        if (p == 1) {
            if (toupper(Look)=='X') ishex = true;
            else if (toupper(Look)=='B') isbin = true;
        }
        if ((p == 0) && (Look == '\'')) ischr = true;
        if ((p == 2) && ischr) ischr = false;
        n.append(Look);
        p++;
        rd(&Look, &Tok);
        if (! ischr) Tok = Tok.trimmed();
    }
//       if ishex then GetNumber := inttostr(converthex(n))
//        else if isbin then GetNumber := inttostr(convertbin(n))
//        else if n[1]='''' then GetNumber := inttostr(ord(n[2]))
//        else
    return (n);
}



//{--------------------------------------------------------------}
//{  }

/*!
 \brief Parse and Translate a Factor

 \fn Clcc::Factor
*/
void Clcc::Factor(void) {
    //var s{, temp}: string;
    QByteArray s;

    writln("LOG",";Factor:"+Tok);
    if (sourceinASM) writln(outf,"\t; Factor : "+Tok);
    if (Look =='(') {
        rd(&Look, &Tok);
        Tok = Tok.trimmed();
        Expression();
        rd(&Look, &Tok);
        Tok = Tok.trimmed();
    }
    else if (IsDigit(Look)) {
        LoadConstant(GetNumber());
    }
    else if (Alpha.contains(toupper(Look))) {
        bool bracketFound = false;
        s = GetName();
        if (Look == '[') {
            writln("LOG",";Factor(found[):"+Tok);
            bracketFound=true;
            rd(&Look, &Tok);
           Tok = Tok.trimmed();
            Expression();
            rd(&Look, &Tok);
           Tok = Tok.trimmed();
           writln("LOG",";Factor(found[2):"+Tok);
            //Push;
        }
        if (FindVar(s)) {
            if (bracketFound && varlist.at(VarFound).pointer) pointer = ptrREFARR;
            LoadVariableMain(s);//LoadVariable(s);
        }
        else if (FindProc(s)) {
            Tok = s + " (" + Tok.trimmed();
            ProcCall();
            Tok = Tok.trimmed();
        }
    }
    else Error("Unrecognized character " + Look);

    writln("LOG",";Factor(end):"+Tok);
}
//{--------------------------------------------------------------}



//{--------------------------------------------------------------}
//{  }
/*!
 \brief Parse and Translate a Factor with Optional "Not"

 \fn Clcc::NotFactor
*/
void Clcc::NotFactor(void) {
    writln("LOG",";NotFactor:"+Tok);
    if (Look == '!') {
        rd(&Look, &Tok);
        Tok = Tok.trimmed();
        Factor();
        NotIt();
    }
    else if (Look == '*') {
        rd(&Look, &Tok);
        Tok = Tok.trimmed();
        pointer = ptrREF;
        Factor();
    }
    else if (Look == '&') {
        rd(&Look, &Tok);
        Tok = Tok.trimmed();
        pointer = ptrADR;
        Factor();
    }
    else {
        pointer = 0;
        Factor();
    }
}
//{--------------------------------------------------------------}


//{--------------------------------------------------------------}
//{  }
//FIXME: Mod a un problème. A checker
/*!
 \brief Parse and Translate a Term

 \fn Clcc::Term
*/
void Clcc::Term(void) {
    NotFactor();
    while (QByteArray("*/&%").contains(Look)) {
        switch (Look) {
        case '*': Multiply();break;
        case '/': Divide();break;
        case '&': _And();break;
        case '%': _Mod();break;
        }
    }
}

//{--------------------------------------------------------------}
//{  }
/*!
 \brief Parse and Translate a Factor with Optional Sign

 \fn Clcc::SignedTerm
*/
void Clcc::SignedTerm(void) {
    char Sign;
    Sign = Look;
    if (IsAddop(Look)) {
        rd(&Look, &Tok);
        Tok = Tok.trimmed();
    }
    Term();
    if (Sign == '-') Negate();
}

//{--------------------------------------------------------------}
//{  }

/*!
 \brief Parse and Translate an Addition Operation

 \fn Clcc::Add
*/
void Clcc::Add() {
    rd(&Look, &Tok);
    Tok = Tok.trimmed();
    Push();
    Term();
    PopAdd();
}

//{--------------------------------------------------------------}
//{  }

/*!
 \brief Parse and Translate a Subtraction Operation

 \fn Clcc::Subtract
*/
void Clcc::Subtract(void) {
    rd(&Look, &Tok);
    Tok = Tok.trimmed();
    Push();
    Term();
    PopSub();
}


//{--------------------------------------------------------------}
//{  }

/*!
 \brief Parse and Translate an Multiply Operation

 \fn Clcc::Multiply
*/
void Clcc::Multiply(void) {
    rd(&Look, &Tok);
    Tok = Tok.trimmed();
    Push();
    Term();
    PopMul();
}


//{--------------------------------------------------------------}
//{  }

/*!
 \brief Parse and Translate a Divide Operation

 \fn Clcc::Divide
*/
void Clcc::Divide(void) {
    rd(&Look, &Tok);
    Tok = Tok.trimmed();
    Push();
    Term();
    PopDiv();
}


//{ Parse and Translate a Subtraction Operation }

/*!
 \brief

 \fn Clcc::_Or
*/
void Clcc::_Or(void) {
    rd(&Look, &Tok);
    Tok = Tok.trimmed();
    Push();
    Term();
    PopOr();
}

//{  }

/*!
 \brief Parse and Translate a Shift Operation

 \fn Clcc::ShiftR
*/
void Clcc::ShiftR(void) {
    rd(&Look, &Tok);
    Tok = Tok.trimmed();
    Push();
    Term();
    PopSr();
}

//{  }

/*!
 \brief Parse and Translate a Shift Operation

 \fn Clcc::ShiftL
*/
void Clcc::ShiftL(void) {
    rd(&Look, &Tok);
    Tok = Tok.trimmed();
    Push();
    Term();
    PopSl();
}

//{  }

/*!
 \brief Parse and Translate a Subtraction Operation

 \fn Clcc::_Xor
*/
void Clcc::_Xor(void) {
    rd(&Look, &Tok);
    Tok = Tok.trimmed();
    Push();
    Term();
    PopXor();
}


//{  }

/*!
 \brief Parse and Translate a Boolean And Operation

 \fn Clcc::_And
*/
void Clcc::_And(void) {
    rd(&Look, &Tok);
    Tok = Tok.trimmed();
    Push();
    NotFactor();
    PopAnd();
}


//{  }

/*!
 \brief Parse and Translate a Modulo Operation

 \fn Clcc::_Mod
*/
void Clcc::_Mod(void) {
    rd(&Look, &Tok);
    Tok = Tok.trimmed();
    Push();
    Term();
    PopMod();
}

//{--------------------------------------------------------------}

//{-------------------------------------------------------------}
//{  }

/*!
 \brief Procedure Call

 \fn Clcc::ProcCall
*/
void Clcc::ProcCall() {
    int i, c, a;
    QByteArray temp;


    writln("LOG",";ProCall:"+Tok);
    if (Tok.isEmpty()) return;

    temp = ExtrWord(&Tok);
    writln("LOG",";ProCall(after extrWord):"+Tok);
    //        s := Tok;          // Nur den Parameterblock der ersten Funktion extrahieren!!!
    a = 0;
    rd(&Look, &Tok);
    Tok = Tok.trimmed();
    if (temp == "_LCC_DEPEND") {
        QByteArray name2 = ExtrWord(&Tok);
        name2.chop(1);
        if (FindProc(name2)) {
                calledProc.insertMulti(currproc,ProcFound);
        }
        else {
            if (showErrors) QMessageBox::about(0,"ERROR","_LCC_DEPEND("+name2+") - '"+ name2+"' undefined");
        }
    }
    else
    if (FindProc(temp)) {
        i = proclist[ProcFound].ParCnt;
        if (i > 0) {
            //delete(s, 1, 1); s := trim(s);
            c = 0;
            do {
                rd(&Look, &Tok);
                Tok = Tok.trimmed();
                if (proclist[ProcFound].partyp[c] != "word") {
                    isword = false;
                    a++;
                }
                else {
                    isword = true;
                    a+=2;
                }
                bool partypeisword = isword;
                // check if parameter type == Expression type;
                Expression();
                isword = partypeisword;
                Push();
                c++;
                if (c > proclist[ProcFound].ParCnt) Error("Too many parameters for "+proclist[ProcFound].ProcName);
                //until (Look <> ',");
            }
            while (Look == ',');
            if (c != proclist[ProcFound].ParCnt) Error("Wrong number of parameters for "+proclist[ProcFound].ProcName);
        }
        else {

        }
        i = proclist[ProcFound].LocCnt;
        if (i > 0) {
            for (c = 0; c < i;c++) {
                if (proclist.at(ProcFound).LocTyp.at(c) != "word") {
                    isword = false;
                    a++;
                }
                else {
                    isword = true;
                    a+=2;
                }
                Push();
            }
        }
        writln(outf,"\tCALL\t"+temp+"\t; Call procedure "+temp);
        if (a > 0) {
            if (proclist.at(ProcFound).ReturnIsWord) {
                writln(outf,"\tLP\t0");
                writln(outf,"\tEXAM");
                writln(outf,"\tLDR");
                writln(outf,tr("\tADIA\t%1").arg(a)); pushcnt -= a;
                writln(outf,"\tSTR");
                writln(outf,"\tEXAM");
            }
            else
            if (proclist[ProcFound].hasreturn) {
                writln(outf,"\tEXAB");
                writln(outf,"\tLDR");
                writln(outf,tr("\tADIA\t%1").arg(a)); pushcnt-=a;
                writln(outf,"\tSTR");
                writln(outf,"\tEXAB");
            }
            else {
                if (a < 4) {
                    for (int i = 0 ; i<a; i++)
                    {
                        writln(outf,"\tPOP"); pushcnt--;
                    }
                }
                else
                {
                    writln(outf,"\tLDR");
                    writln(outf,tr("\tADIA\t%1").arg(a)); pushcnt-=a;
                    writln(outf,"\tSTR");
                }
            }
        }
        //                Tok := s;
        if (Tok.startsWith(')')) Tok.remove(0,1);Tok = Tok.trimmed();
        rd(&Look, &Tok);
        writln("LOG",";ProCall(after FindProc):"+Tok);

        // Keep trace of the call o integrate only called procedure
        calledProc.insertMulti(currproc,ProcFound);
    }
    else
        Expected("procedure call");

    writln("LOG",";ProCall(End):"+Tok);
}
//{-------------------------------------------------------------}



/*!
 \brief

 \fn Clcc::repadr
*/
void Clcc::repadr(void) {
    int nbLocale, nbParams, m, a;
    QByteArray name;

    writln("LOG",";repadr:");
    nbLocale = proclist[currproc].LocCnt;
    nbParams = proclist[currproc].ParCnt;

    if ((nbLocale == 0) && (nbParams == 0)) return;

    if (nbLocale > 0) name = proclist.at(currproc).LocName.at(nbLocale - 1);
    else name = proclist[currproc].parname[nbParams - 1];
    if (! FindVarCurrProc(name)) Error("Var "+name+" not declared!");
    if (! varlist[VarFound].local) Error("Var "+name+" not local!");

    m = 0;
    if (nbParams > 0)
        for (int i = 0; i < nbParams; i++)
            if (proclist[currproc].partyp[i] == "word") m+=2; else m++;

    if (nbLocale > 0)
        for (int i=0; i< nbLocale; i++)
            if (proclist[currproc].LocTyp[i] == "word") m+=2; else m++;

    a = 1;
    if (nbParams > 0) {
        for (int i= 0; i< nbParams;i++) {
            name = proclist[currproc].parname[i];
            if (! FindVarCurrProc(name)) Error("Var "+name+" not declared!");
            if (! varlist[VarFound].local) Error("Var "+name+" not local!");
            varlist[VarFound].address = m - a;
            a++;
            if (proclist[currproc].partyp[i] == "word") a++;
        }
    }
    if (nbLocale > 0) {
        for (int i = 0; i< nbLocale; i++) {
            name = proclist[currproc].LocName[i];
            if (! FindVar(name)) Error("Var "+name+" not declared!");
            if (! varlist[VarFound].local) Error("Var "+name+" not local!");
            varlist[VarFound].address = m - a;
            a++;
            if (proclist[currproc].LocTyp[i] == "word") a++;
        }
    }
}



//{--------------------------------------------------------------}
//{  }

/*!
 \brief Parse and Translate an Assignment Statement

 \fn Clcc::Assignment
*/
void Clcc::Assignment(void) {
    QByteArray name,temp,s;
    bool fv;
    QByteArray forml;
    int i, p;
writln("LOG",";Assignement:"+Tok);
    isword = false;
    p = 0;
    if (Tok[0] == '*') {
        p = ptrREF;
        Tok.remove(0,1);
        Tok = Tok.trimmed();
    }
    rd(&Look, &Tok); Tok = Tok.trimmed();
    name = GetName();
    if (FindVar(name)) {
        Cvar var = varlist.at(VarFound);
        if (p == ptrREF) {
            if (! var.pointer && (var.typ == "word")) isword = true;
            if (var.pointer && (var.pnttyp == "word")) isword = true;
        }
        else if (var.typ == "word") isword = true;
    }
    else Error("Var "+name+" not declared!");
    s = "";
    if (Look == '[') {
        s = Tok;
        if (varlist.at(VarFound).pointer) {
            p = ptrREFARR;
            if (varlist.at(VarFound).pnttyp == "word") isword = true;
            else isword = false;
        }
        Tok.remove(0,Tok.indexOf("=")+1); Tok = Tok.trimmed();
    }
    if (QByteArray("+-*/%&|><").contains(Look)) {
        temp = name + Look;
        rd(&Look, &Tok); Tok = Tok.trimmed();
        if ((Look =='<') || (Look=='>')) {
            temp = temp + Look;
            //Rd(Look, Tok); Tok := trim(Tok);
        }
        else if ((Look =='+') || (Look=='-')) {
            if (FindVar(name) && !(varlist[VarFound].typ == "word") && !varlist.at(VarFound).pointer) {
                if (Look == '+') writln(outf,"\t; "+name+"++");
                else writln(outf,"\t; "+name+"--");
                i = varlist[VarFound].address;
                if (varlist[VarFound].local) {
                    // Local char
                    writln(outf,"\tLDR");
                    writln(outf,tr("\tADIA\t%1").arg(varlist[VarFound].address+2+pushcnt));
                    writln(outf,"\tSTP\t;Load variable "+name);
                    if (Look == '+') writln(outf,"\tADIM\t1");
                    else writln(outf,"\tSBIM\t1");
                }
                else if (i == 0) {
                    if (Look=='+') writln(outf,"\tINCI");
                    else writln(outf,"\tDECI");
                }
                else if (i == 1) {
                    if (Look=='+') writln(outf,"\tINCJ");
                    else writln(outf,"\tDECJ");
                }
                else if (i == 2) {
                    if (Look=='+') writln(outf,"\tINCA");
                    else writln(outf,"\tDECA");
                }
                else if (i == 3) {
                    if (Look=='+') writln(outf,"\tINCB");
                    else writln(outf,"\tDECB");
                }
                else if (i == 8) {

                    if (Look=='+') writln(outf,"\tINCK");
                    else writln(outf,"\tDECK");
                }
                else if (i == 9) {
                    if (Look=='+') writln(outf,"\tINCL");
                    else writln(outf,"\tDECL");
                }
                else if (i == 10) {
                    if (Look=='+') writln(outf,"\tINCM");
                    else writln(outf,"\tDECM");
                }
                else if (i == 11) {
                    if (Look=='+') writln(outf,"\tINCN");
                    else writln(outf,"\tDECN");
                }
                else {
                    // Si var in cpu
                    // alors LP x   ADIM 1
                    Cvar lvar = varlist[VarFound];
                    if (!lvar.array && !lvar.xram &&(lvar.typ=="char" || lvar.typ=="byte")) {
                            if (lvar.address < 64) writln(outf,tr("\tLP\t%1").arg(lvar.address)+"\t; Load variable "+name);
                            else writln(outf,tr("\tLIP\t%1").arg(lvar.address)+"\t; Load variable "+name);
                            if (Look == '+') writln(outf,"\tADIM\t1");
                            else writln(outf,"\tSBIM\t1");
                    }
                    else {
                        LoadVariableMain(name);//LoadVariable(name);
                        if (Look == '+') writln(outf,"\tINCA");
                        else writln(outf,"\tDECA");
                        StoreVariable(name);
                    }
                }
                return;
            }
            else {
                temp = name + Look + '1';
                Tok = " ";
            }
        }
        else if (Look == '=') Tok = " "+ Tok;
        Tok.remove(0,1);
        Tok = temp + Tok;
    }
    Tok = Tok.trimmed();
    forml = Tok;
    if (forml[0] == '=') forml.remove(0,1);
    rd(&Look, &Tok); Tok = Tok.trimmed();

    fv = false;
#if 1
    fv = ExpressionType(forml);
#else
    for (int i = 0; i< VarCount;i++) {
        if (find_text(varlist[i].varname, forml) > 0) {
            if (! varlist[i].pointer && (varlist[i].typ == "word")) isword = true;
            if (varlist[i].pointer && (varlist[i].pnttyp == "word")) isword = true;
            fv = true;
        }
    }
#endif
    if (!fv) for (int i=0;i< proccount;i++)
        if (find_text(proclist.at(i).ProcName, forml) > 0) fv = true;

    if (fv) Expression();
    else LoadConstant(forml);
    if (!s.isEmpty()) {
        Push();
        Tok = s;
        rd(&Look, &Tok); Tok = Tok.trimmed();
        Expression();
    }
    if (p == 0) StoreVariable(name);
    else
        if ((p == ptrREF) || (p==ptrREFARR)) {
            writln(outf,tr("\t; Assignment ptrREF(%1) ").arg(p)+name);
        if (FindVar(name)) {
            if (varlist[VarFound].pnttyp != "word") {
                writln(outf,"\tPUSH"); pushcnt++;
            }
            else {
                writln(outf,"\tPUSH"); pushcnt++;
                writln(outf,"\tEXAB");
                writln(outf,"\tPUSH"); pushcnt++;
            }

            LoadVariable(name);
            if (! varlist[VarFound].pointer)
                Error("This var ("+name+") is not a pointer!");
            if (varlist[VarFound].xram) {
                writln(outf,"\tLP\t6\t; YL");
                writln(outf,"\tEXAM");
                writln(outf,"\tLP\t7\t; YH");
                writln(outf,"\tEXAB");
                writln(outf,"\tEXAM");
                writln(outf,"\tDY");
                if (p==ptrREFARR) {
                    // decalage
                    writln(outf,"\tPOP\t\t; retrieve index");pushcnt--;
                    writln(outf,"\tLP 6");
                    writln(outf,"\tLIB 0");
                    writln(outf,"\tADB");
                }
                if (varlist[VarFound].pnttyp != "word") {
                    writln(outf,"\tPOP"); pushcnt--;
                    writln(outf,"\tIYS\t\t; Store content *"+s);
                }
                else {
                    writln(outf,"\tPOP"); pushcnt--;
                    writln(outf,"\tEXAB");
                    writln(outf,"\tPOP"); pushcnt--;
                    writln(outf,"\tIYS\t\t; Store content LB *"+s);
                    writln(outf,"\tEXAB");
                    writln(outf,"\tIYS\t\t; Store content HB *"+s);
                }
            }
            else { // manage offset on non xram pointer
                // LIP
                writln(outf,"\tSTP\t\t; Set P");
                if (varlist[VarFound].pnttyp != "word") {
                    writln(outf,"\tPOP"); pushcnt--;
                    writln(outf,"\tEXAM\t\t; Store content *"+s);
                }
                else {
                    writln(outf,"\tPOP"); pushcnt--;
                    writln(outf,"\tEXAB");
                    writln(outf,"\tPOP"); pushcnt--;
                    writln(outf,"\tEXAM\t\t; Store content LB *"+s);
                    writln(outf,"\tEXAB");
                    writln(outf,"\tEXAM\t\t; Store content HB *"+s);
                }
            }
        }
    }
}
//{--------------------------------------------------------------}


//{---------------------------------------------------------------}
//{  }

/*!
 \brief Parse and Translate a Boolean Expression

 \fn Clcc::BoolExpression
*/
void Clcc::BoolExpression(void) {
    int i;
    i = 0;
    writln("LOG",";BoolExpression:"+Tok);
    while (i < (Tok.length()-1)) {
        if (Tok.mid(i,2) == "||") {
            Tok.remove(i,1); Tok[i]=BO;
        }
        else if (Tok.mid(i,2) == "&&") {
            Tok.remove(i,1); Tok[i]=BA;
        }
        else if (Tok.mid(i,3) == "= =") {
            Tok.remove(i,2); Tok[i]=EQ; i--;
        }
        else if (Tok.mid(i,3) == "> =" ) {
            Tok.remove(i,2); Tok[i]=GE; i--;
        }
        else if (Tok.mid(i,3) == "< =") {
            Tok.remove(i,2); Tok[i]=SE; i--;
        }
        else if (Tok.mid(i,3) == "! =" ) {
            Tok.remove(i,2); Tok[i]=NE; i--;
        }
        else
            i++;
    }
    rd(&Look, &Tok); Tok = Tok.trimmed();
    NotCompTerm();
    while ((Look==BO) || (Look==BA)) {
        Push();
        if (Look==BO) BoolOr();
        if (Look==BA) BoolAnd();
    }
}


//{--------------------------------------------------------------}
//{  }

/*!
 \brief Recognize and Translate a Boolean OR

 \fn Clcc::BoolOr
*/
void Clcc::BoolOr(void) {
   rd(&Look, &Tok); Tok = Tok.trimmed();
   NotCompTerm();
   PopOr();
}

//{--------------------------------------------------------------}
//{  }

/*!
 \brief Recognize and Translate a Boolean AND

 \fn Clcc::BoolAnd
*/
void Clcc::BoolAnd(void) {
   rd(&Look, &Tok); Tok=Tok.trimmed();
   NotCompTerm();
   PopAnd();
}

/*!
 \brief

 \fn Clcc::NotCompTerm
*/
void Clcc::NotCompTerm(void) {
char Sign;

    writln("LOG",";NotCompTerm:"+Tok);
    Sign = Look;
    if (Look == '!') {
                rd(&Look, &Tok); Tok = Tok.trimmed();
            }
    CompTerm();
    if (Sign == '!') NotIt();
}


/*!
 \brief

 \fn Clcc::CompTerm
*/
void Clcc::CompTerm(void) {
    char compOp;
    writln("LOG",";CompTerm:"+Tok);
    if (Look =='(') {
        //Rd(Look, tok); tok := trim(tok);
        BoolExpression();
        rd(&Look, &Tok); Tok = Tok.trimmed();
    }
    else {
        Expression();
        Push();
        compOp = Look;
        rd(&Look, &Tok); Tok = Tok.trimmed();
        Expression();
        switch ((unsigned char)compOp) {
        case '>': CompGreater();break;
        case '<': CompSmaller();break;
        case EQ: CompEqual();break;
        case GE: CompGrOrEq();break;
        case SE: CompSmOrEq();break;
        case NE: CompNotEqual();break;
        }
    }
}

//{--------------------------------------------------------------}
//{  }

/*!
 \brief Generate a Unique Label

 \fn Clcc::NewLabel
 \return QByteArray
*/
QByteArray Clcc::NewLabel(void) {
   LCount++;
   return "LB" + QByteArray::number(LCount-1);
}

//{--------------------------------------------------------------}
//{  }

/*!
 \brief Post a Label To Output

 \fn Clcc::PostLabel
 \param L
*/
void Clcc::PostLabel(QByteArray L) {
   writln(outf,"  "+L+":");
}


//{-------------------------------------------------------------}
//{  }

/*!
 \brief Switch Statement

 \fn Clcc::DoSwitch
*/
void Clcc::DoSwitch(void) {
    QByteArray L1, temp;
    bool iselse;

    Tok.remove(0,7); Tok = Tok.trimmed();
    writln(outf,"\t; Switch");
    rd(&Look, &Tok); Tok = Tok.trimmed();
    Expression();
    writln(outf,"\tCASE");
    iselse = false;
    do {
        GetToken(MODESTR, &dummy);
        temp = ExtrCust(&Tok, ':'); Tok = Tok.trimmed();
        if (!Tok.isEmpty()) {
            dummy = Tok + ";}" + dummy;
            level++;
        }
        Tok = temp;

        if (Tok.contains("else")) iselse = true;
        L1 = NewLabel();
        writln(outf,'\t' + Tok + '\t' + L1);
        outfile = false;
        writln(outf,"  " + L1 + ':');
        Block();
        writln(outf,"\t"+stdOp[OP_RTN]);
        writln(outf,"");
        outfile = true;
    }
    while (dummy.trimmed().at(0) != '}');
    //until trim(dummy)[1] = '}';
    if (!iselse) writln(outf,"\tELSE:\tEOP");
    writln(outf,"\tENDCASE");
    writln(outf,"\t; End switch");
}




//{-------------------------------------------------------------}
//{  }
/*!
 \brief If Statement

 \fn Clcc::DoIf
*/
void Clcc::DoIf(void) {
    QByteArray L1, L2;

    writln(outf,"\t; If block: Boolean expression");
    writln(outf,"");
    Tok.remove(0,4);//delete(Tok, 1, 4);
    Tok = Tok.trimmed();
    BoolExpression();
    if (!Tok.isEmpty()) {
        dummy = Tok + ";}" + dummy;
        level++;
    }
    L1 = NewLabel(); L2 = L1;
    BranchFalse(L1);
    writln(outf,"");
    writln(outf,"\t; If expression = true");
    Block();

    if (dummy.startsWith("else")) {
        GetToken(MODESTR, &dummy);
        Tok.remove(0,4); Tok = Tok.trimmed(); //delete(Tok, 1, 4); Tok:=trim(Tok);
        if (!Tok.isEmpty()) {
            dummy = Tok + ";}" + dummy;
            level++;
        }
        L2 = NewLabel();
        Branch(L2);
        PostLabel(L1);
        writln(outf,"\t; If expression = false");

        Block();
    }
    writln(outf,"\t; End of if");
    PostLabel(L2);
}
//{-------------------------------------------------------------}



//{-------------------------------------------------------------}
//{  }

/*!
 \brief Goto Statement

 \fn Clcc::DoGoto
*/
void Clcc::DoGoto(void) {
    Tok.remove(0,5); Tok = Tok.trimmed();
    writln(outf, "\tRJMP\t"+Tok+"\t; Goto");
    writln(outf,"");
}


//{-------------------------------------------------------------}
//{  }

/*!
 \brief Label Statement

 \fn Clcc::DoLabel
*/
void Clcc::DoLabel(void) {
    Tok.remove(0,6); Tok = Tok.trimmed();//delete(tok, 1, 6); tok := trim(tok);
    if (FindVar(Tok) || FindProc(Tok)) Error(Tok+": This label name is already used!");
    writln(outf,"");
    writln(outf,"  "+Tok+":\t; User label");
}


//{-------------------------------------------------------------}
//{  }

/*!
 \brief Break Statement

 \fn Clcc::DoBreak
*/
void Clcc::DoBreak(void) {
    if (InnerLoop == "loop") writln(outf,"\tLEAVE\t\t; Break");
    else writln(outf,"\tRJMP\t"+ExitLabel+"\t; Break");
    writln(outf,"");
}


//{-------------------------------------------------------------}
//{  }

/*!
 \brief Exit Statement

 \fn Clcc::DoReturn
*/
void Clcc::DoReturn(void) {
    Tok.remove(0,6); Tok = Tok.trimmed();
    if (!Tok.isEmpty()) {
        rd(&Look, &Tok); Tok = Tok.trimmed();
        isword = proclist[currproc].ReturnIsWord;
        Expression();
    }
    writln(outf,"\t"+stdOp[OP_RTN]+"\t\t; Return");
    writln(outf,"");
}


//{-------------------------------------------------------------}
//{  }

/*!
 \brief Loop Statement

 \fn Clcc::DoLoop
*/
void Clcc::DoLoop(void) {
    QByteArray L1,L2;

    InnerLoop = "loop";
    L1 = NewLabel();
    L2 = NewLabel();
    ExitLabel = L2;
    writln(outf,"\t; Loop");
    writln(outf,"");
    Tok.remove(0,6); Tok = Tok.trimmed();
    rd(&Look, &Tok); Tok = Tok.trimmed();
    Expression();
    isword = false;
    Push();pushcnt--;
    PostLabel(L1);
    if (!Tok.isEmpty()) {
        dummy = Tok + ";}" + dummy;
        level++;
    }
    Block();
    writln(outf,"\tLOOP\t"+L1);
    PostLabel(L2);
    writln(outf,"\t; End of loop");
}


//{-------------------------------------------------------------}
//{  }

/*!
 \brief While Statement

 \fn Clcc::DoWhile
*/
void Clcc::DoWhile(void) {
    QByteArray L1,L2;

    InnerLoop = "while";
    L1 = NewLabel();
    L2 = NewLabel();
    ExitLabel = L2;
    writln(outf,"\t; While");
    writln(outf,"");
    PostLabel(L1);
    Tok.remove(0,7);
    BoolExpression();
    BranchFalse(L2);
    writln(outf,"\t; While expression = true");
    if (!Tok.isEmpty()) {
        dummy = Tok + ";}" + dummy;
        level++;
    }
    Block();
    writln(outf,"\tRJMP\t"+L1);
    PostLabel(L2);
    writln(outf,"\t; End of while");
}


//{-------------------------------------------------------------}
//{  }

/*!
 \brief For Statement

 \fn Clcc::DoFor
*/
void Clcc::DoFor(void) {
    QByteArray L1, L2, temp;
    QByteArray afterop;

    afterop ="";
    InnerLoop = "for";
    Tok.remove(0,5); Tok = Tok.trimmed();
    writln(outf,"\t; For loop");
    Assignment();
    L1 = NewLabel();
    L2 = NewLabel();
    ExitLabel = L2;
    PostLabel(L1);
    GetToken(MODESTR, &dummy);
    BoolExpression();
    BranchFalse(L2);
    if (dummy.trimmed().at(0) != ')') {
        GetToken(MODESTR, &dummy);
        if (Tok.endsWith(')')) {
            Tok.remove(Tok.length()-1,1);//delete(tok, length(tok), 1);
            Tok = Tok.trimmed();
            afterop = Tok;
            //if (!Tok.isEmpty()) dummy = Tok +';'+ dummy;
        }
        else {
            temp = ExtrCust(&Tok, ')');
            dummy = temp + ';' + Tok + ";}" + dummy;
            level++;
        }
    }
    else {
        GetToken(MODESTR, &dummy);
        if (Tok != ")") {
            Tok.remove(0,1);Tok = Tok.trimmed();
            dummy = Tok + ";}" + dummy;
            level++;
        }
    }

    Block();
    QByteArray savedummy = dummy;
    dummy = afterop+";";
    Block();
    dummy = savedummy;
    writln(outf,"\tRJMP\t"+L1);
    PostLabel(L2);
    writln(outf,"\t; End of for");
}

//{--------------------------------------------------------------}
//{ Inserts library code for used libs }
void Clcc::addlib(int libid) {
    if (libcnt == 0)
        libtext = libtext + "; LIB Code\r\n";
    libcnt++;

    if (!libins.contains(libid)) {
        libins.append(libid);
        libtext.append(stdOp[OP_INCLUDE]+" "+libname[libid]+".lib\r\n");
    }
}


void  Clcc::addasm(QByteArray s) {
    asmlist.append(s+"\r\n");
    asmcnt++;
}

/*!
 \brief

 \fn Clcc::DoLoad
*/
void Clcc::DoLoad(void) {
    Tok.remove(0,5);
    rd(&Look, &Tok);
    Expression();
}


/*!
 \brief

 \fn Clcc::DoSave
*/
void Clcc::DoSave(void) {
    QByteArray name;

    Tok.remove(0,5);
    rd(&Look, &Tok);
    Tok = Tok.trimmed();
    name = GetName();
    StoreVariable(name);
}

//{-------------------------------------------------------------}
//{  }

/*!
 \brief Do..While Statement

 \fn Clcc::DoDoWhile
*/
void Clcc::DoDoWhile(void) {
    QByteArray L1, L2;

    InnerLoop = "do";
    L1 = NewLabel();
    L2 = NewLabel();
    ExitLabel = L2;
    writln(outf,"\t; Do..while");
    writln(outf,"");
    PostLabel(L1);
    //        GetToken(MODESTR, dummy);
    ExtrWord(&Tok); Tok = Tok.trimmed();
    if (!Tok.isEmpty()) {
        dummy = Tok + ";}" + dummy;
        level++;
    }
    Block();
    GetToken(MODESTR, &dummy);
    Tok.remove(0,7);//delete(Tok, 1, 7);
    Tok.remove(Tok.length()-1,1);//delete(Tok, length(Tok), 1);
    BoolExpression();
    BranchFalse(L2);
    writln(outf,"\t; While expression = true");
    writln(outf,"\tRJMP\t"+L1);
    PostLabel(L2);
    writln(outf,"\t; End of do..while");
}

/*!
 \brief

 \fn Clcc::DoSaveState
*/
void Clcc::DoSaveState(void) {
    if (LState.isEmpty()) {
        LState = NewLabel();
    }
    writln(outf,"\t; Save CPU state");
    writln(outf,"\tLP\t0");
    writln(outf,"\tLIDP\t"+LState);
    writln(outf,"\tLII\t0x5F");
    writln(outf,"\tEXWD");
    writln(outf,"");
    Tok.remove(0,5).prepend("#restore");
    DoRestoreState();
}

/*!
 \brief

 \fn Clcc::DoRestoreState
*/
void Clcc::DoRestoreState(void) {
    if (LState.isEmpty()) {
        if (showErrors) QMessageBox::about(mainwindow,"ERROR","Restore with no previous save!!!");
        return;
    }
    writln(outf,"\tLP\t0");
    writln(outf,"\tLIDP\t"+LState);
    writln(outf,"\tLII\t0x5F");
    writln(outf,"\tMVWD");
    writln(outf,"");
    Tok.remove(0,8);

}

/*!
 \brief

 \fn Clcc::DoSaveState
*/
void ClccPC1500::DoSaveState(void) {
    if (LState.isEmpty()) {
        LState = NewLabel();
    }
    writln(outf,"\t; Save CPU state");
    writln(outf,"\tLP\t0");
    writln(outf,"\tLIDP\t"+LState);
    writln(outf,"\tLII\t0x5F");
    writln(outf,"\tEXWD");
    writln(outf,"");
    Tok.remove(0,5).prepend("#restore");
    DoRestoreState();
}

/*!
 \brief

 \fn Clcc::DoRestoreState
*/
void ClccPC1500::DoRestoreState(void) {
    if (LState.isEmpty()) {
        if (showErrors) QMessageBox::about(mainwindow,"ERROR","Restore with no previous save!!!");
        return;
    }
    writln(outf,"\tLP\t0");
    writln(outf,"\tLIDP\t"+LState);
    writln(outf,"\tLII\t0x5F");
    writln(outf,"\tMVWD");
    writln(outf,"");
    Tok.remove(0,8);

}
//{--------------------------------------------------------------}
//{  }

/*!
 \brief Parse and Translate a Block

 \fn Clcc::Block
*/
void Clcc::Block(void) {
    QByteArray name;

    writln("LOG",";BLOCK:"+dummy);
    do {
        GetToken(MODESTR, &dummy);
        Tok = Tok.trimmed();
        name = Tok.mid(0,5);
        if ((name == "byte ") || (name == "char ") || (name == "word "))    // Local var definition
                vardecl();
        else if (firstp) {
            firstp = false;
            repadr();
        }
        if (!Tok.isEmpty()) {
          if (Tok.startsWith("if ")) DoIf();
          else if (Tok.startsWith("load "))     DoLoad();
          else if (Tok.startsWith("save "))     DoSave();
          else if (Tok.startsWith("switch "))   DoSwitch();
          else if (Tok.startsWith("loop "))     DoLoop();
          else if (Tok.startsWith("while "))    DoWhile();
          else if (Tok.startsWith("for "))      DoFor();
          else if (Tok.startsWith("goto "))     DoGoto();
          else if (Tok.startsWith("label "))    DoLabel();
          else if (Tok.contains(":"))           DoLabel();
          else if ((Tok.mid(0,2) == "do") &&
                  !Alpha.contains(toupper(Tok[3])))    DoDoWhile();
          else if (Tok == "break")              DoBreak();
          else if (Tok.startsWith("return"))    DoReturn();
          else if (Tok.startsWith("#save"))     DoSaveState();
          else if (Tok.startsWith("#restore"))  DoRestoreState();
          else if (Tok.startsWith("#asm")) {
                Tok = ExtrCust(&dummy, 0x0d);
                while (!Tok.startsWith("#endasm")) {
                        writln(outf,"\t"+Tok);
                        Tok = ExtrCust(&dummy, 0x0D );
                    }
            }
          else if (Tok.contains("=")) Assignment();
          else if (Tok.contains("(")) {
              ProcCall();
          }
          else Assignment();
          writln(outf,"");
      }
        dummy = dummy.trimmed();
    }
    while (!( dummy.trimmed().isEmpty() || dummy.trimmed().startsWith("}")));
    if (!dummy.isEmpty()) rd(&Look, &dummy);
    dummy = dummy.trimmed();
}
//{--------------------------------------------------------------}



/*!
 \brief

 \fn Clcc::removelocvars
 \param pn
*/
void Clcc::removelocvars(QByteArray pn) {
    int i;

    i = 0;
    if (! FindProc(pn)) Error("function "+pn+" unknown!");
    if (proclist[ProcFound].ParCnt == 0) return;

    while (i < VarCount) {
        if (varlist.at(i).local && (varlist.at(i).locproc == ProcFound)) {
            varlist.removeAt(i);
            VarCount--;
        }
        else {

            i++;
        }
    }

}


/*{--------------------------------------------------------------}
{  }
*/
/*!
 \brief Do First Scan

 \fn Clcc::FirstScan
 \param filen
*/
void Clcc::FirstScan(QByteArray filen) {
    QByteArray name, name2, t, temp, s;
    int i;
    bool hasret;
    QByteArray src= filen;

    GetToken(MODESTR, &filen);
    int z= Tok.length();
    src = filen.mid(z);
    while (Tok != "")
    {
        name = Tok.mid(0,4);//copy(Tok,1,4); //ExtrWord(Tok);
        if ((level == 0) && ((name == "byte") || (name == "char") || (name == "word")))
        {
            Tok.remove(0,4); Tok = Tok.trimmed();
            Tok = name + " " + Tok;
            vardecl();
        }
        else if (Tok.startsWith("#org ")) {
            ExtrWord(&Tok);
            org = QByteArray::number(mathparse(Tok.trimmed(),16));
        }
        else if (Tok.startsWith("#pc ")) { ExtrWord(&Tok); pc = Tok.trimmed(); }
        else if (Tok.startsWith("#nosave")) nosave = true;
        else if (Tok.contains('('))
        {
            name2 = ExtrWord(&Tok);
            isword = false;
            hasret = false;
            if (name == "word")
            {
                isword = true;
                name = ExtrWord(&Tok);
                hasret = true;
            }
            else if ((name == "char") || (name == "byte"))
            {
                name = ExtrWord(&Tok);
                hasret = true;
            }
            else {
                name = name2;
            }
            Tok = Tok.trimmed(); Tok.remove(0,1); Tok = Tok.trimmed();
            i = 0;
            temp = "";
            QList<QByteArray> loc_partyp;
            QList<QByteArray> loc_parname;
            if (!Tok.startsWith(")"))
            {
                Tok.remove(Tok.size()-1,1);
                temp = Tok;
                s = Tok + ',';
                currproc = proccount;

                while (!s.isEmpty()) {
                    Tok = ExtrList(&s);
#if 1
                    // new part to parse pointer in parameters
                    QByteArray locname = Tok.mid(0,4);
                    if ((locname == "byte") || (locname == "char") || (locname == "word"))
                    {
                        Tok.remove(0,4); Tok = Tok.trimmed();
                        Tok = locname + " " + Tok;
                    }
#endif  
                    //loc_partyp.append(Tok.left(4));//copy(Tok, 1, 4);
                    procd = true;
                    loc_parname.append(vardecl());
                    loc_partyp.append(varlist.last().typ);
                    procd = false;
                    i++;
                }
            }
            t = "";
            do {
                //QByteArray src = "dummy";
                rd(&Look, &filen);
                t = t + Look;
            } while ( (level != 0) && (Look != 27));
            t.remove(t.size()-1,1);
            AddProc(name, t.trimmed(), temp, i, hasret, isword,loc_partyp,loc_parname);
        }
        GetToken(MODESTR, &filen);
        src = src.mid(src.indexOf(Tok)+Tok.size());
    }


    if (showErrors && !FindProc("main")) {
        if (showErrors) QMessageBox::about(mainwindow,"ERROR","main not found!");
    }

    printvarlist("output");
    printproclist("output");
}


void Clcc::generateProcCode(QString f,int i) {
    proclist[i].start = asmtext.size();
    writln(f,"");
    writln( f,proclist[i].ProcName+":\t; Procedure");
    dummy = proclist[i].ProcCode;
    level = 1;
    currproc = i;
    pushcnt = 0; firstp = true;
    Block();
    if (pushcnt != 0) Error(proclist[i].ProcName+": Possible Stack corruption!");
    removelocvars(proclist[i].ProcName);
    if (proclist[i].ProcName == "main") {
        writln(f, " EOP:\t"+stdOp[OP_RTN]+"");
    }
    else {
        writln(f,"\t"+stdOp[OP_RTN]+"");
    }
    writln(f,"");
    proclist[i].end = asmtext.size();
    insertedProc.append(i);
}

void Clcc::save() {
    writln(outf,"\tLP\t0");
    writln(outf,"\tLIDP\tSREG");
    writln(outf,"\tLII\t11");
    writln(outf,"\tEXWD");
}

void Clcc::restore() {
    writln(outf,"\tLP\t0");
    writln(outf,"\tLIDP\tSREG");
    writln(outf,"\tLII\t11");
    writln(outf,"\tMVWD");
    writln(outf,"\tRTN");
    writln(outf,"");
    writln(outf,"SREG:\t.DW 0, 0, 0, 0, 0, 0");
}

void ClccPC1500::save() {
    writln(outf,"\tPUSH\tA");
    writln(outf,"\tPUSH\tHL");
    writln(outf,"\tPUSH\tBC");
    writln(outf,"\tPUSH\tDE");
}

void ClccPC1500::restore() {
    writln(outf,"\tPOP\tDE");
    writln(outf,"\tPOP\tBC");
    writln(outf,"\tPOP\tHL");
    writln(outf,"\tPOP\tA");
}


/*!
 \brief

 \fn Clcc::SecondScan
*/
void Clcc::SecondScan(void) {

    QByteArray name, typ, s, s2, s3;
    bool    at, arr;
    int    adr, size, value;


    QString f = outf;

    insertedProc.clear();
    calledProc.clear();
    sourceinASM = true;

    //{ Write Intro }
    writln(f, "; pasm file - assemble with pasm!");
    writln(f, "; Compiled with lcc v1.0");
    writln(f,"");
    writln(f,".ORG\t"+org);
    writln(f,"");
    if (!nosave) {
        save();
    }


    for (int i=0; i<varlist.size();i++) {
        Cvar v = varlist[i];
        adr = v.address;
        size = (v.pointer ? (v.typ =="word"? 2 : 1): v.size);
        name = v.varname;
        value = v.initn;
        typ = v.typ;
        at = v.at;
        arr = v.array;

        if (!v.local){
            if (v.xram) {
                if (at) {
                    if (value !=-1) {
                        if (!arr) varxram(value, adr, size, name);
                        else varxarr(varlist[i].inits, adr, size, name, typ);
                    }
                }
                else {
                    if (!arr) varcode(value, adr, size, name);
                    else varcarr(varlist[i].inits, adr, size, name, typ);
                }
            }
            else {
                if (value != -1) {
                    if (!arr)  varreg(value, adr, size, name);
                    else varrarr(v.inits, adr, size, name, typ);
                }
            }
        }
    }


    Tok = "main ()";
    currproc = -1;
    ProcCall();
    restore();

//        { Process procedures }
    for (int i = 0; i< proclist.size();i++) {
            generateProcCode(f,i);
    }

    insertedProc.clear();
    insertedProc.append(proclist.size()-1);
    for(int i=proclist.size()-1;i>=0;i--) {
        if (insertedProc.contains(i)) {
            QList<int> childs = calledProc.values(i);
            insertedProc.append(childs);
        }
    }
    for(int i=proclist.size()-1;i>=0;i--) {
        if ( !insertedProc.contains(i)) {
            // remove from asmtext
            asmtext.remove(proclist.at(i).start,
                           proclist.at(i).end - proclist.at(i).start);
        }
    }


    if (!LState.isEmpty()) {
        // ADD the savestate memory array
        QString state ="";
        for (int i=0;i<0x30;i++) state.append("0,");
        state.chop(1);
        writln(f,LState+":\t.DW "+state);
    }

    if (asmcnt > 0)
        for (int i = 0;i<asmcnt;i++) writln(f, asmlist[i]);


        //{ Second pass: Optimize Code }

        s2 = "\tINCA";
        s3 = "\tDECA";
// A REVOIR !!!!!!!!!!!!
//        for (int i = 1 ;i <= 6; i++) {
//            s = "\tLIA\t"+QString("%1").arg(i)+"\t; Load constant "+QString("%1").arg(i)+"\r\tEXAB\r\tLP\t3\r\tSBM\t\t; Subtraction\r\tEXAB";
//            while (asmtext.contains(s)) {
//                asmtext.remove(asmtext.indexOf(s), s.length());
//                insert(s2, asmtext, pos(s, asmtext));
//                s2 = s2 + "\r\tDECA";
//            }
//            s = "\tLIA\t"+QString("%1").arg(i)+"\t; Load constant "+QString("%1").arg(i)+"\r\tLP\t3\r\tADM\t\t; Addition\r\tEXAB";
//            while (asmtext.contains(s)) {
//                asmtext.remove(asmtext.indexOf(s), s.length());
//                insert(s3, asmtext, pos(s, asmtext));
//                s3 = s3 + "\r\tINCA";
//            }
//        }
        s = "\tEXAB\r\tEXAB\r";
        while (asmtext.contains(s))
            asmtext.remove(asmtext.indexOf(s), s.length());

        writeln(f, asmtext);
        writeln(f, libtext);


#if 0
        {
        // Replace LIA... EXAB to LIB
        assignfile(f, 'temp.asm');
        reset(f);
        assignfile(f2, 'temp2.asm');
        rewrite(f2);

        while not eof(f) do
        begin
                readln(f, s);
                if copy(trim(s),1,3) = 'LIA' then
                begin
                        name := s;
                        readln(f, s);
                        if copy(trim(s),1,4) = 'EXAB' then
                        begin
                                s := #9'LIB' + copy(name,5,255);
                                writeln(f2, s);
                        end else
                        begin
                                writeln(f2, name);
                                writeln(f2, s);
                        end;
                end else
                        writeln(f2, s);
        end;
        closefile(f2);
        closefile(f);

        // Replace PUSH LIB... POP to LIB...
        assignfile(f, 'temp2.asm');
        reset(f);
        assignfile(f2, 'temp.asm');
        rewrite(f2);

        while not eof(f) do
        begin
                readln(f, s);
                if copy(trim(s),1,4) = 'PUSH' then
                begin
                        name := s;
                        readln(f, s);
                        if copy(trim(s),1,3) = 'LIB' then
                        begin
                                typ := s;
                                readln(f, s);
                                if copy(trim(s),1,3) = 'POP' then
                                begin
                                        writeln(f2, typ);
                                end else
                                begin
                                        writeln(f2, name);
                                        writeln(f2, typ);
                                        writeln(f2, s);
                                end;
                        end else
                        begin
                                writeln(f2, name);
                                writeln(f2, s);
                        end;
                end else
                begin
                        writeln(f2, s);
                end;
        end;
        closefile(f2);
        closefile(f);

        // Remove double EXAB
        assignfile(f, 'temp.asm');
        reset(f);
        assignfile(f2, 'temp2.asm');
        rewrite(f2);

        while not eof(f) do
        begin
                readln(f, s);
                if copy(trim(s),1,3) = 'EXAB' then
                begin
                        name := s;
                        readln(f, s);
                        if copy(trim(s),1,4) <> 'EXAB' then
                        begin
                                writeln(f2, name);
                                writeln(f2, s);
                        end;
                end else
                        writeln(f2, s);
        end;
        closefile(f2);
        closefile(f);

        // Replace n++ code to INCA
        assignfile(f, 'temp2.asm');
        reset(f);
        assignfile(f2, 'temp.asm');
        rewrite(f2);

        while not eof(f) do
        begin
                readln(f, s);
                if (copy(trim(s),1,6) = 'LIB\t1'#9) or (s = 'LIB\t1') then
                begin
                        name := s;
                        readln(f, s);
                        if copy(trim(s),1,4) = 'LP\t3' then
                        begin
                                typ := s;
                                readln(f, s);
                                if copy(trim(s),1,3) = 'ADM' then
                                begin
                                        s2 := s;
                                        readln(f, s);
                                        if copy(trim(s),1,4) = 'EXAB' then
                                        begin
                                                writeln(f2, #9'INCA');
                                        end else
                                        begin
                                                writeln(f2, name);
                                                writeln(f2, typ);
                                                writeln(f2, s2);
                                                writeln(f2, s);
                                        end;
                                end else
                                begin
                                        writeln(f2, name);
                                        writeln(f2, typ);
                                        writeln(f2, s);
                                end;
                        end else
                        begin
                                writeln(f2, name);
                                writeln(f2, s);
                        end;
                end else
                begin
                        writeln(f2, s);
                end;
        end;
        closefile(f2);
        closefile(f);

        // Replace n-- code to DECA
        assignfile(f, 'temp.asm');
        reset(f);
        assignfile(f2, filen);
        rewrite(f2);
        i := 0;

        while not eof(f) do
        begin
                readln(f, s);
                if (copy(trim(s),1,6) = 'LIB\t1'#9) or (s = 'LIB\t1') then
                begin
                        name := s;
                        readln(f, s);
                        if copy(trim(s),1,4) = 'LP\t3' then
                        begin
                                typ := s;
                                readln(f, s);
                                if copy(trim(s),1,4) = 'EXAB' then
                                begin
                                        s2 := s;
                                        readln(f, s);
                                        if copy(trim(s),1,3) = 'SBM' then
                                        begin
                                                s3 := s;
                                                readln(f, s);
                                                if copy(trim(s),1,4) = 'EXAB' then
                                                begin
                                                        writeln(f2, #9'DECA'); i++;
                                                end else
                                                begin
                                                        writeln(f2, name); if name<> '' then inc(i);
                                                        writeln(f2, typ); if typ<> '' then inc(i);
                                                        writeln(f2, s2); if s2<> '' then inc(i);
                                                        writeln(f2, s3); if s3<> '' then inc(i);
                                                        writeln(f2, s); if s<> '' then inc(i);
                                                end;
                                        end else
                                        begin
                                                writeln(f2, name); if name<> '' then inc(i);
                                                writeln(f2, typ); if typ<> '' then inc(i);
                                                writeln(f2, s2); if s2<> '' then inc(i);
                                                writeln(f2, s); if s<> '' then inc(i);
                                        end;
                                end else
                                begin
                                        writeln(f2, name); if name<> '' then inc(i);
                                        writeln(f2, typ); if typ<> '' then inc(i);
                                        writeln(f2, s); if s<> '' then inc(i);
                                end;
                        end else
                        begin
                                writeln(f2, name); if name<> '' then inc(i);
                                writeln(f2, s); if s<> '' then inc(i);
                        end;
                end else
                begin
                        writeln(f2, s);  if s<> '' then inc(i);
                end;
        end;
        closefile(f2);
        closefile(f);

        deletefile('temp.asm');
        deletefile('temp2.asm');
    }
#endif
        writeln("output",tr("Complete: %1 assembler lines were produced!").arg(asmcnt));
    }
//{--------------------------------------------------------------}


/*!
 \brief

 \fn Clcc::run
*/
void Clcc::run() {

    // Fetch all sources and precomp
    QMapIterator<QString, QByteArray> i(*sources);
    while (i.hasNext()) {
        i.next();
        inpf = i.key();
        QFileInfo fInfo(inpf);
        if (inpf != "output") {
            outf = fInfo.baseName()+".asm";
            FirstScan(i.value());
            SecondScan();
        }
    }

}


#endif
