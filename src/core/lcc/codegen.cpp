#include "lcc.h"


extern MainWindowPockemul *mainwindow;

void Clcc::LoadVariableMain(QByteArray s) {

    if (sourceinASM) writln(outf,"\t; LoadVariable : "+s);

    if ( (pointer == ptrREF) || (pointer == ptrREFARR)) {
        if (pointer == ptrREFARR) {
            writln(outf,"\tPUSH\t\t; store the offset");pushcnt++;
        }
        LoadVariable(s);
        if (! varlist[VarFound].pointer) Error("This var ("+s+") is not a pointer!");
        if (varlist[VarFound].xram) {
            writln(outf,"\tLP\t4\t; XL");
            writln(outf,"\tEXAM");
            writln(outf,"\tLP\t5\t; XH");
            writln(outf,"\tEXAB");
            writln(outf,"\tEXAM");
            writln(outf,"\tDX");
            if (pointer==ptrREFARR) {
                // decalage
                writln(outf,"\tPOP\t\t; retrieve index");pushcnt--;
                writln(outf,"\tLP 4");
                writln(outf,"\tLIB 0");
                writln(outf,"\tADB");
            }
            if (varlist[VarFound].pnttyp != "word") {
                writln(outf,"\tIXL\t\t; Load content *"+s);
                writln(outf,"\tLIB\t0\t; Load 0 in HB"+s);
            }
            else {
                writln(outf,"\tIXL\t\t; Load content LB *"+s);
                writln(outf,"\tEXAB");
                writln(outf,"\tIXL\t\t; Load content HB *"+s);
                writln(outf,"\tEXAB");
            }
        }
        else {
            // LIP
            writln(outf,"\tSTP\t\t; Set P");
            if (varlist[VarFound].pnttyp != "word") {
                writln(outf,"\tLDM\t\t; Load content *"+s);
            }
            else {
                writln(outf,"\tLDM\t\t; Load content LB *"+s);
                writln(outf,"\tEXAB");
                writln(outf,"\tINCP");
                writln(outf,"\tLDM\t\t; Load content HB *"+s);
                writln(outf,"\tEXAB");
            }
        }
    }
    else
    if (pointer == ptrADR) {
        if (varlist[VarFound].xram) {
            if (varlist[VarFound].address == -1) {
                writln(outf,"\tLIA\tLB("+s+")\t; &"+s);
                writln(outf,"\tLIB\tHB("+s+")\t; &"+s);
            }
            else {
                writln(outf,tr("\tLIA\tLB(%1)\t; &").arg(varlist[VarFound].address)+s);
                writln(outf,tr("\tLIB\tHB(%1)\t; &").arg(varlist[VarFound].address)+s);
            }
        }
        else {
            writln(outf,tr("\tLIA\t%1\t; &").arg(varlist[VarFound].address)+s);
        }
    }
    else
        LoadVariable(s);
}

void ClccPC1500::LoadVariableMain(QByteArray s) {

    if (sourceinASM) writln(outf,"\t; LoadVariable : "+s);

    if ( (pointer == ptrREF) || (pointer == ptrREFARR)) {
        if (pointer == ptrREFARR) {
            writln(outf,"\tPUSH\t\t; store the offset");pushcnt++;
        }
        LoadVariable(s);
        if (! varlist[VarFound].pointer) Error("This var ("+s+") is not a pointer!");
        if (varlist[VarFound].xram) {
            writln(outf,"\tLP\t4\t; XL");
            writln(outf,"\tEXAM");
            writln(outf,"\tLP\t5\t; XH");
            writln(outf,"\tEXAB");
            writln(outf,"\tEXAM");
            writln(outf,"\tDX");
            if (pointer==ptrREFARR) {
                // decalage
                writln(outf,"\tPOP\t\t; retrieve index");pushcnt--;
                writln(outf,"\tLP 4");
                writln(outf,"\tLIB 0");
                writln(outf,"\tADB");
            }
            if (varlist[VarFound].pnttyp != "word") {
                writln(outf,"\tIXL\t\t; Load content *"+s);
                writln(outf,"\tLIB\t0\t; Load 0 in HB"+s);
            }
            else {
                writln(outf,"\tIXL\t\t; Load content LB *"+s);
                writln(outf,"\tEXAB");
                writln(outf,"\tIXL\t\t; Load content HB *"+s);
                writln(outf,"\tEXAB");
            }
        }
        else {
            // LIP
            writln(outf,"\tSTP\t\t; Set P");
            if (varlist[VarFound].pnttyp != "word") {
                writln(outf,"\tLDM\t\t; Load content *"+s);
            }
            else {
                writln(outf,"\tLDM\t\t; Load content LB *"+s);
                writln(outf,"\tEXAB");
                writln(outf,"\tINCP");
                writln(outf,"\tLDM\t\t; Load content HB *"+s);
                writln(outf,"\tEXAB");
            }
        }
    }
    else
    if (pointer == ptrADR) {
        if (varlist[VarFound].xram) {
            if (varlist[VarFound].address == -1) {
                writln(outf,"\tLIA\tLB("+s+")\t; &"+s);
                writln(outf,"\tLIB\tHB("+s+")\t; &"+s);
            }
            else {
                writln(outf,tr("\tLIA\tLB(%1)\t; &").arg(varlist[VarFound].address)+s);
                writln(outf,tr("\tLIB\tHB(%1)\t; &").arg(varlist[VarFound].address)+s);
            }
        }
        else {
            writln(outf,tr("\tLIA\t%1\t; &").arg(varlist[VarFound].address)+s);
        }
    }
    else
        LoadVariable(s);
}

