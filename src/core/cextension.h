#ifndef _CEXTENSION_H
#define _CEXTENSION_H

#include <QString>
#include <QAction>
#include <QMenu>

//PC-1500 ext
#define ID_CE151	0
#define ID_CE153	1
#define ID_CE155	2
#define ID_CE156	3
#define ID_CE157	4
#define ID_CE158	5
#define ID_CE159	6
#define ID_CE160	7
#define ID_CE161	8       // 16 Kb module

// PC-13xx PC-14XX ext
#define ID_CE201M	9
#define ID_CE202M	10
#define ID_CE203M	11
#define ID_CE210M	12
#define ID_CE211M	13
#define ID_CE212M	14
#define ID_CE2H16M	15
#define ID_CE2H32M	16
#define ID_CE2H64M	17

// PC-1600 ext
#define ID_CE1600M  18      // 32 Kb module
#define ID_CE1601M  19      // 64 Kb module
#define ID_CE1620M  20      // 32 Kb eprom
#define ID_CE1625M  21      // 32 Kb eprom
// Custom PC-1600 module
#define ID_CE16096  22      // 96 Kb module
#define ID_CE16128  23      // 128 Kb module
#define ID_CE16160  24      // 160 Kb module
#define ID_CE16192  25      // 192 Kb module
#define ID_CE16224  26      // 224 Kb module
#define ID_CE16256  27      // 256 Kb module

// CASIO PB Module
#define ID_OM51P    28      // PROLOG
#define ID_OM52C    29      // C
#define ID_OM53B    30      // BASIC
#define ID_OM54A    31      // CASL
#define ID_OM55L    32      // LISP

// CASIO FP-200 Module
#define ID_FP201    33      // 8Ko RAM Module
#define ID_FP205    34      // 8Ko EPROM Module
#define ID_FP231CE  35      // 8Ko RAM Module

#define ID_RP_8     36
#define ID_RP_33    37
#define ID_RP_256   38

#define NB_EXT  39

class CExtension{
public:
    const char*	GetClassName(){ return("CExtension"); }

    QString	Id;
    QString Description;
    bool	IsAvailable;
    bool	IsChecked;
    bool	needreboot;
    QString fname;
    int		DX,DY;
    int		offset_pcX,offset_pcY;

    QAction *Action;

    CExtension(void);			//[Basic constructor] {"","",false,false,true}
    CExtension(QString,QString,QString,int,int,int,int,bool,bool,bool);



};

class CExtensionArray{
public:
    const char*	GetClassName(){ return("CExtensionArray"); }

    QString Id;
    QString Description;
    QMenu	*Menu;
    QAction *emptyAction;
    QAction *loadAction;
    QAction *saveAction;

    QActionGroup *actionGroup;

    CExtension	*ExtArray[NB_EXT];
    QList<CExtension> ExtList;

    void setAvailable(int ind,bool value);
    void setAvailable(int ind,bool value,bool checked);
    void setChecked(int ind,bool value);


    CExtensionArray(QString,QString);               //[constructor]
    ~CExtensionArray(void)							//[constructor]
    {
        delete Menu;
        delete emptyAction;
        delete loadAction;
        delete saveAction;
        delete actionGroup;
        for (int i=0;i<NB_EXT;i++) delete ExtArray[i];

    }
};



#endif