void Clcc::LoadVariableArray(Cvar v) {

    if ((v.typ=="char") || (v.typ=="byte")) {
        if (!v.xram) {
            writln(outf,tr("\tLIB\t%1").arg(v.address)+"\t; Load array element from "+v.varname);
            writln(outf,"\tLP\t3");
            writln(outf,"\tADM");
            writln(outf,"\tEXAB");
            writln(outf,"\tSTP");
            writln(outf,"\tLDM");
        }
        else {
            writln(outf,"\tPUSH\t\t; Load array element from "+v.varname); pushcnt++;
            writln(outf,"\tLP\t5\t; HB of address");
            if (v.address !=-1) {
                writln(outf,tr("\tLIA\tHB(%1-1)").arg(v.address));
                writln(outf,"\tEXAM");
                writln(outf,"\tLP4\t; LB");
                writln(outf,tr("\tLIA\tLB(%1-1)").arg(v.address));
            }
            else {
                writln(outf,"\tLIA\tHB("+v.varname+"-1)");
                writln(outf,"\tEXAM");
                writln(outf,"\tLP\t4\t; LB");
                writln(outf,"\tLIA\tLB("+v.varname+"-1)");
            }
            writln(outf,"\tEXAM");
            writln(outf,"\tPOP"); pushcnt--;
            writln(outf,"\tLIB\t0");
            writln(outf,"\tADB");
            writln(outf,"\tIXL");
        }
    }
    else {
        if (!v.xram) {
            writln(outf,"\tRC");
            writln(outf,"\tSL");
            writln(outf,tr("\tLII\t%1").arg(v.address)+"\t; Store array element from "+v.varname);
            writln(outf,"\tLP\t0");
            writln(outf,"\tADM");
            writln(outf,"\tEXAM");
            writln(outf,"\tSTP");
            writln(outf,"\tLDM");
            writln(outf,"\tEXAB");
            writln(outf,"\tINCP");
            writln(outf,"\tLDM");
            writln(outf,"\tEXAB");
        }
        else {
            writln(outf,"\tRC");
            writln(outf,"\tSL");
            writln(outf,"\tPUSH\t\t; Load array element from "+v.varname); pushcnt++;
            writln(outf,"\tLP\t5\t; HB of address");
            if (v.address !=-1) {
                writln(outf,tr("\tLIA\tHB(%1-1)").arg(v.address));
                writln(outf,"\tEXAM");
                writln(outf,"\tLP\t4\t; LB");
                writln(outf,tr("\tLIA\tLB(%1-1)").arg(v.address));
            }
            else {
                writln(outf,"\tLIA\tHB("+v.varname+"-1)");
                writln(outf,"\tEXAM");
                writln(outf,"\tLP\t4\t; LB");
                writln(outf,"\tLIA\tLB("+v.varname+"-1)");
            }
            writln(outf,"\tEXAM");
            writln(outf,"\tPOP"); pushcnt--;
            writln(outf,"\tLIB\t0");
            writln(outf,"\tADB");
            writln(outf,"\tIXL");
            writln(outf,"\tEXAB");
            writln(outf,"\tIXL");
            writln(outf,"\tEXAB");
        }
    }
}
void ClccPC1500::LoadVariableArray(Cvar v) {

    if ((v.typ=="char") || (v.typ=="byte")) {
        if (!v.xram) {
            writln(outf,tr("\tLIB\t%1").arg(v.address)+"\t; Load array element from "+v.varname);
            writln(outf,"\tLP\t3");
            writln(outf,"\tADM");
            writln(outf,"\tEXAB");
            writln(outf,"\tSTP");
            writln(outf,"\tLDM");
        }
        else {
            writln(outf,"\tPUSH\t\t; Load array element from "+v.varname); pushcnt++;
            writln(outf,"\tLP\t5\t; HB of address");
            if (v.address !=-1) {
                writln(outf,tr("\tLIA\tHB(%1-1)").arg(v.address));
                writln(outf,"\tEXAM");
                writln(outf,"\tLP4\t; LB");
                writln(outf,tr("\tLIA\tLB(%1-1)").arg(v.address));
            }
            else {
                writln(outf,"\tLIA\tHB("+v.varname+"-1)");
                writln(outf,"\tEXAM");
                writln(outf,"\tLP\t4\t; LB");
                writln(outf,"\tLIA\tLB("+v.varname+"-1)");
            }
            writln(outf,"\tEXAM");
            writln(outf,"\tPOP"); pushcnt--;
            writln(outf,"\tLIB\t0");
            writln(outf,"\tADB");
            writln(outf,"\tIXL");
        }
    }
    else {
        if (!v.xram) {
            writln(outf,"\tRC");
            writln(outf,"\tSL");
            writln(outf,tr("\tLII\t%1").arg(v.address)+"\t; Store array element from "+v.varname);
            writln(outf,"\tLP\t0");
            writln(outf,"\tADM");
            writln(outf,"\tEXAM");
            writln(outf,"\tSTP");
            writln(outf,"\tLDM");
            writln(outf,"\tEXAB");
            writln(outf,"\tINCP");
            writln(outf,"\tLDM");
            writln(outf,"\tEXAB");
        }
        else {
            writln(outf,"\tRC");
            writln(outf,"\tSL");
            writln(outf,"\tPUSH\t\t; Load array element from "+v.varname); pushcnt++;
            writln(outf,"\tLP\t5\t; HB of address");
            if (v.address !=-1) {
                writln(outf,tr("\tLIA\tHB(%1-1)").arg(v.address));
                writln(outf,"\tEXAM");
                writln(outf,"\tLP\t4\t; LB");
                writln(outf,tr("\tLIA\tLB(%1-1)").arg(v.address));
            }
            else {
                writln(outf,"\tLIA\tHB("+v.varname+"-1)");
                writln(outf,"\tEXAM");
                writln(outf,"\tLP\t4\t; LB");
                writln(outf,"\tLIA\tLB("+v.varname+"-1)");
            }
            writln(outf,"\tEXAM");
            writln(outf,"\tPOP"); pushcnt--;
            writln(outf,"\tLIB\t0");
            writln(outf,"\tADB");
            writln(outf,"\tIXL");
            writln(outf,"\tEXAB");
            writln(outf,"\tIXL");
            writln(outf,"\tEXAB");
        }
    }
}

/*!
 \brief Load a Variable to the Primary Register

 \fn Clcc::LoadVariable
 \param name
*/
void Clcc::LoadVariable(QByteArray name) {

    writln("LOG",";LoadVariable:"+name);
    if (!FindVar(name)) Error("Variable not defined: "+name);

    Cvar v = varlist.at(VarFound);

    if (v.array) {
        LoadVariableArray(v);
    }
    else {
        if ((v.typ=="char") || (v.typ=="byte")) {
            if (v.local) {
                // Local char
                writln(outf,"\tLDR");
                writln(outf,tr("\tADIA\t%1").arg(v.address+2+pushcnt));
                writln(outf,"\tSTP");
                writln(outf,"\tLDM\t\t; Load variable "+v.varname);
            }
            else {
                if (v.xram) {
                    if (v.address !=-1) writln(outf,tr("\tLIDP\t%1").arg(v.address)+"\t; Load variable "+v.varname);
                    else writln(outf,"\tLIDP\t"+v.varname+"\t; Load variable "+v.varname);
                    writln(outf,"\tLDD");
                }
                else {
                    if (v.address < 64) writln(outf,tr("\tLP\t%1").arg(v.address)+"\t; Load variable "+v.varname);
                    else writln(outf,tr("\tLIP\t%1").arg(v.address)+"\t; Load variable "+v.varname);
                    writln(outf,"\tLDM");
                }
            }
            if (isword) writln(outf,"\tLIB\t0");
        }
        else {
            if (v.local) {
                // Local word
                writln(outf,"\tLDR");
                writln(outf,tr("\tADIA\t%1").arg(v.address+1+pushcnt));
                writln(outf,"\tSTP");
                writln(outf,"\tLDM\t; HB - Load variable "+v.varname);
                writln(outf,"\tEXAB");
                writln(outf,"\tINCP");
                writln(outf,"\tLDM\t; LB");
            }
            else {
                if (v.xram) {
                    if (v.address != -1)
                        writln(outf,tr("\tLIDP\t%1").arg(v.address+1)+"\t; Load 16bit variable "+v.varname);
                    else
                        writln(outf,"\tLIDP\t"+v.varname+"+1\t; Load 16bit variable "+v.varname);

                    writln(outf,"\tLDD\t\t; HB");
                    writln(outf,"\tEXAB");
                    if ((v.address != -1) && ((v.address + 1) / 256 == (v.address / 256))) {
                        writln(outf,tr("\tLIDL\tLB(%1)").arg(v.address));
                    }
                    else {
                        if (v.address !=-1) {
                            writln(outf,tr("\tLIDP\t%1").arg(v.address));
                        }
                        else {
                            writln(outf,"\tLIDP\t"+v.varname);
                        }
                    }
                    writln(outf,"\tLDD\t\t; LB");
                }
                else {
                    if (v.address < 64) writln(outf,tr("\tLP\t%1").arg(v.address+1)+"\t; Load 16bit variable "+v.varname);
                    else writln(outf,tr("\tLIP\t%1").arg(v.address+1)+"\t; Load 16bit variable "+v.varname);
                    writln(outf,"\tLDM\t\t; HB");
                    writln(outf,"\tEXAB");
                    writln(outf,"\tDECP\t\t; LB");
                    writln(outf,"\tLDM");
                }
            }
        }
    }
}

void ClccPC1500::LoadVariable(QByteArray name) {

    writln("LOG",";LoadVariable:"+name);
    if (!FindVar(name)) Error("Variable not defined: "+name);

    Cvar v = varlist.at(VarFound);

    if (v.array) {
        LoadVariableArray(v);
    }
    else {
        if ((v.typ=="char") || (v.typ=="byte")) {
            if (v.local) {
                // Local char
                writln(outf,"\tLDR");
                writln(outf,tr("\tADIA\t%1").arg(v.address+2+pushcnt));
                writln(outf,"\tSTP");
                writln(outf,"\tLDM\t\t; Load variable "+v.varname);
            }
            else {
                if (v.xram) {
                    if (v.address !=-1) writln(outf,tr("\tLIDP\t%1").arg(v.address)+"\t; Load variable "+v.varname);
                    else writln(outf,"\tLIDP\t"+v.varname+"\t; Load variable "+v.varname);
                    writln(outf,"\tLDD");
                }
                else {
                    if (v.address < 64) writln(outf,tr("\tLP\t%1").arg(v.address)+"\t; Load variable "+v.varname);
                    else writln(outf,tr("\tLIP\t%1").arg(v.address)+"\t; Load variable "+v.varname);
                    writln(outf,"\tLDM");
                }
            }
            if (isword) writln(outf,"\tLIB\t0");
        }
        else {
            if (v.local) {
                // Local word
                writln(outf,"\tLDR");
                writln(outf,tr("\tADIA\t%1").arg(v.address+1+pushcnt));
                writln(outf,"\tSTP");
                writln(outf,"\tLDM\t; HB - Load variable "+v.varname);
                writln(outf,"\tEXAB");
                writln(outf,"\tINCP");
                writln(outf,"\tLDM\t; LB");
            }
            else {
                if (v.xram) {
                    if (v.address != -1)
                        writln(outf,tr("\tLIDP\t%1").arg(v.address+1)+"\t; Load 16bit variable "+v.varname);
                    else
                        writln(outf,"\tLIDP\t"+v.varname+"+1\t; Load 16bit variable "+v.varname);

                    writln(outf,"\tLDD\t\t; HB");
                    writln(outf,"\tEXAB");
                    if ((v.address != -1) && ((v.address + 1) / 256 == (v.address / 256))) {
                        writln(outf,tr("\tLIDL\tLB(%1)").arg(v.address));
                    }
                    else {
                        if (v.address !=-1) {
                            writln(outf,tr("\tLIDP\t%1").arg(v.address));
                        }
                        else {
                            writln(outf,"\tLIDP\t"+v.varname);
                        }
                    }
                    writln(outf,"\tLDD\t\t; LB");
                }
                else {
                    if (v.address < 64) writln(outf,tr("\tLP\t%1").arg(v.address+1)+"\t; Load 16bit variable "+v.varname);
                    else writln(outf,tr("\tLIP\t%1").arg(v.address+1)+"\t; Load 16bit variable "+v.varname);
                    writln(outf,"\tLDM\t\t; HB");
                    writln(outf,"\tEXAB");
                    writln(outf,"\tDECP\t\t; LB");
                    writln(outf,"\tLDM");
                }
            }
        }
    }
}


void Clcc::StoreVariableArray(Cvar v) {

    if ((v.typ=="char") || (v.typ=="byte"))
    {
        if (!v.xram)
        {
            writln(outf,"\tLIB\t"+QByteArray::number(v.address)+"\t; Store array element from "+v.varname);
            writln(outf,"\tLP\t3");
            writln(outf,"\tADM");
            writln(outf,"\tEXAB");
            writln(outf,"\tSTP");
            writln(outf,"\tPOP"); pushcnt--;
            writln(outf,"\tEXAM");
        }
        else
        {
            writln(outf,"\tPUSH\t\t; Store array element from "+v.varname); pushcnt++;
            writln(outf,"\tLP\t7\t; HB of address");

            writln(outf,"\tLIA\tHB("+v.getLabel()+"-1)");
            writln(outf,"\tEXAM");
            writln(outf,"\tLP\t6\t; LB");
            writln(outf,"\tLIA\tLB("+v.getLabel()+"-1)");

            writln(outf,"\tEXAM");
            writln(outf,"\tPOP"); pushcnt--;
            writln(outf,"\tLIB\t0");
            writln(outf,"\tADB");
            writln(outf,"\tPOP"); pushcnt--;
            writln(outf,"\tIYS");
        }
    }
    else
    {
        if (!v.xram)
        {
            writln(outf,"\tRC");
            writln(outf,"\tSL");
            writln(outf,"\tLII\t"+QByteArray::number(v.address)+"\t; Store array element from "+v.varname);
            writln(outf,"\tLP\t0");
            writln(outf,"\tADM");
            writln(outf,"\tEXAM");
            writln(outf,"\tSTP");
            writln(outf,"\tINCP");
            writln(outf,"\tPOP"); pushcnt--;
            writln(outf,"\tEXAM");
            writln(outf,"\tDECP");
            writln(outf,"\tPOP"); pushcnt--;
            writln(outf,"\tEXAM");
        }
        else
        {
            writln(outf,"\tRC");
            writln(outf,"\tSL");
            writln(outf,"\tPUSH\t _t; Store array element from "+v.varname); pushcnt++;
            writln(outf,"\tLP\t7\t; HB of address");

            writln(outf,"\tLIA\tHB("+v.getLabel()+"-1)");
            writln(outf,"\tEXAM");
            writln(outf,"\tLP\t6\t; LB");
            writln(outf,"\tLIA\tLB("+v.getLabel()+"-1)");

            writln(outf,"\tEXAM");
            writln(outf,"\tPOP"); pushcnt--;
            writln(outf,"\tLIB\t0");
            writln(outf,"\tADB");
            writln(outf,"\tPOP"); pushcnt--;
            writln(outf,"\tEXAB");
            writln(outf,"\tPOP"); pushcnt--;
            writln(outf,"\tIYS");
            writln(outf,"\tEXAB");
            writln(outf,"\tIYS");
        }
    }
}

/*!
 \brief Store the Primary Register to a Variable

 \fn Clcc::StoreVariable
 \param name
*/

bool Clcc::StoreVariable(QByteArray name) {

    Cvar var;

    if (sourceinASM) writln(outf,"\t; StoreVariable : "+name);
    if (! FindVar(name)) {
        if (showErrors) QMessageBox::about(mainwindow,"ERROR","Variable not defined: "+name);
        return false;
    }
    var = varlist.at(VarFound);

    if (var.array) {
        StoreVariableArray(var);
    }
    else {
        if ( (var.typ=="char") || (var.typ=="byte") ) {
            if (var.xram) {
                writln(outf,"\tLIDP\t"+var.getLabel()+"\t; Store result in "+var.varname);
                writln(outf,"\tSTD");
            }
            else {
                if (var.local) {
                    // Local char
                    writln(outf,"\tEXAB");
                    writln(outf,"\tLDR");
                    writln(outf,"\tADIA\t"+QByteArray::number(var.address+2+pushcnt));
                    writln(outf,"\tSTP");
                    writln(outf,"\tEXAB");
                    writln(outf,"\tEXAM\t\t; Store result in "+var.varname);
                }
                else {
                    if (var.address <= 63) {
                        writln(outf,"\tLP\t"+QByteArray::number(var.address)+"\t; Store result in "+var.varname);
                    }
                    else {
                        writln(outf,"\tLIP\t"+QByteArray::number(var.address)+"\t; Store result in "+var.varname);
                    }
                    writln(outf,"\tEXAM");
                }
            }
        }
        else {
            if (var.local) {
                // Local word
                writln(outf,"\tPUSH"); pushcnt++;
                writln(outf,"\tLDR");
                writln(outf,"\tADIA\t"+QByteArray::number(var.address+2+pushcnt));
                writln(outf,"\tSTP");
                writln(outf,"\tPOP"); pushcnt--;
                writln(outf,"\tEXAM\t; LB - Store result in "+var.varname);
                writln(outf,"\tEXAB");
                writln(outf,"\tDECP");
                writln(outf,"\tEXAM\t; HB");
            }
            else {
                if (var.xram) {
                    writln(outf,"\tLIDP\t"+var.getLabel()+"\t; Store 16bit variable "+var.varname);
                    writln(outf,"\tSTD\t\t; LB");
                    writln(outf,"\tEXAB");
                    if ( (var.address>=0) &&
                         ( ((var.address + 1) / 256) == (var.address / 256) ) ) {
                        writln(outf,"\tLIDL\tLB("+QByteArray::number(var.address)+"+1)");
                    }
                    else  {
                        writln(outf,"\tLIDP\t"+ var.getLabel()+"+1");
                    }
                    writln(outf,"\tSTD\t\t; HB");
                }
                else {
                    // non local, non xram
                    if (var.address < 64) {
                        writln(outf,"\tLP\t"+QByteArray::number(var.address)+"\t; Store 16bit variable "+var.varname);
                    }
                    else {
                        writln(outf,"\tLIP\t"+QByteArray::number(var.address)+"\t; Store 16bit variable "+var.varname);
                    }
                    writln(outf,"\tEXAM\t\t; LB");
                    writln(outf,"\tEXAB");
                    writln(outf,"\tINCP\t\t; HB");
                    writln(outf,"\tEXAM");
                }
            }
        }
    }

}


bool ClccPC1500::StoreVariable(QByteArray name) {

    Cvar var;

    if (sourceinASM) writln(outf,"\t; StoreVariable : "+name);
    if (! FindVar(name)) {
        if (showErrors) QMessageBox::about(mainwindow,"ERROR","Variable not defined: "+name);
        return false;
    }
    var = varlist.at(VarFound);

    if (var.array) {
        StoreVariableArray(var);
    }
    else {
        if ( (var.typ=="char") || (var.typ=="byte") ) {
            if (var.xram) {
                writln(outf,"\tSTA\t("+var.getLabel()+")\t; Store result in "+var.varname);
            }
            else {
                if (var.local) {
                    // Local char
                    writln(outf,"\tEXAB");
                    writln(outf,"\tLDR");
                    writln(outf,"\tADIA\t"+QByteArray::number(var.address+2+pushcnt));
                    writln(outf,"\tSTP");
                    writln(outf,"\tEXAB");
                    writln(outf,"\tEXAM\t\t; Store result in "+var.varname);
                }
                else {
                    if (var.address <= 63) {
                        writln(outf,"\tLP\t"+QByteArray::number(var.address)+"\t; Store result in "+var.varname);
                    }
                    else {
                        writln(outf,"\tLIP\t"+QByteArray::number(var.address)+"\t; Store result in "+var.varname);
                    }
                    writln(outf,"\tEXAM");
                }
            }
        }
        else {
            if (var.local) {
                // Local word
                writln(outf,"\tPUSH"); pushcnt++;
                writln(outf,"\tLDR");
                writln(outf,"\tADIA\t"+QByteArray::number(var.address+2+pushcnt));
                writln(outf,"\tSTP");
                writln(outf,"\tPOP"); pushcnt--;
                writln(outf,"\tEXAM\t; LB - Store result in "+var.varname);
                writln(outf,"\tEXAB");
                writln(outf,"\tDECP");
                writln(outf,"\tEXAM\t; HB");
            }
            else {
                if (var.xram) {
                    writln(outf,"\tLIDP\t"+var.getLabel()+"\t; Store 16bit variable "+var.varname);
                    writln(outf,"\tSTD\t\t; LB");
                    writln(outf,"\tEXAB");
                    if ( (var.address>=0) &&
                         ( ((var.address + 1) / 256) == (var.address / 256) ) ) {
                        writln(outf,"\tLIDL\tLB("+QByteArray::number(var.address)+"+1)");
                    }
                    else  {
                        writln(outf,"\tLIDP\t"+ var.getLabel()+"+1");
                    }
                    writln(outf,"\tSTD\t\t; HB");
                }
                else {
                    // non local, non xram
                    if (var.address < 64) {
                        writln(outf,"\tLP\t"+QByteArray::number(var.address)+"\t; Store 16bit variable "+var.varname);
                    }
                    else {
                        writln(outf,"\tLIP\t"+QByteArray::number(var.address)+"\t; Store 16bit variable "+var.varname);
                    }
                    writln(outf,"\tEXAM\t\t; LB");
                    writln(outf,"\tEXAB");
                    writln(outf,"\tINCP\t\t; HB");
                    writln(outf,"\tEXAM");
                }
            }
        }
    }

}



//{ Generates init code for a var in xram }

void Clcc::varxram(int value, int adr, int size, QByteArray nm) {
    writln(outf,"\tLIDP\t"+QString("%1").arg(adr)+"\t; Variable "+nm+" = "+QString("%1").arg(value));
    if (size == 1) {
        if (value == 0) writln(outf,"\tRA");
        else  writln(outf,tr("\tLIA\t%1").arg(value));
        writln(outf,"\tSTD");
    }
    else {
        if (value % 256 == 0)  writln(outf, "\tRA");
        else writln(outf, tr("\tLIA\tLB(%1)").arg(value));

        writln(outf, "\tSTD");
        if ( (adr/256) == ((adr+1)/256) ) writln(outf,tr("\tLIDL\tLB(%1)").arg(adr+1));
        else writln(outf,tr("\tLIDP\t%1").arg(adr+1));

        if ((value %256) != (value / 256)) {
            if ((value / 256) == 0) writln(outf, "\tRA");
            else writln(outf,tr("\tLIA\tHB(%1)").arg(value));
        }
        writln(outf, "\tSTD");

    }
    writln(outf,"");
}

void ClccPC1500::varxram(int value, int adr, int size, QByteArray nm) {
//    writln(outf,"\tLIDP\t"+QString("%1").arg(adr)+"\t; Variable "+nm+" = "+QString("%1").arg(value));
    if (size == 1) {
        if (value == 0) writln(outf,"\tCLA");
        else  writln(outf,tr("\tLDA\t%1").arg(value));
        writln(outf,"\tSTA ("+QString("%1").arg(adr)+")\t; Variable "+nm+" = "+QString("%1").arg(value));
    }
    else {
        if (value % 256 == 0)  writln(outf, "\tCLA");
        else writln(outf, tr("\tLIA\tLB(%1)").arg(value));

        writln(outf, "\tSTD");
        if ( (adr/256) == ((adr+1)/256) ) writln(outf,tr("\tLIDL\tLB(%1)").arg(adr+1));
        else writln(outf,tr("\tLIDP\t%1").arg(adr+1));

        if ((value %256) != (value / 256)) {
            if ((value / 256) == 0) writln(outf, "\tRA");
            else writln(outf,tr("\tLIA\tHB(%1)").arg(value));
        }
        writln(outf, "\tSTD");

    }
    writln(outf,"");
}
//{--------------------------------------------------------------}


//{--------------------------------------------------------------}
//{ Generates init code for an array in xram }


void Clcc::varxarr(QByteArray value,int adr, int size,QByteArray nm, QByteArray typ) {

    int v,c;
    QByteArray s;

    if (size == 0) return;

    s = "";
    for (int i = 0 ; i< size;i++) {
        if (i < value.length()) {
            if (typ !="word") s.append(tr("%1").arg((unsigned char)value.at(i)));
            else s.append(tr("%1").arg(256*value[i*2]+value[i*2+1]));
        }
        else s.append("0");

        if (i < (size-1))s.append(", ");
    }

    if ((typ != "word") && (size <= 5)) {
        // Set up address and write 1st byte
        writln(outf, "\tLIDP\t"+tr("%1").arg(adr)+"\t; Variable "+nm+" = ("+s+")");
        v = value[0];
        if (v == 0) writln(outf,"\tRA");
        else writln(outf,tr("\tLIA\t%1").arg(v));
        writln(outf,"\tSTD");

        c = v;
        if (size > 1) {
            for (int i = 1 ;i< size;i++) {
                if (((adr+i-1) / 256) == ((adr+i) / 256)) writln(outf,tr("\tLIDL\tLB(%1)").arg(adr+i));
                else writln(outf,tr("\tLIDP\t%1").arg(adr+i));

                if (i < value.size()) v = value[i];
                else v = 0;

                if (v != c) {
                    if (v == 0) writln(outf,"\tRA");
                    else writln(outf,tr("\tLIA\t%1").arg(v));
                    writln(outf,"\tSTD");
                }
                else writln(outf,"\tSTD");
                c = v;
            }
        }
    }
    else if (typ != "word") {
        writln(outf,"\t; Variable "+nm+" = ("+s+")");
        load_x(nm+"-1");
        load_y(QByteArray::number(adr-1));
        writln(outf, tr("\tLII\t%1\t; Load I as counter").arg(size));
        writln(outf, "\tIXL");
        writln(outf, "\tIYS");
        writln(outf, "\tDECI");
        writln(outf, "\tJRNZM\t4");

        addasm(nm+":\t; Variable init data "+nm);
        s="\t.DB\t" + s;
        addasm(s);
        addasm("");
    }
    else {
        writln(outf, "\t; Variable "+nm+" = ("+s+")");
        load_x(nm+"-1");
        load_y(QByteArray::number(adr-1));
        writln(outf, tr("\tLII\t%1\t; Load I as counter").arg(size*2));
        writln(outf, "\tIXL");
        writln(outf, "\tIYS");
        writln(outf, "\tDECI");
        writln(outf, "\tJRNZM\t4");

        addasm(nm+":\t; Variable init data "+nm);
        s="\t.DW\t" + s;
        addasm(s);
        addasm("");
    }
    writln(outf,"");
}
//{--------------------------------------------------------------}


//{--------------------------------------------------------------}
//{ Generates init code for an array variable in a register }

//void Clcc::varrarr(QList<unsigned char> value,int adr, int size,QByteArray nm, QByteArray typ) {
void Clcc::varrarr(QByteArray value,int adr, int size,QByteArray nm, QByteArray typ) {
    QByteArray s;

    if (size == 0) return;
    s="";
    // TODO : convert \n \t \r to corresponding char value
    value.replace("\\n","\n").replace("\\r","\r");
    for (int i=0 ; i< size; i++) {
        if (i <= value.size()) {
            if (typ !="word") s.append(tr("%1").arg((unsigned char)value.at(i)));
            else s.append(tr("%1").arg(256*value[i*2]+value[i*2+1]));
        }
        else s.append("0");

        if (i < (size-1)) s.append(", ");
    }

    if (typ !="word") writln(outf,tr("\tLII\t%1").arg(size-1)+"\t; Variable "+nm+" = ("+s+")");
    else writln(outf,tr("\tLII\t%1").arg(size*2-1)+"\t; Variable "+nm+" = ("+s+")");
    writln(outf,"\tLIDP\t"+nm);
    if (adr <= 63) writln(outf,tr("\tLP\t%1").arg(adr));
    else writln(outf,tr("\tLIP\t%1").arg(adr));
    writln(outf,"\tMVWD");

    addasm(nm+":\t; Variable init data "+nm);
    if (typ != "word") s="\t.DB\t" + s;
    else s="\t.DW\t" + s;
    addasm(s);
    addasm("");

    writln(outf,"");
}
//{--------------------------------------------------------------}



void Clcc::load_x(QByteArray s) {
        writln(outf, "\tLP\t4\t; Load XL");
        writln(outf, "\tLIA\tLB("+s+")");
        writln(outf, "\tEXAM");
        writln(outf, "\tLP\t5\t; Load XH");
        writln(outf, "\tLIA\tHB("+s+")");
        writln(outf, "\tEXAM");
    }


void Clcc::load_y(QByteArray s) {
        writln(outf, "\tLP\t6\t; Load YL");
        writln(outf, "\tLIA\tLB("+s+")");
        writln(outf, "\tEXAM");
        writln(outf, "\tLP\t7\t; Load YH");
        writln(outf, "\tLIA\tHB("+s+")");
        writln(outf, "\tEXAM");
    }

//{--------------------------------------------------------------}
//{ Generates init code for a var in code space }

void Clcc::varcode(int value, int adr, int size,QByteArray nm) {
    if (value == -1) value = 0;
    addasm(nm+":\t; Variable "+nm+" = "+QByteArray::number(value));
    if (size == 1) addasm("\t.DB\t"+QByteArray::number(value));
    else addasm("\t.DW\t"+QByteArray::number(value));
    addasm("");
}
//{--------------------------------------------------------------}

//{--------------------------------------------------------------}
//{ Generates init code for an array in code space }
//void Clcc::varcarr(QList<unsigned char> value, int adr, int size,QByteArray nm, QByteArray typ) {
void Clcc::varcarr(QByteArray value, int adr, int size,QByteArray nm, QByteArray typ) {
    QByteArray s;
    if (size == 0) return;

    s ="";
    for (int i = 0; i< size;i++) {
        if (i < value.size()) {
            if (typ !="word") s.append(tr("%1").arg((unsigned char)value.at(i)));
            else s.append(tr("%1").arg(256*value[i*2]+value[i*2+1]));
        }
        else s.append("0");

        if (i < (size-1)) s.append(", ");
    }

    addasm(nm+":\t; Variable "+nm+" = ("+s+")");
    if (typ !="word") s = "\t.DB\t" + s;
    else s = "\t.DW\t" + s;
    addasm(s);
    addasm("");
}
//{--------------------------------------------------------------}


//{--------------------------------------------------------------}
//{ Generates init code for a variable in a register }

void Clcc::varreg(int value, int adr, int size, QByteArray nm) {
    //        { Check for named register }
    if (((adr == 0) || (adr == 1)) && (size == 1)) {
        if (adr == 0) writln(outf,tr("\tLII\t%1").arg(value)+"\t; I is "+nm+tr(" = %1").arg(value));
        else writln(outf,tr("\tLIJ\t%1").arg(value)+"\t; J is "+nm+tr(" = %1").arg(value));
        writln(outf,"");
        return;
    }

    if (adr <= 63) writln(outf,tr("\tLP\t%1").arg(adr)+"\t; Variable "+nm+tr(" = %1").arg(value));
    else {
        writln(outf,tr("\tLIP\t%1").arg(adr)+"\t; Variable "+nm+tr(" = %1").arg(value));
    }
    if (size == 1) {
        writln(outf,tr("\tLIA\t%1").arg(value));
        writln(outf,"\tEXAM");
    }
    else {
        if ((value % 256) == 0) writln(outf,"\tRA\t; LB");
        else writln(outf,tr("\tLIA\tLB(%1)\t;LB").arg(value));
        writln(outf,"\tEXAM");

        writln(outf,"\tINCP");
        if ((value % 256) != (value / 256)) {
            if ((value / 256) == 0) writln(outf,"\tRA\t; HB");
            else writln(outf,tr("\tLIA\tHB(%1)\t; HB").arg(value));
        }
        writln(outf,"\tEXAM");
    }
    writln(outf,"");
}

//{---------------------------------------------------------------}
//{ Comparison Equal }

void Clcc::CompEqual(void) {
    if (isword) {
        writln(outf,"\tLP\t0");
        writln(outf,"\tEXAM");
        writln(outf,"\tEXAB");
        writln(outf,"\tLP\t1");
        writln(outf,"\tEXAM");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tCALL\tLIB_CPE16\t; Compare for equal");
        addlib(CPE16);
    }
    else {
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tLP\t3");
        writln(outf,"\tCPMA\t\t; Compare for equal");
        writln(outf,"\tRA");
        writln(outf,"\tJRNZP\t2");
        writln(outf,"\tDECA");
    }
    writln(outf,"");
}


//{---------------------------------------------------------------}
//{ Comparison Not Equal }

void Clcc::CompNotEqual(void) {
    if (isword) {
        writln(outf,"\tLP\t0");
        writln(outf,"\tEXAM");
        writln(outf,"\tEXAB");
        writln(outf,"\tLP\t1");
        writln(outf,"\tEXAM");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tCALL\tLIB_CPNE16\t; Compare not equal");
        addlib(CPNE16);
    }
    else {
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tLP\t3");
          writln(outf,"\tCPMA\t\t; Compare for not equal");
          writln(outf,"\tRA");
          writln(outf,"\tJRZP\t2");
        writln(outf,"\tDECA");
    }
    writln(outf,"");
}


//{---------------------------------------------------------------}
//{ Comparison Greater or Equal }

void Clcc::CompGrOrEq(void) {
    if (isword) {
        writln(outf,"\tLP\t0");
        writln(outf,"\tEXAM");
        writln(outf,"\tEXAB");
        writln(outf,"\tLP\t1");
        writln(outf,"\tEXAM");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tCALL\tLIB_CPGE16\t; Compare for Greater or Equal");
        addlib(CPGE16);
    }
    else {
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tEXAB");
        writln(outf,"\tLP\t3");
        writln(outf,"\tCPMA\t\t; Compare for Greater or Equal");
        writln(outf,"\tRA");
        writln(outf,"\tJRCP\t2");
        writln(outf,"\tDECA");
    }
    writln(outf,"");
}


//{---------------------------------------------------------------}
//{ Comparison smaller or equal }

void Clcc::CompSmOrEq(void) {
    if (isword) {
        writln(outf,"\tLP\t0");
        writln(outf,"\tEXAM");
        writln(outf,"\tEXAB");
        writln(outf,"\tLP\t1");
        writln(outf,"\tEXAM");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tCALL\tLIB_CPSE16\t; Compare for smaller or equal");
        addlib(CPSE16);
    }
    else {
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tLP\t3");
        writln(outf,"\tCPMA\t\t; Compare for smaller or equal");
        writln(outf,"\tRA");
        writln(outf,"\tJRCP\t2");
        writln(outf,"\tDECA");
    }
    writln(outf,"");
}


//{---------------------------------------------------------------}
//{ Comparison Greater }

void Clcc::CompGreater(void) {
    if (isword) {
        writln(outf,"\tLP\t0");
        writln(outf,"\tEXAM");
        writln(outf,"\tEXAB");
        writln(outf,"\tLP\t1");
        writln(outf,"\tEXAM");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tCALL\tLIB_CPG16\t; Compare for greater");
        addlib(CPG16);
    }
    else {
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tLP\t3");
          writln(outf,"\tCPMA\t\t; Compare for greater");
          writln(outf,"\tRA");
          writln(outf,"\tJRNCP\t2");
        writln(outf,"\tDECA");
    }
    writln(outf,"");
}


//{---------------------------------------------------------------}
//{ Comparison smaller }

void Clcc::CompSmaller(void) {
    if (isword) {
        writln(outf,"\tLP\t0");
        writln(outf,"\tEXAM");
        writln(outf,"\tEXAB");
        writln(outf,"\tLP\t1");
        writln(outf,"\tEXAM");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tCALL\tLIB_CPS16\t; Compare for smaller");
        addlib(CPS16);
    }
    else {
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tEXAB");
        writln(outf,"\tLP\t3");
        writln(outf,"\tCPMA\t\t; Compare for smaller");
        writln(outf,"\tRA");
        writln(outf,"\tJRNCP\t2");
        writln(outf,"\tDECA");
    }
    writln(outf,"");
}


//{---------------------------------------------------------------}
//{ Branch Unconditional  }

void Clcc::Branch(QByteArray L){
    writln(outf,"\tRJMP\t"+L);
}

//{---------------------------------------------------------------}
//{ Branch False }

void Clcc::BranchFalse(QByteArray L) {
    writln(outf,"\tTSIA\t255\t; Branch if false");
//    writln(outf,"\tJRZP\t"+L);
    writln(outf,"\tJPZ\t"+L);
    writln(outf,"");
}

//{--------------------------------------------------------------}
//{ Bitwise Not Primary }

void Clcc::NotIt(void) {
    if (isword) {
        writln(outf,"\tLP\t1");
        writln(outf,"\tORIM\t255");
        writln(outf,"\tLP\t0");
        writln(outf,"\tORIM\t255");
        writln(outf,"\tSBB\t\t; Negate");
        writln(outf,"\tLP\t1");
        writln(outf,"\tLDM");
        writln(outf,"\tEXAB");
        writln(outf,"\tLP\t0");
        writln(outf,"\tLDM");
    }
    else {
        writln(outf,"\tLIB\t0");
        writln(outf,"\tLP\t3");
        writln(outf,"\tSBM\t\t; Negate");
        writln(outf,"\tEXAB");
    }
}


//{--------------------------------------------------------------}
//{ Negate Primary }

void Clcc::Negate(void) {
    if (isword) {
        writln(outf,"\tLP\t1");
        writln(outf,"\tORIM\t255");
        writln(outf,"\tLP\t0");
        writln(outf,"\tORIM\t255");
        writln(outf,"\tSBB\t\t; Negate");
        writln(outf,"\tLP\t1");
        writln(outf,"\tLDM");
        writln(outf,"\tEXAB");
        writln(outf,"\tLP\t0");
        writln(outf,"\tLDM");
    }
    else {
        writln(outf,"\tLIB\t0");
        writln(outf,"\tLP\t3");
        writln(outf,"\tSBM\t\t; Negate");
        writln(outf,"\tEXAB");
    }
}

//{--------------------------------------------------------------}
//{ Or TOS with Primary }

void Clcc::PopOr(void) {
    if (isword) {
        writln(outf,"\tLP\t0");
        writln(outf,"\tEXAM");
        writln(outf,"\tEXAB");
        writln(outf,"\tLP\t1");
        writln(outf,"\tEXAM");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tORMA\t\t; OR HB");
        writln(outf,"\tLP\t0");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tORMA\t\t; OR LB");
        writln(outf,"\tEXAM");
        writln(outf,"\tEXAB");
        writln(outf,"\tLP\t1");
        writln(outf,"\tEXAM");
        writln(outf,"\tEXAB");
    }
    else {
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tLP\t3");
        writln(outf,"\tORMA\t\t; OR");
        writln(outf,"\tEXAB");
    }
}


//{--------------------------------------------------------------}
//{ Exclusive-Or TOS with Primary }

void Clcc::PopXor(void) {
    if (isword) {
        writln(outf,"\tLP\t0");
        writln(outf,"\tEXAM");
        writln(outf,"\tEXAB");
        writln(outf,"\tLP\t1");
        writln(outf,"\tEXAM");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tCALL\tLIB_XOR16\t; XOR");
        addlib(XOR16);
        addlib(XOR8);
    }
    else {
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tCALL\tLIB_XOR8\t; XOR");
        addlib(XOR8);
    }
}

//{--------------------------------------------------------------}
//{ Shift left TOS with Primary }

void Clcc::PopSl(void) {
    if (isword) {
        writln(outf,"\tLP\t0");
        writln(outf,"\tEXAM");
        writln(outf,"\tEXAB");
        writln(outf,"\tLP\t1");
        writln(outf,"\tEXAM");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tCALL\tLIB_SL16\t; SL");
        addlib(SL16);
    }
    else {
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tCALL\tLIB_SL8\t; Shift left");
        addlib(SL8);
    }
}


//{--------------------------------------------------------------}
//{ Shift right TOS with Primary }

void Clcc::PopSr(void) {
    if (isword) {
        writln(outf,"\tLP\t0");
        writln(outf,"\tEXAM");
        writln(outf,"\tEXAB");
        writln(outf,"\tLP\t1");
        writln(outf,"\tEXAM");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tCALL\tLIB_SR16\t; SR");
        addlib(SR16);
    }
    else {
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tCALL\tLIB_SR8\t; Shift right");
        addlib(SR8);
    }
}

//{--------------------------------------------------------------}
//{ Modulo TOS with Primary }
void Clcc::PopMod(void) {
    if (isword) {
        writln(outf,"\tLP\t1");
        writln(outf,"\tEXAM");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tEXAM");
        writln(outf,"\tLP\t0");
        writln(outf,"\tEXAM");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tEXAM");
        writln(outf,"\tCALL\tLIB_DIVMOD16\t; Modulo");
        addlib(DIVMOD16);
    }
    else {
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tCALL\tLIB_DIV8\t; Modulo");
        writln(outf,"\tEXAB");
        addlib(DIVMOD8);
    }
}
//{--------------------------------------------------------------}


//{--------------------------------------------------------------}
//{ And Primary with TOS }

void Clcc::PopAnd(void) {
    if (isword) {
        writln(outf,"\tLP\t0");
        writln(outf,"\tEXAM");
        writln(outf,"\tEXAB");
        writln(outf,"\tLP\t1");
        writln(outf,"\tEXAM");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tANMA\t\t; AND HB");
        writln(outf,"\tLP\t0");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tANMA\t\t; AND LB");
        writln(outf,"\tEXAM");
        writln(outf,"\tEXAB");
        writln(outf,"\tLP\t1");
        writln(outf,"\tEXAM");
        writln(outf,"\tEXAB");
    }
    else {
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tLP\t3");
        writln(outf,"\tANMA\t\t; AND");
        writln(outf,"\tEXAB");
    }
}
//{--------------------------------------------------------------}


///{--------------------------------------------------------------}
//{ Push Primary to Stack }

void Clcc::Push(void) {
    if (isword) {
        writln(outf,"\tPUSH\t\t; Push A then B"); pushcnt++;
        writln(outf,"\tEXAB");
        writln(outf,"\tPUSH"); pushcnt++;
    }
    else {
        writln(outf,"\tPUSH"); pushcnt++;
    }
}


//{--------------------------------------------------------------}
//{ Add TOS to Primary }

void Clcc::PopAdd(void) {
    if (isword) {
        writln(outf,"\tLP\t0");
        writln(outf,"\tEXAM");
        writln(outf,"\tEXAB");
        writln(outf,"\tLP\t1");
        writln(outf,"\tEXAM");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
//        writln(outf,"\tEXAB");
        writln(outf,"\tLP\t0");
        writln(outf,"\tADB\t\t; Addition");
        writln(outf,"\tLP\t1");
        writln(outf,"\tLDM");
        writln(outf,"\tEXAB");
        writln(outf,"\tLP\t0");
        writln(outf,"\tLDM");
    }
    else {
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tLP\t3");
        writln(outf,"\tADM\t\t; Addition");
        writln(outf,"\tEXAB");
    }
}


//{--------------------------------------------------------------}
//{ Subtract TOS from Primary }

void Clcc::PopSub(void) {
    if (isword) {
        writln(outf,"\tLP\t1");
        writln(outf,"\tEXAM");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tEXAM");
        writln(outf,"\tLP\t0");
        writln(outf,"\tEXAM");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tEXAM");
        writln(outf,"\tSBB\t\t; Subtraction");
        writln(outf,"\tLP\t1");
        writln(outf,"\tLDM");
        writln(outf,"\tEXAB");
        writln(outf,"\tLP\t0");
        writln(outf,"\tLDM");
    }
    else {
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tLP\t3");
        writln(outf,"\tEXAB");
        writln(outf,"\tSBM\t\t; Subtraction");
        writln(outf,"\tEXAB");
    }
}

//{--------------------------------------------------------------}


//{--------------------------------------------------------------}
//{ Multiply TOS by Primary }

void Clcc::PopMul(void) {
    if (isword) {
        writln(outf,"\tLP\t0");
        writln(outf,"\tEXAM");
        writln(outf,"\tEXAB");
        writln(outf,"\tLP\t1");
        writln(outf,"\tEXAM");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tCALL\tLIB_MUL16\t; Multiplication");
        addlib(MUL16);
    }
    else {
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tCALL\tLIB_MUL8\t; Multiplication");
        addlib(MUL8);
    }
}

//{--------------------------------------------------------------}
//{ Divide Primary by TOS }

void Clcc::PopDiv(void) {
    if (isword) {
        writln(outf,"\tLP\t1");
        writln(outf,"\tEXAM");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tEXAM");
        writln(outf,"\tLP\t0");
        writln(outf,"\tEXAM");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tEXAM");
        writln(outf,"\tCALL\tLIB_DIVMOD16\t; Division");
        writln(outf,"\tLP\t1");
        writln(outf,"\tLDM");
        writln(outf,"\tEXAB");
        writln(outf,"\tLP\t0");
        writln(outf,"\tLDM");
        addlib(DIVMOD16);
    }
    else {
        writln(outf,"\tEXAB");
        writln(outf,"\tPOP"); pushcnt--;
        writln(outf,"\tCALL\tLIB_DIV8\t; Division");
        addlib(DIVMOD8);
    }
}
//{--------------------------------------------------------------}


//{--------------------------------------------------------------}
//{ Load the Primary Register with a Constant }

void Clcc::LoadConstant(QByteArray n) {
    if (isword) {
        writln(outf,"\tLIA\tLB("+n+")\t; Load constant LB "+n);
        writln(outf,"\tLIB\tHB("+n+")\t; Load constant HB "+n);
    }
    else {

        if (n.toInt() == 0) writln(outf, "\tRA\t\t; Load 0");
        else writln(outf,"\tLIA\t"+n+"\t; Load constant "+n);
    }
}



#if 0
void Clcc::LoadVariable(QByteArray name) {
    QByteArray typ;
    bool xram,arr,local,pnt;
    int adr;

    writln("LOG",";LoadVariable:"+name);
    if (!FindVar(name)) Error("Variable not defined: "+name);
    typ = varlist[VarFound].typ;
    adr = varlist[VarFound].address;
    local = varlist[VarFound].local;
    arr = varlist[VarFound].array;
    xram = varlist[VarFound].xram;
    pnt = varlist.at(VarFound).pointer;
    Cvar v = varlist.at(VarFound);

    if (! arr) {
        if ((typ=="char") || (typ=="byte")) {
            if (!xram) {
                if (!local) {
                    if (adr < 64) writln(outf,tr("\tLP\t%1").arg(adr)+"\t; Load variable "+name);
                    else writln(outf,tr("\tLIP\t%1").arg(adr)+"\t; Load variable "+name);
                    writln(outf,"\tLDM");
                }
                else {// Local char
                    writln(outf,"\tLDR");
                    writln(outf,tr("\tADIA\t%1").arg(adr+2+pushcnt));
                    writln(outf,"\tSTP");
                    writln(outf,"\tLDM\t\t; Load variable "+name);
                }
            }
            else {
                if (adr !=-1) writln(outf,tr("\tLIDP\t%1").arg(adr)+"\t; Load variable "+name);
                else writln(outf,"\tLIDP\t"+name+"\t; Load variable "+name);
                writln(outf,"\tLDD");
            }
            if (isword) writln(outf,"\tLIB\t0");
        }
        else {
            if (!xram) {
                if (!local) {
                    if (adr < 64) writln(outf,tr("\tLP\t%1").arg(adr+1)+"\t; Load 16bit variable "+name);
                    else writln(outf,tr("\tLIP\t%1").arg(adr+1)+"\t; Load 16bit variable "+name);
                    writln(outf,"\tLDM\t\t; HB");
                    writln(outf,"\tEXAB");
                    writln(outf,"\tDECP\t\t; LB");
                    writln(outf,"\tLDM");
                }
                else {// Local word
                    writln(outf,"\tLDR");
                    writln(outf,tr("\tADIA\t%1").arg(adr+1+pushcnt));
                    writln(outf,"\tSTP");
                    writln(outf,"\tLDM\t; HB - Load variable "+name);
                    writln(outf,"\tEXAB");
                    writln(outf,"\tINCP");
                    writln(outf,"\tLDM\t; LB");
                }
            }
            else {
                if (adr != -1) writln(outf,tr("\tLIDP\t%1").arg(adr+1)+"\t; Load 16bit variable "+name);
                else writln(outf,"\tLIDP\t"+name+"+1\t; Load 16bit variable "+name);
                writln(outf,"\tLDD\t\t; HB");
                writln(outf,"\tEXAB");
                if ((adr != -1) && ((adr + 1) / 256 == (adr / 256)))
                    writln(outf,tr("\tLIDL\tLB(%1)").arg(adr));
                else if (adr !=-1) writln(outf,tr("\tLIDP\t%1").arg(adr));
                else writln(outf,"\tLIDP\t"+name);
                writln(outf,"\tLDD\t\t; LB");
            }
        }
    }
    else {

        if ((typ=="char") || (typ=="byte")) {
            if (!xram) {
                writln(outf,tr("\tLIB\t%1").arg(adr)+"\t; Load array element from "+name);
                writln(outf,"\tLP\t3");
                writln(outf,"\tADM");
                writln(outf,"\tEXAB");
                writln(outf,"\tSTP");
                writln(outf,"\tLDM");
            }
            else {
                writln(outf,"\tPUSH\t\t; Load array element from "+name); pushcnt++;
                writln(outf,"\tLP\t5\t; HB of address");
                if (adr !=-1) {
                    writln(outf,tr("\tLIA\tHB(%1-1)").arg(adr));
                    writln(outf,"\tEXAM");
                    writln(outf,"\tLP4\t; LB");
                    writln(outf,tr("\tLIA\tLB(%1-1)").arg(adr));
                }
                else {
                    writln(outf,"\tLIA\tHB("+name+"-1)");
                    writln(outf,"\tEXAM");
                    writln(outf,"\tLP\t4\t; LB");
                    writln(outf,"\tLIA\tLB("+name+"-1)");
                }
                writln(outf,"\tEXAM");
                writln(outf,"\tPOP"); pushcnt--;
                writln(outf,"\tLIB\t0");
                writln(outf,"\tADB");
                writln(outf,"\tIXL");
            }
        }
        else {
            if (!xram) {
                writln(outf,"\tRC");
                writln(outf,"\tSL");
                writln(outf,tr("\tLII\t%1").arg(adr)+"\t; Store array element from "+name);
                writln(outf,"\tLP\t0");
                writln(outf,"\tADM");
                writln(outf,"\tEXAM");
                writln(outf,"\tSTP");
                writln(outf,"\tLDM");
                writln(outf,"\tEXAB");
                writln(outf,"\tINCP");
                writln(outf,"\tLDM");
                writln(outf,"\tEXAB");
            }
            else {
                writln(outf,"\tRC");
                writln(outf,"\tSL");
                writln(outf,"\tPUSH\t\t; Load array element from "+name); pushcnt++;
                writln(outf,"\tLP\t5\t; HB of address");
                if (adr !=-1) {
                    writln(outf,tr("\tLIA\tHB(%1-1)").arg(adr));
                    writln(outf,"\tEXAM");
                    writln(outf,"\tLP\t4\t; LB");
                    writln(outf,tr("\tLIA\tLB(%1-1)").arg(adr));
                }
                else {
                    writln(outf,"\tLIA\tHB("+name+"-1)");
                    writln(outf,"\tEXAM");
                    writln(outf,"\tLP\t4\t; LB");
                    writln(outf,"\tLIA\tLB("+name+"-1)");
                }
                writln(outf,"\tEXAM");
                writln(outf,"\tPOP"); pushcnt--;
                writln(outf,"\tLIB\t0");
                writln(outf,"\tADB");
                writln(outf,"\tIXL");
                writln(outf,"\tEXAB");
                writln(outf,"\tIXL");
                writln(outf,"\tEXAB");
            }
        }
    }
}


#endif
#if 0
void Clcc::StoreVariable(QByteArray name) {

    Cvar var;

    if (! FindVar(name)) { if (showErrors) QMessageBox::about(mainwindow,"ERROR","Variable not defined: "+name); }
    var = varlist.at(VarFound);

    if (!var.array) {
        if ( (var.typ=="char") || (var.typ=="byte") ) {
            if (! var.xram) {
                if (!var.local) {
                    if (var.address <= 63) {
                        writln(outf,"\tLP\t"+QByteArray::number(var.address)+"\t; Store result in "+var.varname);
                    }
                    else {
                        writln(outf,"\tLIP\t"+QByteArray::number(var.address)+"\t; Store result in "+var.varname);
                    }
                    writln(outf,"\tEXAM");
                }
                else {
                    // Local char
                    writln(outf,"\tEXAB");
                    writln(outf,"\tLDR");
                    writln(outf,"\tADIA\t"+QByteArray::number(var.address+2+pushcnt));
                    writln(outf,"\tSTP");
                    writln(outf,"\tEXAB");
                    writln(outf,"\tEXAM\t\t; Store result in "+var.varname);
                }
            }
            else {
                writln(outf,"\tLIDP\t"+var.getLabel()+"\t; Store result in "+var.varname);
                writln(outf,"\tSTD");
            }
        }
        else {
            if (!var.xram) {
                if (!var.local) {
                    if (var.address < 64) {
                        writln(outf,"\tLP\t"+QByteArray::number(var.address)+"\t; Store 16bit variable "+var.varname);
                    }
                    else {
                        writln(outf,"\tLIP\t"+QByteArray::number(var.address)+"\t; Store 16bit variable "+var.varname);
                    }
                    writln(outf,"\tEXAM\t\t; LB");
                    writln(outf,"\tEXAB");
                    writln(outf,"\tINCP\t\t; HB");
                    writln(outf,"\tEXAM");
                }
                else {
                // Local word
                    writln(outf,"\tPUSH"); pushcnt++;
                    writln(outf,"\tLDR");
                    writln(outf,"\tADIA\t"+QByteArray::number(var.address+2+pushcnt));
                    writln(outf,"\tSTP");
                    writln(outf,"\tPOP"); pushcnt--;
                    writln(outf,"\tEXAM\t; LB - Store result in "+var.varname);
                    writln(outf,"\tEXAB");
                    //FIXME: ???? INCP or DECP
                    writln(outf,"\tDECP");
                    writln(outf,"\tEXAM\t; HB");
                    //writln(outf,"\tEXAB");
                }
            }
            else {
                writln(outf,"\tLIDP\t"+var.getLabel()+"\t; Store 16bit variable "+var.varname);
                writln(outf,"\tSTD\t\t; LB");
                writln(outf,"\tEXAB");
                if ( var.address &&
                     ( ((var.address + 1) / 256) == (var.address / 256) ) ) {
                     writln(outf,"\tLIDL\tLB("+QByteArray::number(var.address)+"+1)");
                }
                else  {
                    writln(outf,"\tLIDP\t"+ var.getLabel()+"+1");
                }
                writln(outf,"\tSTD\t\t; HB");
            }
        }
    }
    else {
        if ((var.typ=="char") || (var.typ=="byte"))
        {
            if (!var.xram)
            {
                writln(outf,"\tLIB\t"+QByteArray::number(var.address)+"\t; Store array element from "+var.varname);
                writln(outf,"\tLP\t3");
                writln(outf,"\tADM");
                writln(outf,"\tEXAB");
                writln(outf,"\tSTP");
                writln(outf,"\tPOP"); pushcnt--;
                writln(outf,"\tEXAM");
            }
            else
            {
                writln(outf,"\tPUSH\t\t; Store array element from "+var.varname); pushcnt++;
                writln(outf,"\tLP\t7\t; HB of address");

                writln(outf,"\tLIA\tHB("+var.getLabel()+"-1)");
                writln(outf,"\tEXAM");
                writln(outf,"\tLP\t6\t; LB");
                writln(outf,"\tLIA\tLB("+var.getLabel()+"-1)");

                writln(outf,"\tEXAM");
                writln(outf,"\tPOP"); pushcnt--;
                writln(outf,"\tLIB\t0");
                writln(outf,"\tADB");
                writln(outf,"\tPOP"); pushcnt--;
                writln(outf,"\tIYS");
            }
        }
        else
        {
            if (!var.xram)
            {
                writln(outf,"\tRC");
                writln(outf,"\tSL");
                writln(outf,"\tLII\t"+QByteArray::number(var.address)+"\t; Store array element from "+var.varname);
                writln(outf,"\tLP\t0");
                writln(outf,"\tADM");
                writln(outf,"\tEXAM");
                writln(outf,"\tSTP");
                writln(outf,"\tINCP");
                writln(outf,"\tPOP"); pushcnt--;
                writln(outf,"\tEXAM");
                writln(outf,"\tDECP");
                writln(outf,"\tPOP"); pushcnt--;
                writln(outf,"\tEXAM");
            }
            else
            {
                writln(outf,"\tRC");
                writln(outf,"\tSL");
                writln(outf,"\tPUSH\t _t; Store array element from "+var.varname); pushcnt++;
                writln(outf,"\tLP\t7\t; HB of address");

                writln(outf,"\tLIA\tHB("+var.getLabel()+"-1)");
                writln(outf,"\tEXAM");
                writln(outf,"\tLP\t6\t; LB");
                writln(outf,"\tLIA\tLB("+var.getLabel()+"-1)");

                writln(outf,"\tEXAM");
                writln(outf,"\tPOP"); pushcnt--;
                writln(outf,"\tLIB\t0");
                writln(outf,"\tADB");
                writln(outf,"\tPOP"); pushcnt--;
                writln(outf,"\tEXAB");
                writln(outf,"\tPOP"); pushcnt--;
                writln(outf,"\tIYS");
                writln(outf,"\tEXAB");
                writln(outf,"\tIYS");
            }
        }
    }
}
#endif
